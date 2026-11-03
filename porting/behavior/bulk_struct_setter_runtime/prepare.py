#!/usr/bin/env python3
"""Recover exact int32/float FFrame setters of reflected embedded struct members."""
import collections,hashlib,json,re
from pathlib import Path
import sys
sys.path.insert(0,'/tmp/common-input-capstone')
from capstone import Cs,CS_ARCH_X86,CS_MODE_64
ROOT=Path(__file__).resolve().parents[3]; HERE=Path(__file__).resolve().parent; REFL=ROOT/'porting/generated/reflection'; OUT=Path('/tmp/gameplay-bulk-struct-setter')
sys.path.insert(0,str(ROOT/'porting'))
import gen_native_classes as GEN   # split_properties: excludes enum/array/map/set metadata children
IMAGE=Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'); SHA='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'; BASE=0x140000000
load=lambda p:json.loads(p.read_text()); ACCESS={0x20000:'public',0x40000:'private',0x80000:'protected'}; NET=0x80000000|0x01000000|0x00200000|0x4000|0x1000|0x100|0x80|0x40
H=r'(0x[0-9a-f]+|[0-9]+)'
# Common head: Step/StepExplicitProperty (0x142ed9cf0 / 0x142ed9d20) into a zeroed float local, converge, advance P_FINISH.
HEAD=(r'mov qword ptr \[rsp \+ H\], rbx; push rdi; sub rsp, 0x20; cmp qword ptr \[rdx \+ 0x20\], 0; xorps xmm0, xmm0; mov rdi, rcx; movss dword ptr \[rsp \+ H\], xmm0; mov rbx, rdx; mov rcx, rdx; je (0x[0-9a-f]+); '
 r'mov rdx, qword ptr \[rdx \+ 0x18\]; lea r8, \[rsp \+ H\]; call 0x142ed9cf0; jmp (0x[0-9a-f]+); mov r8, qword ptr \[rdx \+ 0x80\]; mov rax, qword ptr \[r8 \+ 0x20\]; mov qword ptr \[rdx \+ 0x80\], rax; lea rdx, \[rsp \+ H\]; call 0x142ed9d20; '
 r'mov rax, qword ptr \[rbx \+ 0x20\]; xor ecx, ecx; movss (xmm0|xmm1), dword ptr \[rsp \+ H\]; test rax, rax; setne cl; add rcx, rax; mov qword ptr \[rbx \+ 0x20\], rcx; ').replace('H',H)
