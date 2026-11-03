#!/usr/bin/env python3
"""Conservative inherited scalar getter recovery; writes only local proposals and scratch report."""
import argparse, collections, hashlib, json, math, re, struct, sys
from pathlib import Path
sys.path.insert(0, '/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
ROOT=Path(__file__).resolve().parents[3]; REFL=ROOT/'porting/generated/reflection'
HERE=Path(__file__).resolve().parent
IMAGE=Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
BASE=0x140000000; SHA='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
load=lambda p: json.loads(p.read_text())
PREFIX='mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
WRAP='push rbx; sub rsp, 0x20; mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; mov rbx, r8; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
NET=0x80000000|0x01000000|0x00200000|0x4000|0x1000|0x100|0x80|0x40
ACCESS={0x20000:'public',0x40000:'private',0x80000:'protected'}
def main():
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('--revalidate-own',action='store_true',help='rescan ported entries belonging to bulk_inherited_runtime')
 parser.add_argument('--output-dir',type=Path,default=Path('/tmp/gameplay-bulk-inherited'),help='directory for the proposal manifest and report')
 args=parser.parse_args()
 image=IMAGE.read_bytes(); digest=hashlib.sha256(image).hexdigest()
 if digest!=SHA: raise SystemExit('image digest mismatch')
 md=Cs(CS_ARCH_X86,CS_MODE_64)
 def decode(addr):
  pos=addr-BASE
  if pos<0 or pos+128>len(image): return ''
  ins=[]
  for i in md.disasm(image[pos:pos+128],addr):
   ins.append(i.mnemonic+' '+i.op_str)
   if i.mnemonic=='ret': return '; '.join(x.rstrip() for x in ins)
   if i.mnemonic.startswith('j'): return ''
  return ''
 native=load(REFL/'native_funcs.json'); names=load(REFL/'class_name_map.json'); parents=load(REFL/'class_parent_map.json')
 reflection=load(REFL/'fortnite_game_reflection.json'); bits=load(REFL/'bool_bits.json')['by_class']
 blocks={c['block_va'].lower():c for c in reflection['classes_in_groups_with_fort_names']}
 byname=collections.defaultdict(list)
 for va,name in names.items(): byname[name].append(va)
 selected={x.strip() for x in (ROOT/'porting/native_class_set.txt').read_text().splitlines() if x.strip() and not x.lstrip().startswith('#')}
 own_path=str(HERE.relative_to(ROOT))+'/'
 ported={(e.get('class'),e.get('function')) for e in load(ROOT/'porting/behavior/MANIFEST.json') if e.get('status')=='ported' and not (args.revalidate_own and (e.get('inc_path') or '').startswith(own_path))}
 def ancestry(va):
  """Follow only original selected FG/FUI ancestors also present in the C++ base chain."""
  chain=[]; seen=set()
  while va:
   if va in seen: return []
   seen.add(va)
   meta=parents.get(va,{}); owner=names.get(va); block=blocks.get(va.lower())
   if meta.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI'): break
   if not owner or len(byname.get(owner,[]))!=1 or owner not in selected or not block or not meta.get('chain_ok') or int(block['class_flags'],0)&0x4000: return []
   hp=list((ROOT/'porting/native/Source').glob('*/Generated/'+owner+'.h'))
   if len(hp)!=1: return []
   header=hp[0].read_text()
   base=re.search(r'class\s+\w+_API\s+'+re.escape(owner)+r'\s*:\s*public\s+(\w+)',header)
   if not base or base.group(1)!=meta.get('parent_name'): return []
   chain.append((owner,block,header))
   va=meta.get('parent_va')
  return chain
 ancestry_cache={}
 results=[]; rejects=collections.Counter(); total=0
 for cls,funcs in sorted(native.items()):
  for name,fn in sorted(funcs.items()):
   total+=1
   def reject(reason): rejects[reason]+=1
   if not isinstance(fn,dict): reject('invalid_record'); continue
   ps=fn.get('params')
   if not isinstance(ps,list) or len(ps)!=1 or ps[0].get('name')!='ReturnValue' or int(ps[0].get('property_flags','0'),0)&0x400==0: reject('not_zero_arg_getter'); continue
   if ps[0].get('array_dim') != 1 or int(ps[0].get('property_flags','0'),0) & 0x08000000:
    reject('return_reference_or_array'); continue
   kind=ps[0].get('kind')
   if kind not in ('Int','Float','Byte','Bool'): reject('unsupported_return_type'); continue
   if kind=='Byte' and ps[0].get('type_refs'): reject('typed_byte_unsupported'); continue
   try: flags=int(fn['flags'],0); addr=int(fn['exec_addr'],0)
   except (ValueError,TypeError,KeyError): reject('missing_flags_or_address'); continue
   if flags & (~0xffffffcf|NET|0x08000000|0x20000000|0x2000) or not flags&0x400 or (flags&0xe0000) not in ACCESS: reject('flags'); continue
   if (cls,name) in ported: reject('already_ported'); continue
   vs=byname.get(cls,[])
   if len(vs)!=1 or cls not in selected: reject('class_not_unique_or_selected'); continue
   va=vs[0]; parent=parents.get(va,{}); block=blocks.get(va.lower())
   if not block or parent.get('name')!=cls or parent.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000: reject('class_or_package'); continue
   if not re.fullmatch(r'[A-Za-z_]\w*',name): reject('identifier'); continue
   asm=decode(addr); inner=None; target=None
   if asm.startswith(PREFIX): inner=asm[len(PREFIX):]; form='inline'
   elif asm.startswith('mov rax, qword ptr [rdx + 0x20]; xor ecx, ecx; test rax, rax; setne cl; add rcx, rax; mov qword ptr [rdx + 0x20], rcx; '):
    inner=asm.split('mov qword ptr [rdx + 0x20], rcx; ',1)[1]; form='inline_constant'
   elif asm.startswith(WRAP):
    m=re.fullmatch(r'call (0x[0-9a-f]+); (mov (?:byte|dword) ptr \[rbx\], (?:al|eax)|movss dword ptr \[rbx\], xmm0); add rsp, 0x20; pop rbx; ret',asm[len(WRAP):])
    if m:
     target=int(m.group(1),16); pure=decode(target)
     w=m.group(2)
     patterns=[(r'mov eax, dword ptr \[rcx \+ (0x[0-9a-f]+)\]; ret','mov dword ptr [rbx], eax','dword'),(r'movss xmm0, dword ptr \[rcx \+ (0x[0-9a-f]+)\]; ret','movss dword ptr [rbx], xmm0','dword'),(r'movzx eax, byte ptr \[rcx \+ (0x[0-9a-f]+)\]; ret','mov byte ptr [rbx], al','byte'),(r'movzx eax, byte ptr \[rcx \+ (0x[0-9a-f]+)\]; and al, (0x[0-9a-f]+|[0-9]+); ret','mov byte ptr [rbx], al','bit')]
     shift=re.fullmatch(r'movzx eax, byte ptr \[rcx \+ (0x[0-9a-f]+)\]; shr (al|eax), (0x[0-9a-f]+|[0-9]+); and (al|eax), 1; ret', pure)
     if shift and shift.group(2)==shift.group(4) and w=='mov byte ptr [rbx], al' and 1<=int(shift.group(3),0)<=7:
      inner=('shift',int(shift.group(1),0),1<<int(shift.group(3),0)); form='wrapper'
     for pat,store,typ in patterns:
      q=re.fullmatch(pat,pure)
      if q and w==store: inner=(typ,int(q.group(1),0),int(q.group(2),0) if typ=='bit' else None); form='wrapper'; break
     if inner is None:
      byteconst=re.fullmatch(r'(?:xor al, al|mov al, (0x[0-9a-f]+|[0-9]+)); ret',pure)
      intconst=re.fullmatch(r'(?:xor eax, eax|mov eax, (0x[0-9a-f]+|[0-9]+)); ret',pure)
      if byteconst and w=='mov byte ptr [rbx], al': inner=('constant_byte',int(byteconst.group(1),0) if byteconst.group(1) else 0,None)
      elif intconst and w=='mov dword ptr [rbx], eax': inner=('constant_dword',int(intconst.group(1),0) if intconst.group(1) else 0,None)
      elif pure=='xorps xmm0, xmm0; ret' and w=='movss dword ptr [rbx], xmm0': inner=('constant_dword',0,None)
   if isinstance(inner,str):
    patterns=[(r'mov eax, dword ptr \[rcx \+ (0x[0-9a-f]+)\]; mov dword ptr \[r8\], eax; ret','dword'),(r'movzx eax, byte ptr \[rcx \+ (0x[0-9a-f]+)\]; mov byte ptr \[r8\], al; ret','byte'),(r'movzx eax, byte ptr \[rcx \+ (0x[0-9a-f]+)\]; and al, (0x[0-9a-f]+|[0-9]+); mov byte ptr \[r8\], al; ret','bit')]
    matched=None
    shift=re.fullmatch(r'movzx eax, byte ptr \[rcx \+ (0x[0-9a-f]+)\]; shr (al|eax), (0x[0-9a-f]+|[0-9]+); and (al|eax), 1; mov byte ptr \[r8\], al; ret',inner)
    if shift and shift.group(2)==shift.group(4) and 1<=int(shift.group(3),0)<=7:
     matched=('shift',int(shift.group(1),0),1<<int(shift.group(3),0))
    constant=re.fullmatch(r'mov (byte|dword) ptr \[r8\], (0x[0-9a-f]+|[0-9]+); ret',inner)
    if constant: matched=('constant_'+constant.group(1),int(constant.group(2),0),None)
    for pat,typ in patterns:
     q=re.fullmatch(pat,inner)
     if q: matched=(typ,int(q.group(1),0),int(q.group(2),0) if typ=='bit' else None); break
    inner=matched
    if form=='inline_constant' and (not inner or not inner[0].startswith('constant_')): inner=None
   if not isinstance(inner,tuple): reject('unmatched_complete_code'); continue
   typ,offset,mask=inner
   if typ.startswith('constant_'): reject('not_inherited_field'); continue
   if (kind in ('Int','Float') and typ!='dword') or (kind=='Byte' and typ!='byte') or (kind=='Bool' and typ not in ('byte','bit','shift')): reject('return_width_mismatch'); continue
   chain=ancestry_cache.setdefault(va,ancestry(va)) if va not in ancestry_cache else ancestry_cache[va]
   if len(chain)<2: reject('no_supported_ancestor'); continue
   matches=[]
   for owner,owner_block,header in chain:
    for p in owner_block['properties']:
     if p.get('kind')!=kind or p.get('array_dim')!=1 or any(k in p for k in ('array_inner_of','map_key_of','map_value_of','set_element_of')): continue
     if not re.fullmatch(r'[A-Za-z_]\w*',p.get('name','')): continue
     if kind=='Byte' and p.get('type_refs'): continue
     if kind=='Bool':
      bs=[b for b in bits.get(owner,[]) if b['name']==p['name'] and b['offset']==offset]
      if len(bs)!=1 or bs[0]['mask']!=(mask if typ in ('bit','shift') else 255): continue
     elif p.get('offset')!=offset: continue
     matches.append((owner,p,header))
   if len(matches)!=1: reject('field_absent_or_ambiguous'); continue
   owner,prop_record,header=matches[0]
   if owner==cls: reject('not_inherited_field'); continue
   prop=prop_record['name']
   declaration=re.search(r'\b'+re.escape(prop)+r'\s*(?::\s*\d+)?\s*;',header)
   if not declaration: reject('field_not_declared'); continue
   labels=list(re.finditer(r'^\s*(public|protected|private):',header[:declaration.start()],re.M))
   if not labels or labels[-1].group(1)=='private': reject('inaccessible_field'); continue
   ret={'Int':'int32','Float':'float','Byte':'uint8','Bool':'bool'}[kind]
   # A non-unit bit mask yields a non-normalized raw byte in the native thunk.
   if kind=='Bool' and typ=='bit' and mask!=1: reject('bool_non_normalized'); continue
   stem=f'{addr:x}_{cls}_{name}'; inc=HERE/(stem+'.inc'); body='// Original address: '+hex(addr)+'.\n// CONFIDENCE: HIGH\n// Mechanically matched complete scalar read; see bulk_inherited_runtime/README.md.\nreturn '+owner+'::'+prop+';\n'
   if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs: '+str(inc))
   if not inc.exists(): inc.write_text(body)
   result={'class':cls,'function':name,'kind':'exec','signature':f"{ret} {name}()"+(' const' if flags&0x40000000 else ''),'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':False,'access':ACCESS[flags&0xe0000], 'notes':f'Original {form} inherited scalar getter at {hex(addr)}, reflected {kind} field {owner}::{prop} offset {hex(offset)}; source recovery only.'}
   results.append({'entry':result,'field_owner':owner,'field':prop,'offset':offset,'mask':mask,'assembly':asm,'target':hex(target) if target else None})
 args.output_dir.mkdir(parents=True,exist_ok=True)
 (args.output_dir/'manifest-entries.json').write_text(json.dumps([r['entry'] for r in results],indent=2)+'\n')
 report={'image_sha256':digest,'inventory_records':total,'candidates':len(results),'rejections':dict(sorted(rejects.items())),'results':results}
 (args.output_dir/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps({k:report[k] for k in ('inventory_records','candidates','rejections')},indent=2))
if __name__=='__main__': main()
