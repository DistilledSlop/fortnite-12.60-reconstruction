#!/usr/bin/env python3
"""Recover exact Boolean setter thunks with conditional full-byte stores.
Outputs source proposals only; no canonical manifests or generated Native edits."""
import collections,hashlib,json,re
from pathlib import Path
import sys
sys.path.insert(0,'/tmp/common-input-capstone')
from capstone import Cs,CS_ARCH_X86,CS_MODE_64
ROOT=Path(__file__).resolve().parents[3]; HERE=Path(__file__).resolve().parent; REFL=ROOT/'porting/generated/reflection'; OUT=Path('/tmp/gameplay-bulk-dirty-scalar')
sys.path.insert(0,str(ROOT/'porting'))
import gen_native_classes as GEN   # split_properties: excludes enum/array/map/set metadata children
IMAGE=Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'); SHA='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'; BASE=0x140000000
load=lambda p:json.loads(p.read_text()); ACCESS={0x20000:'public',0x40000:'private',0x80000:'protected'}; NET=0x80000000|0x01000000|0x00200000|0x4000|0x1000|0x100|0x80|0x40
H=r'(0x[0-9a-f]+|[0-9]+)'
# Complete FFrame extraction, Boolean normalization, direct call, and epilogue.
THUNK=re.compile(r'mov qword ptr \[rsp \+ 8\], rbx; mov qword ptr \[rsp \+ 0x18\], rsi; push rdi; sub rsp, 0x20; xor edi, edi; mov rsi, rcx; mov rbx, rdx; mov dword ptr \[rsp \+ 0x38\], edi; mov rcx, rdx; cmp qword ptr \[rdx \+ 0x20\], rdi; je (0x[0-9a-f]+); mov rdx, qword ptr \[rdx \+ 0x18\]; lea r8, \[rsp \+ 0x38\]; call 0x142ed9cf0; jmp (0x[0-9a-f]+); mov r8, qword ptr \[rdx \+ 0x80\]; mov rax, qword ptr \[r8 \+ 0x20\]; mov qword ptr \[rdx \+ 0x80\], rax; lea rdx, \[rsp \+ 0x38\]; call 0x142ed9d20; mov rax, qword ptr \[rbx \+ 0x20\]; mov rcx, rsi; test rax, rax; setne dil; add rdi, rax; cmp dword ptr \[rsp \+ 0x38\], 0; mov qword ptr \[rbx \+ 0x20\], rdi; setne dl; call (0x[0-9a-f]+); mov rbx, qword ptr \[rsp \+ 0x30\]; mov rsi, qword ptr \[rsp \+ 0x40\]; add rsp, 0x20; pop rdi; ret')
LEAF=re.compile(r'cmp dl, byte ptr \[rcx \+ (0x[0-9a-f]+)\]; je (0x[0-9a-f]+); mov byte ptr \[rcx \+ (0x[0-9a-f]+)\], dl; mov byte ptr \[rcx \+ (0x[0-9a-f]+)\], 1; ret')
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
 maintained=set()
 for cpp in (ROOT/'porting/native/Source').rglob('*.cpp'):
  if 'Generated' in cpp.parts: continue
  maintained.update(re.findall(r'\b([A-Za-z_]\w*)::([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?\{',cpp.read_text(errors='replace')))
 cache={}; rejects=collections.Counter(); shapes=collections.Counter(); results=[]; total=0
 bits=load(REFL/'bool_bits.json')['by_class']
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
   if p.get('kind')!='Bool': rejects['parameter_not_bool']+=1; continue
   pf=int(p.get('property_flags','0'),0)
   if p.get('array_dim')!=1 or p.get('type_refs') or pf&(0x400|0x100|0x08000000) or not pf&0x80 or not re.fullmatch(r'[A-Za-z_]\w*',pname): rejects['parameter_flags_or_name']+=1; continue
   try: flags=int(fn['flags'],0); addr=int(fn['exec_addr'],0)
   except (KeyError,ValueError,TypeError): rejects['address_or_flags']+=1; continue
   if flags & (~0xffffffcf|NET|0x08000000|0x20000000|0x2000|0x40000000) or not flags&0x400 or (flags&0xe0000) not in ACCESS: rejects['function_flags']+=1; continue
   if (cls,name) in ported or (cls,name) in maintained: rejects['already_ported_or_maintained']+=1; continue
   vs=byname.get(cls,[])
   if len(vs)!=1 or cls not in selected or not re.fullmatch(r'[A-Za-z_]\w*',name): rejects['class_or_identifier']+=1; continue
   par=parents.get(vs[0],{}); block=blocks.get(vs[0].lower())
   if not block or par.get('name')!=cls or par.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000: rejects['class_metadata']+=1; continue
   ins,asm=decode(addr)
   m=THUNK.fullmatch(asm)
   if not m: rejects['unmatched_thunk_shape']+=1; continue
   g=m.groups(); target=int(g[2],0); leafins,leaf=decode(target); lm=LEAF.fullmatch(leaf)
   if not lm: rejects['unmatched_leaf_shape']+=1; continue
   lg=lm.groups()
   if lg[0]!=lg[2] or int(lg[1],0)!=leafins[-1].address: rejects['bad_leaf_branch_or_offset']+=1; continue
   alt=next((i for i in ins if i.mnemonic=='mov' and i.op_str=='r8, qword ptr [rdx + 0x80]'),None)
   join=next((ins[k+1] for k,i in enumerate(ins[:-1]) if i.mnemonic=='call' and i.op_str=='0x142ed9d20'),None)
   if not (alt and join and int(g[0],0)==alt.address and int(g[1],0)==join.address): rejects['bad_branch_targets']+=1; continue
   offset=int(lg[0],0); dirty_offset=int(lg[3],0); mode='bool_leaf'
   field_bits=[(a,x) for a in par.get('chain',[]) for x in bits.get(a,[]) if x.get('offset')==offset]
   if len(field_bits)!=1 or field_bits[0][1].get('form')!='mov8' or field_bits[0][1].get('mask')!=1: rejects['field_bool_not_full_byte']+=1; continue
   bit_owner,bit=field_bits[0]
   fields=[(bit_owner,x) for x,ch in (members(bit_owner) or []) if x.get('name')==bit.get('name') and x.get('kind')=='Bool' and x.get('array_dim')==1 and not ch]
   overlaps=[(a,x) for a in par.get('chain',[]) for x,ch in (members(a) or []) if x.get('offset')==offset and not ch]
   if len(fields)!=1 or overlaps: rejects['field_missing_ambiguous_or_type']+=1; continue
   owner,field=fields[0]; prop=field.get('name','')
   if cls not in par.get('chain',[]) or not re.fullmatch(r'[A-Za-z_]\w*',prop) or len(headers[owner])!=1 or len(headers[cls])!=1: rejects['header_or_field']+=1; continue
   hdr=cache.setdefault(owner,headers[owner][0].read_text()); chdr=cache.setdefault(cls,headers[cls][0].read_text())
   fm=re.search(r'^\s*bool\s+'+re.escape(prop)+r'\s*;',hdr,re.M)
   if not fm: rejects['field_header']+=1; continue
   labels=list(re.finditer(r'^\s*(public|protected|private)\s*:',hdr[:fm.start()],re.M))
   if owner!=cls and (not labels or labels[-1].group(1)=='private'): rejects['inherited_access']+=1; continue
   sig=f'void {name}(bool {pname})'
   decl=re.search(r'^\s*[^\n;]*\b'+re.escape(name)+r'\s*\(',chdr,re.M)
   if decl and not re.search(r'^\s*'+re.escape(sig)+r'\s*;',chdr,re.M): rejects['function_header_mismatch']+=1; continue
   field_bits=[(a,x) for a in par.get('chain',[]) for x in bits.get(a,[]) if x.get('offset')==offset]
   if len(field_bits)!=1 or field_bits[0][0]!=owner or field_bits[0][1].get('name')!=prop or field_bits[0][1].get('form')!='mov8' or field_bits[0][1].get('mask')!=1: rejects['field_bool_not_full_byte']+=1; continue
   if offset==dirty_offset: rejects['dirty_aliases_field']+=1; continue
   dirty_candidates=[(a,x) for a in par.get('chain',[]) for x in bits.get(a,[]) if x.get('offset')==dirty_offset]
   if any(x.get('offset')==dirty_offset for a in par.get('chain',[]) for x,ch in (members(a) or []) if not ch): rejects['dirty_overlapping_property']+=1; continue
   if len(dirty_candidates)!=1: rejects['dirty_bool_missing_or_ambiguous']+=1; continue
   dirty_owner,db=dirty_candidates[0]; dirty=db.get('name','')
   if db.get('form')!='mov8' or db.get('mask')!=1 or not re.fullmatch(r'[A-Za-z_]\w*',dirty) or len(headers[dirty_owner])!=1: rejects['dirty_bool_not_full_byte']+=1; continue
   dmprops=[x for x,ch in (members(dirty_owner) or []) if x.get('name')==dirty and x.get('kind')=='Bool' and x.get('array_dim')==1 and not ch]
   if len(dmprops)!=1: rejects['dirty_bool_reflection_mismatch']+=1; continue
   dhdr=cache.setdefault(dirty_owner,headers[dirty_owner][0].read_text())
   dm=re.search(r'^\s*bool\s+'+re.escape(dirty)+r'\s*;',dhdr,re.M)
   labels=list(re.finditer(r'^\s*(public|protected|private)\s*:',dhdr[:dm.start()] if dm else '',re.M))
   if not dm or (dirty_owner!=cls and (not labels or labels[-1].group(1)=='private')): rejects['dirty_bool_header_or_access']+=1; continue
   inc=HERE/f'{addr:x}_{cls}_{name}.inc'; body=f'// Original exec {hex(addr)}, leaf {hex(target)}; conditional Boolean and dirty-byte stores.\n// CONFIDENCE: HIGH\nif (this->{prop} != {pname})\n{{\n    this->{prop} = {pname};\n    this->{dirty} = true;\n}}\n'
   if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs: '+str(inc))
   inc.write_text(body)
   entry={'class':cls,'function':name,'kind':'exec','signature':sig,'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':False,'access':ACCESS[flags&0xe0000],'notes':f'Exact FFrame Boolean extraction and complete leaf {hex(target)}, conditional full-byte bool stores {owner}.{prop} offset {hex(offset)} and {dirty_owner}.{dirty} offset {hex(dirty_offset)}; source recovery only.'}
   results.append({'entry':entry,'mode':mode,'exec':asm,'writer':leaf,'target':hex(target),'field_owner':owner,'offset':offset,'dirty_offset':dirty_offset})
 OUT.mkdir(parents=True,exist_ok=True); (OUT/'manifest-entries.json').write_text(json.dumps([x['entry'] for x in results],indent=2)+'\n')
 report={'image_sha256':digest,'inventory_records':total,'candidates':len(results),'by_mode':dict(collections.Counter(x['mode'] for x in results)),'rejections':dict(rejects),'top_unmatched_shapes':shapes.most_common(8),'results':results}
 (OUT/'report.json').write_text(json.dumps(report,indent=2)+'\n'); print(json.dumps({k:report[k] for k in ('inventory_records','candidates','by_mode','rejections')},indent=2))
if __name__=='__main__': main()
