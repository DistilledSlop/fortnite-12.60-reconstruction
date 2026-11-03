#!/usr/bin/env python3
"""Recover exact parameterless BlueprintNativeEvent stubs through constructor vtables."""
import argparse, collections, hashlib, json, re, sys, tempfile
from pathlib import Path
sys.path[:0]=['/tmp/common-input-capstone',str(Path(__file__).resolve().parents[1])]
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
import build_vtable_slots as vt
ROOT=Path(__file__).resolve().parents[3]; REF=ROOT/'porting/generated/reflection'; HERE=Path(__file__).resolve().parent
IMG=Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'); OUT=Path('/tmp/gameplay-bulk-event-scalar-return')
load=lambda p:json.loads(p.read_text())
BASE=0x140000000; SHA='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
NET=0x80000000|0x01000000|0x00200000|0x4000|0x1000|0x100|0x80|0x40
ACCESS={0x20000:'public',0x40000:'private',0x80000:'protected'}
def proven_this_store(img, ctor, table, scratch):
 """Prove a straight-line store to the original this, tracking all register writes."""
 from capstone import CS_GRP_JUMP, CS_GRP_RET, CS_GRP_CALL
 from capstone.x86 import X86_OP_REG, X86_OP_MEM, X86_REG_RIP
 fn=img.func_of(ctor)
 if not fn or fn[0]!=ctor: return False
 md=Cs(CS_ARCH_X86,CS_MODE_64); md.detail=True
 aliases=set(); target_reg=None; incoming_valid=True
 saved={'rbx','rbp','rsi','rdi','r12','r13','r14','r15'}
 def full(reg):
  name=md.reg_name(reg)
  groups={'rax':('rax','eax','ax','al','ah'),'rbx':('rbx','ebx','bx','bl','bh'),
   'rcx':('rcx','ecx','cx','cl','ch'),'rdx':('rdx','edx','dx','dl','dh'),
   'rsi':('rsi','esi','si','sil'),'rdi':('rdi','edi','di','dil'),
   'rbp':('rbp','ebp','bp','bpl'),'rsp':('rsp','esp','sp','spl')}
  for i in range(8,16): groups['r'+str(i)]=tuple('r'+str(i)+s for s in ('','d','w','b'))
  return next((k for k,v in groups.items() if name in v),name)
 for ins in md.disasm(img.m[fn[0]-BASE:fn[1]-BASE],fn[0]):
  if ins.group(CS_GRP_JUMP) or ins.group(CS_GRP_RET) or ins.mnemonic in ('int3','ud2'): return False
  ops=ins.operands
  if (ins.mnemonic=='mov' and len(ops)==2 and ops[0].type==X86_OP_MEM and ops[1].type==X86_OP_REG
      and ops[0].mem.index==0 and ops[0].mem.disp==0 and ops[0].size==8 and ops[1].size==8
      and full(ops[1].reg)==target_reg):
   return full(ops[0].mem.base) in aliases
  copy=(ins.mnemonic=='mov' and len(ops)==2 and all(o.type==X86_OP_REG and o.size==8 for o in ops)
        and md.reg_name(ops[1].reg)=='rcx' and full(ops[0].reg) in saved and incoming_valid)
  lea=(ins.mnemonic=='lea' and len(ops)==2 and ops[0].type==X86_OP_REG and ops[0].size==8
       and ops[1].type==X86_OP_MEM and ops[1].mem.base==X86_REG_RIP and ops[1].mem.index==0
       and ins.address+ins.size+ops[1].mem.disp==table)
  writes={full(reg) for reg in ins.regs_access()[1]}
  if ins.group(CS_GRP_CALL): writes.update(('rax','rcx','rdx','r8','r9','r10','r11'))
  aliases.difference_update(writes)
  if target_reg in writes: target_reg=None
  if 'rcx' in writes: incoming_valid=False
  if copy: aliases.add(full(ops[0].reg))
  if lea: target_reg=full(ops[0].reg)
 return False

