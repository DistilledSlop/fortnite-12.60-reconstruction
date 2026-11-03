#!/usr/bin/env python3
"""Recover exact single-parameter native event stubs through constructor vtables."""
import argparse, collections, hashlib, json, re, sys, tempfile
from pathlib import Path
sys.path[:0]=['/tmp/common-input-capstone',str(Path(__file__).resolve().parents[1])]
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
import build_vtable_slots as vt
ROOT=Path(__file__).resolve().parents[3]; REF=ROOT/'porting/generated/reflection'; HERE=Path(__file__).resolve().parent
IMG=Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'); OUT=Path('/tmp/gameplay-bulk-parameter-event-extra')
load=lambda p:json.loads(p.read_text())
BASE=0x140000000; SHA='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
NET=0x80000000|0x01000000|0x00200000|0x4000|0x1000|0x100|0x80|0x40
ACCESS={0x20000:'public',0x40000:'private',0x80000:'protected'}
import importlib.util
spec=importlib.util.spec_from_file_location("native_event", HERE.parent/'bulk_native_event_runtime/prepare.py')
native_event=importlib.util.module_from_spec(spec); spec.loader.exec_module(native_event)
proven_this_store=native_event.proven_this_store

def main():
 parser=argparse.ArgumentParser(); parser.add_argument('--revalidate-own',action='store_true'); args=parser.parse_args()
 if hashlib.file_digest(IMG.open('rb'),'sha256').hexdigest()!=SHA: raise SystemExit('image digest mismatch')
 img=vt.Image(str(IMG)); md=Cs(CS_ARCH_X86,CS_MODE_64)
 native=load(REF/'native_funcs.json'); names=load(REF/'class_name_map.json'); parents=load(REF/'class_parent_map.json'); index=load(REF/'native_class_index.json'); refl=load(REF/'fortnite_game_reflection.json'); refs=load(REF/'class_ref_map.json')['resolved']
 blocks={x['block_va'].lower():x for x in refl['classes_in_groups_with_fort_names']}; byname=collections.defaultdict(list)
 for va,n in names.items(): byname[n].append(va)
 selected={x.strip() for x in (ROOT/'porting/native_class_set.txt').read_text().splitlines() if x.strip() and not x.lstrip().startswith('#')}; ported={(e.get('class'),e.get('function')) for e in load(ROOT/'porting/behavior/MANIFEST.json') if e.get('status')=='ported' and not (args.revalidate_own and (e.get('inc_path') or '').startswith('porting/behavior/bulk_parameter_event_extra_runtime/'))}
 # Complete instruction sequence of the generated single-object-shaped wrapper. Branches
 # are checked against exact local offsets, and both property helpers are fixed.
 expected_object = [
 'mov qword ptr [rsp + 0x10], rbx','mov qword ptr [rsp + 0x18], rsi',
 'push rdi','sub rsp, 0x20','xor edi, edi','mov rsi, rcx','mov rbx, rdx',
 'mov qword ptr [rsp + 0x30], rdi','mov rcx, rdx','cmp qword ptr [rdx + 0x20], rdi',
 'je LOCAL_35','mov rdx, qword ptr [rdx + 0x18]','lea r8, [rsp + 0x30]',
 'call 0x142ed9cf0','jmp LOCAL_51','mov r8, qword ptr [rdx + 0x80]',
 'mov rax, qword ptr [r8 + 0x20]','mov qword ptr [rdx + 0x80], rax',
 'lea rdx, [rsp + 0x30]','call 0x142ed9d20','mov rax, qword ptr [rbx + 0x20]',
 'mov rcx, rsi','mov rdx, qword ptr [rsp + 0x30]','test rax, rax','setne dil',
 'add rdi, rax','mov qword ptr [rbx + 0x20], rdi','mov rax, qword ptr [rsi]',
 'call qword ptr [rax + SLOT]','mov rbx, qword ptr [rsp + 0x38]',
 'mov rsi, qword ptr [rsp + 0x40]','add rsp, 0x20','pop rdi','ret']
 expected_float = [
 'mov qword ptr [rsp + 0x10], rbx','push rdi','sub rsp, 0x20',
 'cmp qword ptr [rdx + 0x20], 0','xorps xmm0, xmm0','mov rdi, rcx',
 'movss dword ptr [rsp + 0x30], xmm0','mov rbx, rdx','mov rcx, rdx',
 'je LOCAL_33','mov rdx, qword ptr [rdx + 0x18]','lea r8, [rsp + 0x30]',
 'call 0x142ed9cf0','jmp LOCAL_4f','mov r8, qword ptr [rdx + 0x80]',
 'mov rax, qword ptr [r8 + 0x20]','mov qword ptr [rdx + 0x80], rax',
 'lea rdx, [rsp + 0x30]','call 0x142ed9d20','mov rax, qword ptr [rbx + 0x20]',
 'xor ecx, ecx','movss xmm1, dword ptr [rsp + 0x30]','test rax, rax',
 'setne cl','add rcx, rax','mov qword ptr [rbx + 0x20], rcx',
 'mov rcx, rdi','mov rax, qword ptr [rdi]','call qword ptr [rax + SLOT]',
 'mov rbx, qword ptr [rsp + 0x38]','add rsp, 0x20','pop rdi','ret']
 # The instruction lists are complete forms observed in the pinned image. Only
 # operand substitutions are made; all branches and stack slots remain exact.
 expected_int=[x.replace('mov qword ptr [rsp + 0x30], rdi','mov dword ptr [rsp + 0x30], edi').replace('mov rdx, qword ptr [rsp + 0x30]','mov edx, dword ptr [rsp + 0x30]').replace('je LOCAL_35','je LOCAL_34').replace('jmp LOCAL_51','jmp LOCAL_50') for x in expected_object]
 expected_name=expected_object
 expected_bool=expected_int.copy()
 expected_bool[22:23]=[]
 expected_bool[25:25]=['cmp dword ptr [rsp + 0x30], 0']
 expected_bool[28:28]=['setne dl']
 expected_byte=[x.replace('xorps xmm0, xmm0','mov rcx, rdx').replace('movss dword ptr [rsp + 0x30], xmm0','mov byte ptr [rsp + 0x30], 0').replace('movss xmm1, dword ptr [rsp + 0x30]','movzx edx, byte ptr [rsp + 0x30]').replace('je LOCAL_33','je LOCAL_2f').replace('jmp LOCAL_4f','jmp LOCAL_4b') for x in expected_float]
 expected_byte.remove('mov rcx, rdx') # remove the earlier duplicate, keep correct position below
 expected_byte.insert(5,'mov rcx, rdx')
 # Remove the original float form's second copy after mov rbx.
 del expected_byte[8]
 expected={'Int':expected_int,'UInt32':expected_int,'Name':expected_name,'Bool':expected_bool,'Byte':expected_byte}
 maintained=set()
 for cpp in (ROOT/'porting/native/Source').rglob('*.cpp'):
  if 'Generated' not in cpp.parts: maintained.update(re.findall(r'\b([A-Za-z_]\w*)::([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?\{',cpp.read_text(errors='replace')))
 work=[]; rejects=collections.Counter()
 for cls,fs in sorted(native.items()):
  for name,f in sorted(fs.items()):
   if not isinstance(f,dict): continue
   try: flags=int(f['flags'],0); addr=int(f['exec_addr'],0)
   except (KeyError,TypeError,ValueError): continue
   if not flags&0x8000000 or not flags&0x400: continue
   if flags&(~0xffffffcf|NET|0x20000000|0x2000) or (flags&0xe0000) not in ACCESS: continue
   if (cls,name) in maintained or (cls,name+'_Implementation') in maintained or (cls,name) in ported or (cls,name+'_Implementation') in ported or cls not in selected or not re.fullmatch(r'[A-Za-z_]\w*',name): continue
   ps=f.get('params')
   if not isinstance(ps,list) or len(ps)!=1: continue
   p=ps[0]
   if (p.get('kind') not in ('Int','UInt32','Name','Bool','Byte') or p.get('array_dim')!=1 or p.get('index')!=0
       or p.get('property_flags')!='0x10000000000080' or p.get('type_refs')
       or not re.fullmatch(r'[A-Za-z_]\w*',p.get('name',''))): continue
   target_name={'Int':'int32','UInt32':'uint32','Name':'FName','Bool':'bool','Byte':'uint8'}[p['kind']]
   # Do not replace existing maintained generated definitions or inline bodies.
   cpp=list((ROOT/'porting/native/Source').glob('*/Generated/'+cls+'.cpp'))
   headers=list((ROOT/'porting/native/Source').glob('*/Generated/'+cls+'.h'))
   if len(cpp)>1 or len(headers)>1: continue
   if cpp and re.search(r'\b'+re.escape(cls)+r'::'+re.escape(name+'_Implementation')+r'\s*\(',cpp[0].read_text()): continue
   if headers and re.search(r'\b'+re.escape(name+'_Implementation')+r'\s*\([^;{}]*\)\s*(?:const\s*)?\{',headers[0].read_text()): continue
   kind='void'
   vs=byname.get(cls,[])
   if len(vs)!=1: continue
   va=vs[0]; parent=parents.get(va,{}); block=blocks.get(va.lower())
   if not block or parent.get('name')!=cls or parent.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000: continue
   if not BASE<=addr<BASE+len(img.m)-160: continue
   ins=[]; slot=None
   for i in md.disasm(img.m[addr-BASE:addr-BASE+160],addr):
    op=i.op_str
    if i.mnemonic in ('je','jmp'):
     try: op='LOCAL_'+format(int(op,0)-addr,'x')
     except ValueError: break
    if i.mnemonic=='call' and re.fullmatch(r'qword ptr \[rax \+ (0x[0-9a-f]+|[0-9]+)\]',op):
     slot=int(op[17:-1],0); op='qword ptr [rax + SLOT]'
    ins.append((i.mnemonic+' '+op).rstrip())
    if i.mnemonic=='ret': break
   if ins!=expected[p['kind']] or slot is None: rejects['wrapper_shape']+=1; continue
   work.append((cls,name,f,flags,addr,kind,slot,target_name))
 cache={}; rows=[]; proposals=[]
 with tempfile.TemporaryDirectory(prefix='native-event-vtable-') as scratch:
  for cls,name,f,flags,addr,kind,slot,target_name in work:
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
   target=img.u64(table+slot); body={0x1406801c0:('void','c20000','return;'),0x1406801d0:('bool','b001c3','return true;'),0x1406801e0:('bool','32c0c3','return false;')}.get(target)
   if not body or body[0]!=kind or img.m[target-BASE:target-BASE+len(bytes.fromhex(body[1]))]!=bytes.fromhex(body[1]): rejects['target_not_exact_stub']+=1; continue
   signature=f"void {name}_Implementation({target_name} {f['params'][0]['name']})"+(' const' if flags&0x40000000 else '')
   inc=HERE/(body[2].replace(' ','_').replace(';','')+'.inc'); inc.write_text('// Exact original shared stub '+hex(target)+'.\n// CONFIDENCE: HIGH\n'+body[2]+'\n')
   row=dict(class_=cls,function=name,exec_addr=hex(addr),gpsc=hex(gpsc),thunk=hex(thunk),ctor=hex(ctor),vtable=hex(table),slots=count,slot=hex(slot),target=hex(target),flags=f['flags']); rows.append(row)
   proposals.append({'class':cls,'function':name+'_Implementation','kind':'native_event_impl','signature':signature,'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':f['flags'],'static':False,'access':ACCESS[flags&0xe0000],'native_body_address':hex(target),'notes':f'Exact single-parameter exec virtual slot {hex(slot)}; constructor {hex(ctor)} stores unique vtable {hex(table)}, whose slot targets exact stub {hex(target)}. See bulk_parameter_event_extra_runtime/README.md.'})
 OUT.mkdir(parents=True,exist_ok=True)
 report={'counts':{'wrapper_candidates':len(work),'candidate':len(rows),'classes':len(cache)},'rejections':dict(rejects),'candidates':rows}
 for filename,data in [('report.json',report),('manifest-entries.json',proposals)]: (OUT/filename).write_text(json.dumps(data,indent=2)+'\n')
 print(json.dumps({k:report[k] for k in ('counts','rejections')},indent=2))
if __name__=='__main__': main()