DIRECT=re.compile(HEAD+r'mov rcx, rdi; call (0x[0-9a-f]+); mov rbx, qword ptr \[rsp \+ H\]; add rsp, 0x20; pop rdi; ret'.replace('H',H))
INLINE=re.compile(HEAD+r'mov rbx, qword ptr \[rsp \+ H\]; movss dword ptr \[rdi( \+ 0x[0-9a-f]+)?\], xmm0; add rsp, 0x20; pop rdi; ret'.replace('H',H))
LEAF=re.compile(r'movss dword ptr \[rcx( \+ 0x[0-9a-f]+)?\], xmm1; ret')
import importlib.util
spec=importlib.util.spec_from_file_location('intsetter', ROOT/'porting/behavior/bulk_parameter_setter_runtime/prepare.py')
intsetter=importlib.util.module_from_spec(spec); spec.loader.exec_module(intsetter)

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
 cache={}; rejects=collections.Counter(); shapes=collections.Counter(); results=[]; total=0
 structs={x['block_va'].lower():x for x in load(REFL/'struct_blocks.json')['structs']}; cmap=load(REFL/'construct_fn_to_block.json')['map']
 used={(x.get('class'),x.get('function')) for x in load(ROOT/'porting/behavior/MANIFEST.json')}
 definitions=collections.defaultdict(set)
 for f in (ROOT/'porting/native/Source').rglob('*.cpp'):
  for c,n in re.findall(r'\b([A-Za-z_]\w*)::([A-Za-z_]\w*)\s*\(',f.read_text(errors='replace')): definitions[c].add(n)
 def members(cls_name):
  v=byname.get(cls_name,[])
  b=blocks.get(v[0].lower()) if len(v)==1 else None
  if not b: return None
  pairs,notes=GEN.split_properties(b['properties'])
  return [(p,ch) for p,ch in pairs]
 for cls,funcs in sorted(native.items()):
  for name,fn in sorted(funcs.items()):
   total+=1
   if not isinstance(fn,dict) or not isinstance(fn.get('params'),list) or len(fn['params'])!=1: rejects['not_one_parameter']+=1; continue
   pp,pnotes=GEN.split_properties(fn['params'])
   if pnotes or len(pp)!=1 or pp[0][1]: rejects['parameter_metadata']+=1; continue
   p=pp[0][0]; pname=p.get('name','')
   kind=p.get('kind'); typ={'Int':'int32','Float':'float'}.get(kind)
   if not typ: rejects['parameter_not_scalar']+=1; continue
   pf=int(p.get('property_flags','0'),0)
   if p.get('array_dim')!=1 or p.get('type_refs') or pf!=0x10000000000080 or p.get('kind_raw')!={'Int':'0x3','Float':'0xa'}[kind] or p.get('offset')!=0 or p.get('replicated') is not False or p.get('rep_notify') is not None or any(k in p for k in ('array_inner_of','map_key_of','map_value_of','set_element_of')) or not re.fullmatch(r'[A-Za-z_]\w*',pname): rejects['parameter_flags_or_name']+=1; continue
   try: flags=int(fn['flags'],0); addr=int(fn['exec_addr'],0)
   except (KeyError,ValueError,TypeError): rejects['address_or_flags']+=1; continue
   if flags & (~0xffffffcf|NET|0x08000000|0x20000000|0x2000|0x40000000) or not flags&0x400 or (flags&0xe0000) not in ACCESS: rejects['function_flags']+=1; continue
   if (cls,name) in used or (cls,name+'_Implementation') in used or name in definitions[cls] or name+'_Implementation' in definitions[cls]: rejects['already_ported']+=1; continue
   vs=byname.get(cls,[])
   if len(vs)!=1 or cls not in selected or not re.fullmatch(r'[A-Za-z_]\w*',name): rejects['class_or_identifier']+=1; continue
   par=parents.get(vs[0],{}); block=blocks.get(vs[0].lower())
   if not block or par.get('name')!=cls or par.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000: rejects['class_metadata']+=1; continue
   if len(headers[cls])!=1: rejects['class_header']+=1; continue
   chdr=headers[cls][0].read_text()
   if re.search(r'\b'+re.escape(name)+r'(?:_Implementation)?\s*\(',chdr): rejects['existing_declaration']+=1; continue
   ins,asm=decode(addr); target=None
   if kind=='Int':
    m=intsetter.PAT.fullmatch(asm)
    if not m: rejects['unmatched_thunk']+=1; continue
    g=m.groups(); stack=g[3]
    if g[2:5]!=('dword',stack,'edi') or g[6]!=stack or g[8]!=stack or g[9:11]!=('edx','dword') or g[11]!=stack or int(g[0],0)+0x28!=int(g[13],0) or int(g[1],0)+0x28!=int(g[14],0) or int(stack,0) in (int(g[13],0),int(g[14],0)) or not 0x20<=int(stack,0)<0x40: rejects['stack_or_register']+=1; continue
    target=int(g[12],0); leaf=decode(target)[1]; q=re.fullmatch(r'mov dword ptr \[rcx \+ (0x[0-9a-f]+)\], edx; ret',leaf)
    if not q: rejects['unmatched_leaf']+=1; continue
    offset=int(q.group(1),0); mode='direct'
   else:
    m=DIRECT.fullmatch(asm); mode='direct'
    if not m: m=INLINE.fullmatch(asm); mode='inline'
    if not m: rejects['unmatched_thunk']+=1; continue
    g=m.groups(); save,loc,reg=g[0],g[1],g[6]
    if not loc==g[3]==g[5]==g[7]: rejects['stack_or_register']+=1; continue
    if mode=='direct':
     target=int(g[8],0); restore=g[9]
     if reg!='xmm1': rejects['stack_or_register']+=1; continue
     leaf=decode(target)[1]; q=LEAF.fullmatch(leaf)
     if not q: rejects['unmatched_leaf']+=1; continue
     imm=q.group(1)
    else:
     restore=g[8]; imm=g[9]; leaf='(inline)'
     if reg!='xmm0': rejects['stack_or_register']+=1; continue
    if int(save,0)+0x28!=int(restore,0) or int(loc,0)==int(restore,0) or not 0x20<=int(loc,0)<0x40: rejects['stack_or_register']+=1; continue
    offset=int(imm.split('+')[1],16) if imm else 0
   je=next((i for i in ins if i.mnemonic=='je'),None); jmp=next((i for i in ins if i.mnemonic=='jmp'),None)
   alt=next((i for i in ins if i.mnemonic=='mov' and i.op_str=='r8, qword ptr [rdx + 0x80]'),None); join=next((ins[k+1] for k,i in enumerate(ins[:-1]) if i.mnemonic=='call' and i.op_str=='0x142ed9d20'),None)
   if not (je and jmp and alt and join and int(je.op_str,0)==alt.address and int(jmp.op_str,0)==join.address): rejects['branch_targets']+=1; continue
   candidates=[]; summed_matches=[]
   for outer in block.get('properties',[]):
    if outer.get('kind')!='Struct' or outer.get('array_dim')!=1 or any(k in outer for k in ('array_inner_of','map_key_of','map_value_of','set_element_of')) or outer.get('kind_raw')!='0x19' or set(outer.get('type_refs') or {})!={'struct'} or not isinstance(outer.get('offset'),int): continue
    a=outer.get('name',''); rec=cmap.get((outer.get('type_refs') or {}).get('struct','').lower(),{}); st=structs.get(rec.get('block_va','').lower())
    if not st or rec.get('kind')!='struct' or not isinstance(st.get('size'),int) or not outer['offset']<=offset<outer['offset']+st['size']: continue
    stname='F'+st['name']; sh=headers.get(stname,[])
    if len(sh)!=1 or sum(1 for z in structs.values() if z.get('name')==st['name'])!=1 or not re.fullmatch(r'[A-Za-z_]\w*',a) or not re.search(r'\b'+re.escape(stname)+r'\s+'+re.escape(a)+r'\s*;',chdr): continue
    stxt=sh[0].read_text(); classmod=headers[cls][0].parent.parent.name; structmod=sh[0].parent.parent.name
    if par.get('package')!='/Script/'+classmod or st.get('package')!='/Script/'+structmod: continue
    if classmod!=structmod:
     build=headers[cls][0].parent.parent/(classmod+'.Build.cs')
     if not build.is_file() or not re.search(r'^\s*#include\s+"'+re.escape(sh[0].name)+r'"\s*$',chdr,re.M): continue
     dep=re.search(r'\bPublicDependencyModuleNames\s*\.\s*AddRange\s*\(\s*new\[\]\s*\{([^}]*)\}\s*\)',build.read_text(),re.S)
     if not dep or '"'+structmod+'"' not in dep.group(1) or not re.search(r'\bstruct\s+'+re.escape(structmod.upper()+'_API')+r'\s+'+re.escape(stname)+r'\b',stxt): continue
    for inner in st.get('properties',[]):
     if isinstance(inner.get('offset'),int) and outer['offset']+inner['offset']==offset: summed_matches.append((a,inner.get('name')))
     b=inner.get('name','')
     if inner.get('kind')!=kind or inner.get('array_dim')!=1 or any(k in inner for k in ('array_inner_of','map_key_of','map_value_of','set_element_of')) or inner.get('type_refs') or inner.get('kind_raw')!={'Int':'0x3','Float':'0xa'}[kind] or inner.get('offset')!=offset-outer['offset'] or not re.fullmatch(r'[A-Za-z_]\w*',b): continue
     if re.search(r'\b'+typ+r'\s+'+re.escape(b)+r'\s*;',stxt): candidates.append((a,b,stname,outer['offset'],inner['offset']))
   if len(candidates)!=1 or len(summed_matches)!=1: rejects['no_unique_struct_member']+=1; continue
   a,b,stname,outeroff,inneroff=candidates[0]; sig=f'void {name}({typ} {pname})'; ev=f'{mode} pure store'
   inc=HERE/f'{addr:x}_{cls}_{name}.inc'; body=f'// Exact original embedded struct scalar setter; see README.md.\n// CONFIDENCE: HIGH\nthis->{a}.{b} = {pname};\n'
   if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs: '+str(inc))
   inc.write_text(body)
   entry={'class':cls,'function':name,'kind':'exec','signature':sig,'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':False,'access':ACCESS[flags&0xe0000],'notes':f'Exact FFrame extraction and complete {mode} scalar store to reflected {a}.{b}; see bulk_struct_setter_runtime/README.md.'}
   results.append({'entry':entry,'mode':mode,'exec':asm,'writer':leaf,'target':hex(target) if target else None,'struct':stname,'outer_offset':outeroff,'member_offset':inneroff,'offset':offset})
 OUT.mkdir(parents=True,exist_ok=True); (OUT/'manifest-entries.json').write_text(json.dumps([x['entry'] for x in results],indent=2)+'\n')
 report={'image_sha256':digest,'inventory_records':total,'candidates':len(results),'by_mode':dict(collections.Counter(x['mode'] for x in results)),'rejections':dict(rejects),'top_unmatched_shapes':shapes.most_common(8),'results':results}
 (OUT/'report.json').write_text(json.dumps(report,indent=2)+'\n'); print(json.dumps({k:report[k] for k in ('inventory_records','candidates','by_mode','rejections')},indent=2))
if __name__=='__main__': main()