def main():
 parser=argparse.ArgumentParser(); parser.add_argument('--revalidate-own',action='store_true'); args=parser.parse_args()
 if hashlib.file_digest(IMG.open('rb'),'sha256').hexdigest()!=SHA: raise SystemExit('image digest mismatch')
 img=vt.Image(str(IMG)); md=Cs(CS_ARCH_X86,CS_MODE_64)
 native=load(REF/'native_funcs.json'); names=load(REF/'class_name_map.json'); parents=load(REF/'class_parent_map.json'); index=load(REF/'native_class_index.json'); refl=load(REF/'fortnite_game_reflection.json')
 blocks={x['block_va'].lower():x for x in refl['classes_in_groups_with_fort_names']}; byname=collections.defaultdict(list)
 for va,n in names.items(): byname[n].append(va)
 selected={x.strip() for x in (ROOT/'porting/native_class_set.txt').read_text().splitlines() if x.strip() and not x.lstrip().startswith('#')}; ported={(e.get('class'),e.get('function')) for e in load(ROOT/'porting/behavior/MANIFEST.json') if e.get('status')=='ported' and not (args.revalidate_own and (e.get('inc_path') or '').startswith('porting/behavior/bulk_event_scalar_return_runtime/'))}
 prefixes={'void':'mov rax, qword ptr [rdx + 0x20]; xor r8d, r8d; test rax, rax; setne r8b; add r8, rax; mov qword ptr [rdx + 0x20], r8; mov rax, qword ptr [rcx]; jmp qword ptr [rax + SLOT]', 'bool':'push rbx; sub rsp, 0x20; mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; mov rbx, r8; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; mov rax, qword ptr [rcx]; call qword ptr [rax + SLOT]; mov byte ptr [rbx], al; add rsp, 0x20; pop rbx; ret'}
 prefixes={'Int':prefixes['bool'].replace('mov byte ptr [rbx], al','mov dword ptr [rbx], eax'), 'UInt32':prefixes['bool'].replace('mov byte ptr [rbx], al','mov dword ptr [rbx], eax'), 'Float':prefixes['bool'].replace('mov byte ptr [rbx], al','movss dword ptr [rbx], xmm0')}
 patterns={k:re.compile(re.escape(v).replace('SLOT',r'(0x[0-9a-f]+|[0-9]+)')+'$') for k,v in prefixes.items()}
 work=[]; rejects=collections.Counter()
 for cls,fs in sorted(native.items()):
  for name,f in sorted(fs.items()):
   if not isinstance(f,dict): continue
   try: flags=int(f['flags'],0); addr=int(f['exec_addr'],0)
   except (KeyError,TypeError,ValueError): continue
   if not flags&0x8000000 or not flags&0x400: continue
   if flags&(~0xffffffcf|NET|0x20000000|0x2000) or (flags&0xe0000) not in ACCESS: continue
   if (cls,name) in ported or (cls,name+'_Implementation') in ported or cls not in selected or not re.fullmatch(r'[A-Za-z_]\w*',name): continue
   ps=f.get('params'); kind=None
   if isinstance(ps,list) and len(ps)==1 and ps[0].get('name')=='ReturnValue' and ps[0].get('kind') in ('Int','UInt32','Float') and ps[0].get('array_dim')==1:
    try:
     pf=int(ps[0]['property_flags'],0)
     if pf&0x400 and not pf&0x08000000: kind=ps[0]['kind']
    except (KeyError,TypeError,ValueError): pass
   if not kind: continue
   vs=byname.get(cls,[])
   if len(vs)!=1: continue
   va=vs[0]; parent=parents.get(va,{}); block=blocks.get(va.lower())
   if not block or parent.get('name')!=cls or parent.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000: continue
   ins=[]
   for i in md.disasm(img.m[addr-BASE:addr-BASE+96],addr):
    ins.append((i.mnemonic+' '+i.op_str).rstrip())
    if i.mnemonic in ('ret','jmp'): break
   asm='; '.join(ins); m=patterns[kind].fullmatch(asm)
   if not m: rejects['wrapper_shape']+=1; continue
   work.append((cls,name,f,flags,addr,kind,int(m.group(1),0),asm))
 cache={}; rows=[]; proposals=[]; targets=[]
 with tempfile.TemporaryDirectory(prefix='native-event-vtable-') as scratch:
  for cls,name,f,flags,addr,kind,slot,asm in work:
   if cls not in cache:
    gs=index.get(cls,{}).get('gpsc_targets') or []; result=None
    if len(gs)==1:
     gpsc=int(gs[0],0); ctor=vt.ctor_of(img,gpsc,scratch); thunk=vt.LAST_THUNK[0]
     if ctor:
      stores=vt.vptr_candidates(img,ctor,scratch); extra=vt.thunk_store_candidates(img,thunk,scratch) if thunk else []
      if len(stores)==1 and not extra and proven_this_store(img,ctor,stores[0],scratch): result=(gpsc,thunk,ctor,stores[0],len(vt.table_entries(img,stores[0])))
    cache[cls]=result
   v=cache[cls]
   if not v: rejects['constructor_or_vtable_ambiguous']+=1; continue
   gpsc,thunk,ctor,table,count=v
   if slot%8 or slot//8>=count: rejects['slot_outside_table']+=1; continue
   target=img.u64(table+slot)
   raw=bytes(img.m[target-BASE:target-BASE+16]) if BASE<=target<BASE+len(img.m) else b''
   body=None
   if kind in ('Int','UInt32'):
    if raw[:3] in (bytes.fromhex('33c0c3'),bytes.fromhex('31c0c3')): body=('return 0;',raw[:3].hex())
    elif raw[:1]==b'\xb8' and raw[5:6]==b'\xc3':
     val=int.from_bytes(raw[1:5],'little'); literal=str(val) if kind=='UInt32' else str(val if val<0x80000000 else val-0x100000000)
     body=('return '+literal+('u' if kind=='UInt32' else '')+';',raw[:6].hex())
   elif kind=='Float' and raw[:4]==bytes.fromhex('0f57c0c3'):
    body=('return 0.0f;',raw[:4].hex())
   if not body:
    rejects['target_not_exact_stub']+=1
    targets.append(dict(class_=cls,function=name,kind=kind,target=hex(target),bytes=raw.hex(),slot=hex(slot)))
    continue
   ctype={'Int':'int32','UInt32':'uint32','Float':'float'}[kind]
   signature=f'{ctype} {name}_Implementation()'+(' const' if flags&0x40000000 else '')
   inc=HERE/(hex(target)[2:]+'_'+kind+'.inc'); inc.write_text('// Exact original scalar stub '+hex(target)+' ('+body[1]+').\n// CONFIDENCE: HIGH\n'+body[0]+'\n')
   row=dict(class_=cls,function=name,exec_addr=hex(addr),gpsc=hex(gpsc),thunk=hex(thunk),ctor=hex(ctor),vtable=hex(table),slots=count,slot=hex(slot),target=hex(target),target_bytes=body[1],wrapper=asm,flags=f['flags']); rows.append(row)
   proposals.append({'class':cls,'function':name+'_Implementation','kind':'native_event_impl','signature':signature,'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':f['flags'],'static':False,'access':ACCESS[flags&0xe0000],'native_body_address':hex(target),'notes':f'Exact parameterless exec virtual slot {hex(slot)}; constructor {hex(ctor)} stores unique vtable {hex(table)}, whose slot targets exact stub {hex(target)}. See bulk_event_scalar_return_runtime/README.md.'})
 OUT.mkdir(parents=True,exist_ok=True)
 report={'counts':{'wrapper_candidates':len(work),'candidate':len(rows),'classes':len(cache)},'rejections':dict(rejects),'candidates':rows,'unmatched_targets':targets}
 for filename,data in [('report.json',report),('manifest-entries.json',proposals)]: (OUT/filename).write_text(json.dumps(data,indent=2)+'\n')
 print(json.dumps({k:report[k] for k in ('counts','rejections')},indent=2))
if __name__=='__main__': main()
