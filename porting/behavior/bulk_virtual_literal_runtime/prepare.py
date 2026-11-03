#!/usr/bin/env python3
"""Recover exact parameterless ordinary virtual stubs through constructor vtables."""
import argparse, collections, hashlib, json, math, re, struct, sys, tempfile
from pathlib import Path
sys.path[:0]=['/tmp/common-input-capstone',str(Path(__file__).resolve().parents[1])]
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
import build_vtable_slots as vt
ROOT=Path(__file__).resolve().parents[3]; REF=ROOT/'porting/generated/reflection'; HERE=Path(__file__).resolve().parent
IMG=Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'); OUT=Path('/tmp/gameplay-bulk-virtual-literal')
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
 selected={x.strip() for x in (ROOT/'porting/native_class_set.txt').read_text().splitlines() if x.strip() and not x.lstrip().startswith('#')}; ported={(e.get('class'),e.get('function')) for e in load(ROOT/'porting/behavior/MANIFEST.json') if e.get('status')=='ported' and not (args.revalidate_own and (e.get('inc_path') or '').startswith('porting/behavior/bulk_virtual_literal_runtime/'))}
 source=ROOT/'porting/native/Source'
 maintained=set()
 for cpp in source.rglob('*.cpp'):
  if 'Generated' not in cpp.parts:
   maintained.update(re.findall(r'\b([A-Za-z_]\w*)::([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?\{',cpp.read_text(errors='replace')))
 headers=collections.defaultdict(list)
 for h in source.glob('*/Generated/*.h'): headers[h.stem].append(h)
 prefixes={'void':'mov rax, qword ptr [rdx + 0x20]; xor r8d, r8d; test rax, rax; setne r8b; add r8, rax; mov qword ptr [rdx + 0x20], r8; mov rax, qword ptr [rcx]; jmp qword ptr [rax + SLOT]', 'bool':'push rbx; sub rsp, 0x20; mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; mov rbx, r8; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; mov rax, qword ptr [rcx]; call qword ptr [rax + SLOT]; mov byte ptr [rbx], al; add rsp, 0x20; pop rbx; ret'}
 prefixes['int']=prefixes['bool'].replace('mov byte ptr [rbx], al','mov dword ptr [rbx], eax')
 prefixes['byte']=prefixes['bool']
 prefixes['uint']=prefixes['int']
 prefixes['float']=prefixes['bool'].replace('mov byte ptr [rbx], al','movss dword ptr [rbx], xmm0')
 patterns={k:re.compile(re.escape(v).replace('SLOT',r'(0x[0-9a-f]+|[0-9]+)')+'$') for k,v in prefixes.items()}
 work=[]; rejects=collections.Counter()
 for cls,fs in sorted(native.items()):
  for name,f in sorted(fs.items()):
   if not isinstance(f,dict): continue
   try: flags=int(f['flags'],0); addr=int(f['exec_addr'],0)
   except (KeyError,TypeError,ValueError): continue
   if flags&0x8000000 or not flags&0x400: continue
   if flags&(~0xffffffcf|NET|0x20000000|0x2000) or (flags&0xe0000) not in ACCESS: continue
   if cls=='UFortItem' and name in {'GetDurability','GetNormalizedDurability','GetNumInStack','GetNumberOfSlotsTaken','GetRemainingAmmo','GetItemDefinitionBP'}: continue
   if (cls,name) in maintained: continue
   if (cls,name) in ported or (cls,name+'_Implementation') in ported or cls not in selected or not re.fullmatch(r'[A-Za-z_]\w*',name): continue
   ps=f.get('params'); kind=None
   if isinstance(ps,list) and len(ps)==1:
    p=ps[0]
    try:
     rawkind={'Int':3,'UInt32':6,'Byte':0,'Float':10}
     if (p.get('name')=='ReturnValue' and p.get('kind') in rawkind and p.get('array_dim')==1
         and int(p['property_flags'],0)==0x10000000000580 and int(p['kind_raw'],0)==rawkind[p['kind']]
         and p.get('offset')==0 and p.get('replicated') is False and p.get('rep_notify') is None and not p.get('type_refs')):
      kind={'Int':'int','UInt32':'uint','Byte':'byte','Float':'float'}[p['kind']]
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
 cache={}; rows=[]; proposals=[]
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
   hs=headers.get(cls,[])
   if len(hs)!=1 or re.search(r'\b'+re.escape(name)+r'\s*\(',hs[0].read_text()): rejects['generated_header_conflict']+=1; continue
   generated_cpp=hs[0].with_suffix('.cpp')
   if generated_cpp.exists() and re.search(r'\b'+re.escape(cls)+r'::'+re.escape(name)+r'\s*\(',generated_cpp.read_text()): rejects['generated_definition_conflict']+=1; continue
   target=img.u64(table+slot)
   if not BASE<=target<BASE+len(img.m)-16: rejects['target_outside_image']+=1; continue
   raw=bytes(img.m[target-BASE:target-BASE+16]); stmt=None; constant=None; constant_bits=None; section_name=None
   if kind in ('int','uint') and raw[0]==0xb8 and raw[5]==0xc3:
    value=int.from_bytes(raw[1:5],'little')
    if kind=='int' and value>=0x80000000: value-=0x100000000
    stmt='return '+str(value)+('u' if kind=='uint' else '')+';'; leaf=raw[:6].hex()
   elif kind=='byte' and raw[0]==0xb0 and raw[2]==0xc3:
    stmt='return '+str(raw[1])+';'; leaf=raw[:3].hex()
   elif kind=='float' and raw[:4]==bytes.fromhex('f30f1005') and raw[8]==0xc3:
    constant=target+8+int.from_bytes(raw[4:8],'little',signed=True)
    sections=[(n,ch) for n,start,size,ch in img.secs if start<=constant-BASE and constant-BASE+4<=start+size]
    if len(sections)==1 and sections[0][1]&0x40 and sections[0][1]&0x40000000 and not sections[0][1]&0x80000000:
     bits=bytes(img.m[constant-BASE:constant-BASE+4]); constant_bits=bits.hex(); section_name=sections[0][0]; value=struct.unpack('<f',bits)[0]
     if math.isfinite(value) and bits!=bytes.fromhex('00000080'):
      literal=format(value,'.9g')
      if '.' not in literal and 'e' not in literal: literal+='.0'
      if struct.pack('<f',float(literal))==bits:
       stmt='return '+literal+'f;'; leaf=raw[:9].hex()
   if stmt is None: rejects['target_not_exact_literal']+=1; continue
   signature=f"{ {'int':'int32','uint':'uint32','byte':'uint8','float':'float'}[kind]} {name}()"+(' const' if flags&0x40000000 else '')
   inc=HERE/(hex(target)[2:]+'_'+kind+'.inc'); inc.write_text('// Exact original literal stub '+hex(target)+' ('+leaf+').\n// CONFIDENCE: HIGH\n'+stmt+'\n')
   row=dict(class_=cls,function=name,exec_addr=hex(addr),gpsc=hex(gpsc),thunk=hex(thunk),ctor=hex(ctor),vtable=hex(table),slots=count,slot=hex(slot),target=hex(target),target_bytes=leaf,constant=hex(constant) if constant else None,constant_bits=constant_bits,section=section_name,wrapper=asm,statement=stmt,flags=f['flags']); rows.append(row)
   proposals.append({'class':cls,'function':name,'kind':'exec','signature':signature,'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':f['flags'],'static':False,'virtual':True,'access':ACCESS[flags&0xe0000],'native_body_address':hex(target),'notes':f'Exact parameterless exec virtual slot {hex(slot)}; constructor {hex(ctor)} stores unique vtable {hex(table)}, whose slot targets exact stub {hex(target)}. See bulk_virtual_literal_runtime/README.md.'})
 OUT.mkdir(parents=True,exist_ok=True)
 report={'counts':{'wrapper_candidates':len(work),'candidate':len(rows),'classes':len(cache)},'rejections':dict(rejects),'candidates':rows}
 for filename,data in [('report.json',report),('manifest-entries.json',proposals)]: (OUT/filename).write_text(json.dumps(data,indent=2)+'\n')
 print(json.dumps({k:report[k] for k in ('counts','rejections')},indent=2))
if __name__=='__main__': main()
