#!/usr/bin/env python3
"""Finite UFortItem base virtual bodies; reproduce wrapper/constructor/slot proof."""
import hashlib, importlib.util, json, re, struct, sys, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
sys.path[:0]=['/tmp/common-input-capstone',str(ROOT/'porting/behavior')]
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
import build_vtable_slots as vt
spec=importlib.util.spec_from_file_location('proof',ROOT/'porting/behavior/bulk_virtual_scalar_runtime/prepare.py')
proof=importlib.util.module_from_spec(spec); spec.loader.exec_module(proof)
with proof.IMG.open('rb') as source:
 assert hashlib.file_digest(source, 'sha256').hexdigest()==proof.SHA, 'image SHA-256 mismatch'
img=vt.Image(str(proof.IMG)); md=Cs(CS_ARCH_X86,CS_MODE_64); md.detail=True
rows=[('GetDurability',0x142958db0,0x2c8,0x14084bee0,'float','return -1.0f;','movss dword ptr [rbx], xmm0'),('GetNormalizedDurability',0x1429594c0,0x2d0,0x1407c46d0,'float','return 1.0f;','movss dword ptr [rbx], xmm0'),('GetNumInStack',0x142959500,0x368,0x140833bf0,'int32','return 1;','mov dword ptr [rbx], eax'),('GetNumberOfSlotsTaken',0x142959530,0x2e8,0x1406801d0,'uint8','return 1;','mov byte ptr [rbx], al'),('GetRemainingAmmo',0x142959880,0x380,0x140693f70,'int32','return -1;','mov dword ptr [rbx], eax'),('GetItemDefinitionBP',0x142959140,0x3e8,0x140680300,'UFortItemDefinition*','return nullptr;','mov qword ptr [rbx], rax')]
with tempfile.TemporaryDirectory() as s:
 g=vt.load_gpsc(vt.GPSC_INDEX)['UFortItem']; c=vt.ctor_of(img,g,s); thunk=vt.LAST_THUNK[0]; stores=vt.vptr_candidates(img,c,s)
 assert (g,thunk,c,stores)==(0x142951e90,0x1429551e0,0x142024010,[0x14606c3f0])
 assert not vt.thunk_store_candidates(img,thunk,s) and proof.proven_this_store(img,c,stores[0],s)
 table=stores[0]; assert len(vt.table_entries(img,table))>0x3e8//8
ref=json.loads((ROOT/'porting/generated/reflection/native_funcs.json').read_text())['UFortItem']
manifest=json.loads((ROOT/'porting/behavior/MANIFEST.json').read_text())
ported={(e.get('class'),e.get('function')) for e in manifest if e.get('status')=='ported'}
source=ROOT/'porting/native/Source'
maintained=set()
for cpp in source.rglob('*.cpp'):
 if 'Generated' not in cpp.parts:
  maintained.update(re.findall(r'\b([A-Za-z_]\w*)::([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?\{',cpp.read_text(errors='replace')))
header=(source/'FortniteGame/Generated/UFortItem.h').read_text()
implementation=(source/'FortniteGame/Generated/UFortItem.cpp').read_text()
refs=json.loads((ROOT/'porting/generated/reflection/class_ref_map.json').read_text()); names=json.loads((ROOT/'porting/generated/reflection/class_name_map.json').read_text())
assert names[refs['resolved']['0x14295b4c0']]=='UFortItemDefinition'
expected={0x140833bf0:'b801000000c3',0x1406801d0:'b001c3',0x140693f70:'b8ffffffffc3',0x140680300:'33c0c3'}
for va,raw in expected.items(): assert img.m[va-vt.BASE:va-vt.BASE+len(bytes.fromhex(raw))]==bytes.fromhex(raw)
for va,constant,value in [(0x14084bee0,0x1459a979c,-1.0),(0x1407c46d0,0x1459a86f8,1.0)]:
 section=[(n,ch) for n,start,size,ch in img.secs if start<=constant-vt.BASE and constant-vt.BASE+4<=start+size]
 assert len(section)==1 and section[0][1]&0x40 and section[0][1]&0x40000000 and not section[0][1]&0x80000000, (hex(constant),section)
 ins=list(md.disasm(img.m[va-vt.BASE:va-vt.BASE+9],va)); assert len(ins)==2 and ins[0].mnemonic=='movss' and ins[0].address+ins[0].size+ins[0].disp==constant and ins[1].mnemonic=='ret'
 assert struct.unpack_from('<f',img.m,constant-vt.BASE)[0]==value
out=[]
for name,addr,slot,target,typ,stmt,store in rows:
 assert ('UFortItem',name) not in ported and ('UFortItem',name+'_Implementation') not in ported and ('UFortItem',name) not in maintained
 assert not re.search(r'\b'+re.escape(name)+r'\s*\(',header) and not re.search(r'\bUFortItem::'+re.escape(name)+r'\s*\(',implementation)
 f=ref[name]; assert int(f['exec_addr'],0)==addr and int(f['flags'],0)==(0x54020401 if typ=='UFortItemDefinition*' else 0x54020400) and len(f['params'])==1
 param=f['params'][0]; assert param['name']=='ReturnValue' and param['array_dim']==1 and int(param['property_flags'],0)==0x10000000000580
 assert param['kind']=={'float':'Float','int32':'Int','uint8':'Byte','UFortItemDefinition*':'Object'}[typ]
 assert int(param['kind_raw'],0)=={'float':0xa,'int32':3,'uint8':0,'UFortItemDefinition*':0x12}[typ] and param['offset']==0 and not param['replicated'] and param['rep_notify'] is None
 assert param.get('type_refs',{})==({'class':'0x14295b4c0'} if typ=='UFortItemDefinition*' else {})
 assert img.u64(table+slot)==target
 ins=[]
 for i in md.disasm(img.m[addr-vt.BASE:addr-vt.BASE+96],addr):
  ins.append((i.mnemonic+' '+i.op_str).rstrip())
  if i.mnemonic=='ret': break
 prefix=['push rbx','sub rsp, 0x20','mov rax, qword ptr [rdx + 0x20]','xor r9d, r9d','test rax, rax','mov rbx, r8','setne r9b','add r9, rax','mov qword ptr [rdx + 0x20], r9','mov rax, qword ptr [rcx]',f'call qword ptr [rax + {hex(slot)}]',store,'add rsp, 0x20','pop rbx','ret']
 assert ins==prefix,(name,ins)
 inc=Path(__file__).parent/(name+'.inc'); inc.write_text(f'// Exact UFortItem base implementation at {hex(target)}.\n// CONFIDENCE: HIGH\n{stmt}\n')
 out.append({'class':'UFortItem','function':name,'kind':'exec','signature':f'{typ} {name}() const','signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':f['flags'],'static':False,'virtual':True,'access':'public','native_body_address':hex(target),'notes':f'Exact wrapper slot {hex(slot)}; constructor {hex(c)} installs primary vtable {hex(table)}, slot target {hex(target)}. Base implementation only; subclass overrides require separate recovery. See item_queries_runtime/HANDOFF.md.'})
(Path(__file__).parent/'manifest-entries.json').write_text(json.dumps(out,indent=2)+'\n')
print(f'{len(out)} base bodies proven')
