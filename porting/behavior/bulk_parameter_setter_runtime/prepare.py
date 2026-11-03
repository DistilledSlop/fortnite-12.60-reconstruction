#!/usr/bin/env python3
"""Recover direct scalar setters from exact one-argument execs and pure leaf writers."""
import collections,hashlib,json,re
from pathlib import Path
import sys
sys.path.insert(0,'/tmp/common-input-capstone')
from capstone import Cs,CS_ARCH_X86,CS_MODE_64
sys.path.insert(0,str(Path(__file__).resolve().parents[2]))
from gen_native_classes import split_properties
ROOT=Path(__file__).resolve().parents[3]; HERE=Path(__file__).resolve().parent; REFL=ROOT/'porting/generated/reflection'; OUT=Path('/tmp/gameplay-bulk-parameter-setter')
IMAGE=Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'); SHA='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'; BASE=0x140000000
load=lambda p:json.loads(p.read_text()); ACCESS={0x20000:'public',0x40000:'private',0x80000:'protected'}; NET=0x80000000|0x01000000|0x00200000|0x4000|0x1000|0x100|0x80|0x40
# Exact x64 code form: the two branches implement Step/StepExplicitProperty, converge at P_FINISH.
HEAD=r'mov qword ptr \[rsp \+ (0x[0-9a-f]+|[0-9]+)\], rbx; mov qword ptr \[rsp \+ (0x[0-9a-f]+|[0-9]+)\], rsi; push rdi; sub rsp, 0x20; xor edi, edi; mov rsi, rcx; mov rbx, rdx; mov (dword|qword) ptr \[rsp \+ (0x[0-9a-f]+|[0-9]+)\], (edi|rdi); mov rcx, rdx; cmp qword ptr \[rdx \+ 0x20\], rdi; je (0x[0-9a-f]+); mov rdx, qword ptr \[rdx \+ 0x18\]; lea r8, \[rsp \+ (0x[0-9a-f]+|[0-9]+)\]; call 0x142ed9cf0; jmp (0x[0-9a-f]+); mov r8, qword ptr \[rdx \+ 0x80\]; mov rax, qword ptr \[r8 \+ 0x20\]; mov qword ptr \[rdx \+ 0x80\], rax; lea rdx, \[rsp \+ (0x[0-9a-f]+|[0-9]+)\]; call 0x142ed9d20; '
TAIL=r'mov rax, qword ptr \[rbx \+ 0x20\]; mov rcx, rsi; mov (edx|rdx), (dword|qword) ptr \[rsp \+ (0x[0-9a-f]+|[0-9]+)\]; test rax, rax; setne dil; add rdi, rax; mov qword ptr \[rbx \+ 0x20\], rdi; call (0x[0-9a-f]+); mov rbx, qword ptr \[rsp \+ (0x[0-9a-f]+|[0-9]+)\]; mov rsi, qword ptr \[rsp \+ (0x[0-9a-f]+|[0-9]+)\]; add rsp, 0x20; pop rdi; ret'
PAT=re.compile(HEAD+TAIL)
def main():
 img=IMAGE.read_bytes(); digest=hashlib.sha256(img).hexdigest()
 if digest!=SHA: raise SystemExit('image digest mismatch')
 md=Cs(CS_ARCH_X86,CS_MODE_64)
 def decode(a):
  if a<BASE or a-BASE+256>len(img): return [],''
  ins=[]
  for i in md.disasm(img[a-BASE:a-BASE+256],a):
   ins.append(i)
   if i.mnemonic=='ret': return ins,'; '.join((x.mnemonic+' '+x.op_str).rstrip() for x in ins)
  return [],''
 native=load(REFL/'native_funcs.json'); names=load(REFL/'class_name_map.json'); parents=load(REFL/'class_parent_map.json'); blocks={x['block_va'].lower():x for x in load(REFL/'fortnite_game_reflection.json')['classes_in_groups_with_fort_names']}; byname=collections.defaultdict(list)
 for va,n in names.items(): byname[n].append(va)
 selected={x.strip() for x in (ROOT/'porting/native_class_set.txt').read_text().splitlines() if x.strip() and not x.lstrip().startswith('#')}; ported={(x.get('class'),x.get('function')) for x in load(ROOT/'porting/behavior/MANIFEST.json') if x.get('status')=='ported'}
 headers=collections.defaultdict(list)
 for module in ('FortniteGame','FortniteUI'):
  for p in (ROOT/'porting/native/Source'/module/'Generated').glob('*.h'): headers[p.stem].append(p)
 cache={}; rejects=collections.Counter(); results=[]; total=0
 for cls,funcs in sorted(native.items()):
  for name,fn in sorted(funcs.items()):
   total+=1
   if not isinstance(fn,dict) or not isinstance(fn.get('params'),list) or len(fn['params'])!=1: rejects['not_one_parameter']+=1; continue
   p=fn['params'][0]; kind=p.get('kind'); pname=p.get('name','')
   if kind!='Int' or p.get('array_dim')!=1 or p.get('type_refs') or int(p.get('property_flags','0'),0)&(0x400|0x100|0x08000000) or not re.fullmatch(r'[A-Za-z_]\w*',pname): rejects['parameter_kind_or_flags']+=1; continue
   try: flags=int(fn['flags'],0); addr=int(fn['exec_addr'],0)
   except (KeyError,ValueError,TypeError): rejects['address_or_flags']+=1; continue
   if flags & (~0xffffffcf|NET|0x08000000|0x20000000|0x2000|0x40000000) or not flags&0x400 or (flags&0xe0000) not in ACCESS: rejects['function_flags']+=1; continue
   if (cls,name) in ported: rejects['already_ported']+=1; continue
   vs=byname.get(cls,[])
   if len(vs)!=1 or cls not in selected or not re.fullmatch(r'[A-Za-z_]\w*',name): rejects['class_or_identifier']+=1; continue
   par=parents.get(vs[0],{}); block=blocks.get(vs[0].lower())
   if not block or par.get('name')!=cls or par.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000: rejects['class_metadata']+=1; continue
   ins,asm=decode(addr); m=PAT.fullmatch(asm)
   if not m: rejects['unmatched_thunk_float_bool_object_name_or_other']+=1; continue
   g=m.groups(); stack=g[3]
   if g[2:5] != ('dword',stack,'edi') or g[6]!=stack or g[8]!=stack or g[9:11]!=('edx','dword') or g[11]!=stack or int(g[0],0)+0x28!=int(g[13],0) or int(g[1],0)+0x28!=int(g[14],0): rejects['inconsistent_thunk_register_or_stack']+=1; continue
   # Conditional and unconditional jumps must land precisely on the alternate branch and join.
   je=next((i for i in ins if i.mnemonic=='je'),None); jmp=next((i for i in ins if i.mnemonic=='jmp'),None)
   alt=next((i for i in ins if i.mnemonic=='mov' and i.op_str=='r8, qword ptr [rdx + 0x80]'),None); join=next((ins[k+1] for k,i in enumerate(ins[:-1]) if i.mnemonic=='call' and i.op_str=='0x142ed9d20'),None)
   if not (je and jmp and alt and join and int(je.op_str,0)==alt.address and int(jmp.op_str,0)==join.address): rejects['bad_branch_targets']+=1; continue
   target=int(g[12],0); leaf=decode(target)[1]; q=re.fullmatch(r'mov dword ptr \[rcx \+ (0x[0-9a-f]+)\], edx; ret',leaf)
   if not q: rejects['writer_not_pure_single_int_store']+=1; continue
   offset=int(q.group(1),0); fields=[]
   for ancestor in par.get('chain',[]):
    av=byname.get(ancestor,[])
    if len(av)!=1: continue
    b=blocks.get(av[0].lower())
    if b: fields.extend((ancestor,x) for x,children in split_properties(b['properties'])[0] if not children and x.get('offset')==offset and x.get('array_dim')==1)
   if len(fields)!=1 or fields[0][1].get('kind')!='Int' or fields[0][1].get('type_refs'): rejects['field_missing_ambiguous_or_type']+=1; continue
   owner,field=fields[0]; prop=field.get('name','')
   if not re.fullmatch(r'[A-Za-z_]\w*',prop) or len(headers[owner])!=1 or len(headers[cls])!=1: rejects['header_or_field']+=1; continue
   hdr=cache.setdefault(owner,headers[owner][0].read_text()); chdr=cache.setdefault(cls,headers[cls][0].read_text())
   if not re.search(r'^\s*int32\s+'+re.escape(prop)+r'\s*;',hdr,re.M): rejects['field_header']+=1; continue
   # Inherited private storage is inaccessible; inspect the last access label before declaration.
   pos=re.search(r'^\s*int32\s+'+re.escape(prop)+r'\s*;',hdr,re.M).start(); labels=list(re.finditer(r'^\s*(public|protected|private)\s*:',hdr[:pos],re.M))
   if owner!=cls and (not labels or labels[-1].group(1)=='private'): rejects['inherited_access']+=1; continue
   sig=f'void {name}(int32 {pname})'
   decl=re.search(r'^\s*[^\n;]*\b'+re.escape(name)+r'\s*\(',chdr,re.M)
   if decl and not re.search(r'^\s*'+re.escape(sig)+r'\s*;',chdr,re.M): rejects['function_header_mismatch']+=1; continue
   inc=HERE/f'{addr:x}_{cls}_{name}.inc'; body=f'// Original exec {hex(addr)}; pure writer {hex(target)}.\n// CONFIDENCE: HIGH\nthis->{prop} = {pname};\n'
   if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs: '+str(inc))
   inc.write_text(body)
   entry={'class':cls,'function':name,'kind':'exec','signature':sig,'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':False,'access':ACCESS[flags&0xe0000],'notes':f'Exact FFrame extraction and pure int32 store at {hex(target)} to {owner}.{prop} offset {hex(offset)}; source recovery only.'}
   results.append({'entry':entry,'exec':asm,'writer':leaf,'target':hex(target),'field_owner':owner,'offset':offset})
 OUT.mkdir(parents=True,exist_ok=True); (OUT/'manifest-entries.json').write_text(json.dumps([x['entry'] for x in results],indent=2)+'\n'); report={'image_sha256':digest,'inventory_records':total,'candidates':len(results),'rejections':dict(rejects),'results':results}; (OUT/'report.json').write_text(json.dumps(report,indent=2)+'\n'); print(json.dumps({k:report[k] for k in ('inventory_records','candidates','rejections')},indent=2))
if __name__=='__main__': main()
