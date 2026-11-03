#!/usr/bin/env python3
"""Recover exact additional one-parameter exec wrappers whose entire direct leaf is ret."""
import collections, hashlib, importlib.util, json, re, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]; HERE=Path(__file__).resolve().parent
sys.path.insert(0,'/tmp/common-input-capstone'); sys.path.insert(0,str(ROOT/'porting'))
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
from gen_native_classes import split_properties

def module(name):
 p=HERE.parent/name/'prepare.py'; s=importlib.util.spec_from_file_location(name,p); m=importlib.util.module_from_spec(s); s.loader.exec_module(m); return m
F=module('bulk_float_setter_runtime'); I=module('bulk_parameter_setter_runtime')
H=F.H
BYTE_HEAD=F.HEAD.replace('xorps xmm0, xmm0; ', '').replace(r'movss dword ptr \[rsp \+ '+H+r'\], xmm0', r'mov byte ptr \[rsp \+ '+H+r'\], 0').replace(r'movss (xmm0|xmm1), dword ptr \[rsp \+ '+H+r'\]', r'movzx edx, byte ptr \[rsp \+ '+H+r'\]')
BYTE_HEAD=BYTE_HEAD.replace(r'mov rdi, rcx; mov byte ptr \[rsp \+ '+H+r'\], 0; mov rbx, rdx; mov rcx, rdx;', r'mov rdi, rcx; mov rcx, rdx; mov byte ptr \[rsp \+ '+H+r'\], 0; mov rbx, rdx;')
BYTE=re.compile(BYTE_HEAD+r'mov rcx, rdi; call (0x[0-9a-f]+); mov rbx, qword ptr \[rsp \+ '+H+r'\]; add rsp, 0x20; pop rdi; ret')
OUT=Path('/tmp/gameplay-bulk-ignored-extra'); load=lambda p:json.loads(p.read_text())
def main():
 img=F.IMAGE.read_bytes(); digest=hashlib.sha256(img).hexdigest()
 if digest!=F.SHA: raise SystemExit('image digest mismatch')
 md=Cs(CS_ARCH_X86,CS_MODE_64)
 def decode(a):
  if a<F.BASE or a-F.BASE+256>len(img): return [],''
  ins=[]
  for i in md.disasm(img[a-F.BASE:a-F.BASE+256],a):
   ins.append(i)
   if i.mnemonic=='ret': return ins,'; '.join((x.mnemonic+' '+x.op_str).rstrip() for x in ins)
  return [],''
 refl=F.REFL; native=load(refl/'native_funcs.json'); names=load(refl/'class_name_map.json'); parents=load(refl/'class_parent_map.json'); refs=load(refl/'class_ref_map.json')['resolved']
 blocks={x['block_va'].lower():x for x in load(refl/'fortnite_game_reflection.json')['classes_in_groups_with_fort_names']}; byname=collections.defaultdict(list)
 for va,n in names.items(): byname[n].append(va)
 selected={x.strip() for x in (ROOT/'porting/native_class_set.txt').read_text().splitlines() if x.strip() and not x.lstrip().startswith('#')}
 ported={(x.get('class'),x.get('function')) for x in load(ROOT/'porting/behavior/MANIFEST.json') if x.get('status')=='ported'}
 maintained=set()
 for cpp in (ROOT/'porting/native/Source').rglob('*.cpp'):
  if 'Generated' not in cpp.parts: maintained.update(re.findall(r'\b([A-Za-z_]\w*)::([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?\{',cpp.read_text(errors='replace')))
 headers=collections.defaultdict(list)
 for mod in ('FortniteGame','FortniteUI'):
  for p in (ROOT/'porting/native/Source'/mod/'Generated').glob('*.h'): headers[p.stem].append(p)
 rejects=collections.Counter(); results=[]; total=0
 for cls, funcs in sorted(native.items()):
  for name,fn in sorted(funcs.items()):
   total+=1
   if not isinstance(fn,dict) or not isinstance(fn.get('params'),list) or len(fn['params'])!=1: rejects['not_one_parameter']+=1; continue
   pairs,notes=split_properties(fn['params'])
   if notes or len(pairs)!=1 or pairs[0][1]: rejects['parameter_metadata']+=1; continue
   p=pairs[0][0]; kind=p.get('kind'); pname=p.get('name',''); pf=int(p.get('property_flags','0'),0)
   if kind not in ('Name','Byte','UInt32') or p.get('array_dim')!=1 or pf&(0x400|0x100|0x08000000) or not pf&0x80 or not re.fullmatch(r'[A-Za-z_]\w*',pname) or p.get('type_refs'): rejects['parameter_kind_or_flags']+=1; continue
   try: flags=int(fn['flags'],0); addr=int(fn['exec_addr'],0)
   except (KeyError,ValueError,TypeError): rejects['address_or_flags']+=1; continue
   if flags & (~0xffffffcf|F.NET|0x08000000|0x20000000|0x2000|0x40000000) or not flags&0x400 or (flags&0xe0000) not in F.ACCESS: rejects['function_flags']+=1; continue
   if (cls,name) in ported or (cls,name) in maintained: rejects['already_ported_or_maintained']+=1; continue
   vs=byname.get(cls,[])
   if len(vs)!=1 or cls not in selected or not re.fullmatch(r'[A-Za-z_]\w*',name): rejects['class_or_identifier']+=1; continue
   par=parents.get(vs[0],{}); block=blocks.get(vs[0].lower())
   if not block or cls not in par.get('chain',[]) or par.get('name')!=cls or par.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000 or len(headers[cls])!=1: rejects['class_metadata_or_header']+=1; continue
   ins,asm=decode(addr); m=(BYTE if kind=='Byte' else I.PAT).fullmatch(asm)
   if not m: rejects['unmatched_thunk']+=1; continue
   g=m.groups()
   if kind=='Byte':
    loc=g[1]; good=loc==g[3]==g[5]==g[6] and int(g[0],0)+0x28==int(g[8],0) and int(loc,0)!=int(g[8],0) and 0x20<=int(loc,0)<0x40; target=int(g[7],0)
   else:
    loc=g[3]; width=('dword',loc,'edi') if kind=='UInt32' else ('qword',loc,'rdi'); reg=('edx','dword') if kind=='UInt32' else ('rdx','qword')
    good=g[2:5]==width and g[6]==g[8]==g[11]==loc and g[9:11]==reg and int(g[0],0)+0x28==int(g[13],0) and int(g[1],0)+0x28==int(g[14],0) and int(loc,0) not in (int(g[13],0),int(g[14],0)) and 0x20<=int(loc,0)<0x48; target=int(g[12],0)
   if not good: rejects['stack_or_register']+=1; continue
   je=next((i for i in ins if i.mnemonic=='je'),None); jmp=next((i for i in ins if i.mnemonic=='jmp'),None); alt=next((i for i in ins if i.mnemonic=='mov' and i.op_str=='r8, qword ptr [rdx + 0x80]'),None); join=next((ins[k+1] for k,i in enumerate(ins[:-1]) if i.mnemonic=='call' and i.op_str=='0x142ed9d20'),None)
   if not (je and jmp and alt and join and int(je.op_str,0)==alt.address and int(jmp.op_str,0)==join.address): rejects['branch_targets']+=1; continue
   leaf=decode(target)[1]
   if leaf not in ('ret','ret 0'): rejects['leaf_not_exact_ret']+=1; continue
   ctype={'Name':'FName','Byte':'uint8','UInt32':'uint32'}[kind]
   sig=f'void {name}({ctype} {pname})'; hdr=headers[cls][0].read_text(); decl=re.search(r'^\s*[^\n;]*\b'+re.escape(name)+r'\s*\(',hdr,re.M)
   if decl and not re.search(r'^\s*'+re.escape(sig)+r'\s*;',hdr,re.M): rejects['function_header_mismatch']+=1; continue
   inc=HERE/f'{addr:x}_{cls}_{name}.inc'; body=f'// Original exec {hex(addr)}; complete empty leaf {hex(target)}.\n// CONFIDENCE: HIGH\n'
   if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs: '+str(inc))
   inc.write_text(body)
   entry={'class':cls,'function':name,'kind':'exec','signature':sig,'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':False,'access':F.ACCESS[flags&0xe0000],'notes':f'Exact FFrame extraction and direct call to complete ret-only leaf {hex(target)}; parameter ignored; source recovery only.'}
   results.append({'entry':entry,'parameter_kind':kind,'exec':asm,'leaf':leaf,'target':hex(target)})
 OUT.mkdir(parents=True,exist_ok=True); (OUT/'manifest-entries.json').write_text(json.dumps([x['entry'] for x in results],indent=2)+'\n')
 report={'image_sha256':digest,'inventory_records':total,'candidates':len(results),'by_kind':dict(collections.Counter(x['parameter_kind'] for x in results)),'rejections':dict(rejects),'results':results}
 (OUT/'report.json').write_text(json.dumps(report,indent=2)+'\n'); print(json.dumps({k:report[k] for k in ('inventory_records','candidates','by_kind','rejections')},indent=2))
if __name__=='__main__': main()
