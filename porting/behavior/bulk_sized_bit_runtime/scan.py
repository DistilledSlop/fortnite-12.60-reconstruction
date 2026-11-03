#!/usr/bin/env python3
"""Conservative whole-inventory getter recovery; writes only local proposals and scratch report."""
import argparse, collections, hashlib, json, math, re, struct, sys
from pathlib import Path
sys.path.insert(0, '/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
ROOT=Path(__file__).resolve().parents[3]; REFL=ROOT/'porting/generated/reflection'
sys.path.insert(0, str(ROOT/'porting'))
from gen_native_classes import split_properties
sys.path.insert(0, str(ROOT/'porting/behavior/native_class_size_evidence_runtime'))
from scan import extract
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
 parser.add_argument('--revalidate-own',action='store_true',help='rescan ported entries belonging to bulk_sized_bit_runtime')
 parser.add_argument('--output-dir',type=Path,default=Path('/tmp/gameplay-bulk-sized-bit'),help='directory for the proposal manifest and report')
 args=parser.parse_args()
 image=IMAGE.read_bytes(); digest=hashlib.sha256(image).hexdigest()
 if digest!=SHA: raise SystemExit('image digest mismatch')
 md=Cs(CS_ARCH_X86,CS_MODE_64); md.detail=True
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
 index=load(REFL/'native_class_index.json')
 targets=collections.Counter(g.lower() for v in index.values() for g in v.get('gpsc_targets', []))
 size_cache={}
 byname=collections.defaultdict(list)
 for va,name in names.items(): byname[name].append(va)
 selected={x.strip() for x in (ROOT/'porting/native_class_set.txt').read_text().splitlines() if x.strip() and not x.lstrip().startswith('#')}
 own_path=str(HERE.relative_to(ROOT))+'/'
 manifest=load(ROOT/'porting/behavior/MANIFEST.json')
 headers=collections.defaultdict(list)
 for h in (ROOT/'porting/native/Source').glob('*/Generated/*.h'):
  headers[h.stem].append(h)
 def size(va):
  if not isinstance(va,str): return None
  va=va.lower()
  if va in size_cache: return size_cache[va]
  block=blocks.get(va); name=names.get(va); parent=parents.get(va) or {}; entry=index.get(name) or {}
  gs=entry.get('gpsc_targets') or []
  if not block or not name or byname.get(name)!=[va] or parent.get('name')!=name or not parent.get('package') or entry.get('block_va','').lower()!=va or len(gs)!=1 or targets[gs[0].lower()]!=1:
   size_cache[va]=None; return None
  proof=extract(image,md,int(gs[0],0),name,parent['package'])
  if proof is None: size_cache[va]=None; return None
  pairs,_=split_properties(block.get('properties',[]))
  ss={p.get('size_of_outer') for p,_ in pairs if p.get('kind')=='Bool' and isinstance(p.get('size_of_outer'),int) and p['size_of_outer']>0}
  if len(ss)>1 or ss and next(iter(ss))!=proof['size']: raise SystemExit('reflected size conflict: '+name)
  size_cache[va]=(proof['size'],bool(ss),proof['gpsc'])
  return size_cache[va]
 def occupancy(cls,block,own):
  pairs,notes=split_properties(block['properties'])
  if notes: return None
  props=[p for p,_ in pairs]
  locations=[]
  for p in props:
   if p.get('kind')=='Bool':
    bs=[b for b in bits.get(cls,[]) if b['name']==p['name']]
    if len(bs)!=1 or not isinstance(bs[0].get('offset'),int): return None
    loc=bs[0]['offset']
   else: loc=p.get('offset')
   if not isinstance(loc,int) or loc<0 or loc>=own: return None
   locations.append((loc,p))
  fields=[e for e in manifest if e.get('class')==cls and e.get('kind')=='member_field' and e.get('status')=='ported' and not (args.revalidate_own and e.get('recovery_source')=='bulk_sized_bit_runtime')]
  for e in fields:
   try: loc=int(str(e['offset']),0)
   except (KeyError,ValueError,TypeError): return None
   if loc<0 or loc>=own: return None
   locations.append((loc,e))
  offsets=sorted({loc for loc,_ in locations}|{own})
  widths={'Bool':1,'Byte':1,'Int8':1,'Int16':2,'UInt16':2,'Int':4,'UInt32':4,'Float':4,'Int64':8,'UInt64':8,'Double':8,'Name':8,'Object':8,'Class':8,'Array':16,'Str':16}
  spans=[]
  for loc,p in locations:
   kind=p.get('kind'); width=widths.get(kind)
   if kind=='member_field':
    typ=(p.get('signature') or '').split(' ',1)[0]
    width={'bool':1,'uint8':1,'int8':1,'int16':2,'uint16':2,'int32':4,'uint32':4,'float':4,'int64':8,'uint64':8,'double':8}.get(typ)
   dim=p.get('array_dim',1)
   if not isinstance(dim,int) or dim<1: return None
   end=loc+width*dim if width else next(x for x in offsets if x>loc)
   if end>own: return None
   spans.append((loc,end))
  return spans
 ported={(e.get('class'),e.get('function')) for e in manifest if e.get('status')=='ported' and not (args.revalidate_own and (e.get('inc_path') or '').startswith(own_path))}
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
   if kind != 'Bool': reject('unsupported_return_type'); continue
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
   own_proof=size(va); pva=parent.get('parent_va'); parent_proof=size(pva)
   if not own_proof or not parent_proof: reject('size_proof'); continue
   if own_proof[1] and parent_proof[1]: reject('both_sizes_already_reflected'); continue
   own=own_proof[0]; parent_size=parent_proof[0]
   hs=headers.get(cls,[])
   if not own or not parent_size or len(hs)!=1: reject('size_or_generated_header'); continue
   header=hs[0].read_text()
   declaration=re.findall(r'\bclass\s+\w+_API\s+'+re.escape(cls)+r'\s*:\s*public\s+(\w+)',header)
   if declaration!=[parent.get('parent_name')] or names.get(pva)!=parent.get('parent_name'): reject('generated_parent_mismatch'); continue
   spans=occupancy(cls,block,own)
   if spans is None: reject('unknown_occupancy'); continue
   asm=decode(addr); inner=None; target=None; pure=None
   if asm.startswith(PREFIX): inner=asm[len(PREFIX):]; form='inline'
   elif asm.startswith('mov rax, qword ptr [rdx + 0x20]; xor ecx, ecx; test rax, rax; setne cl; add rcx, rax; mov qword ptr [rdx + 0x20], rcx; '):
    inner=asm.split('mov qword ptr [rdx + 0x20], rcx; ',1)[1]; form='inline_constant'
   elif asm.startswith(WRAP):
    m=re.fullmatch(r'call (0x[0-9a-f]+); (mov (?:byte|dword) ptr \[rbx\], (?:al|eax)|movss dword ptr \[rbx\], xmm0); add rsp, 0x20; pop rbx; ret',asm[len(WRAP):])
    if m:
     target=int(m.group(1),16); pure=decode(target)
     w=m.group(2)
     patterns=[(r'mov eax, dword ptr \[rcx \+ (0x[0-9a-f]+)\]; ret','mov dword ptr [rbx], eax','dword'),(r'movss xmm0, dword ptr \[rcx \+ (0x[0-9a-f]+)\]; ret','movss dword ptr [rbx], xmm0','dword'),(r'movzx eax, byte ptr \[rcx \+ (0x[0-9a-f]+)\]; ret','mov byte ptr [rbx], al','byte'),(r'movzx eax, byte ptr \[rcx \+ (0x[0-9a-f]+)\]; and (?:al|eax), (0x[0-9a-f]+|[0-9]+); ret','mov byte ptr [rbx], al','bit')]
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
    patterns=[(r'mov eax, dword ptr \[rcx \+ (0x[0-9a-f]+)\]; mov dword ptr \[r8\], eax; ret','dword'),(r'movzx eax, byte ptr \[rcx \+ (0x[0-9a-f]+)\]; mov byte ptr \[r8\], al; ret','byte'),(r'movzx eax, byte ptr \[rcx \+ (0x[0-9a-f]+)\]; and (?:al|eax), (0x[0-9a-f]+|[0-9]+); mov byte ptr \[r8\], al; ret','bit')]
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
   if not (typ == 'shift' or (typ == 'bit' and mask == 1)):
    reject('not_normalized_packed_bit_read'); continue
   width=1
   if not (parent_size<=offset and offset+width<=own): reject('outside_declaring_class'); continue
   if any(offset<end and start<offset+width for start,end in spans): reject('occupied'); continue
   field_name=f'NativeRecoveredBits_0x{offset:x}'
   taken={p.get('name') for p in block['properties']} | set(funcs) | {e.get('function') for e in manifest if e.get('class')==cls and not (args.revalidate_own and e.get('recovery_source')=='bulk_sized_bit_runtime')}
   if field_name in taken: reject('field_name_collision'); continue
   results.append({'class':cls,'function':name,'kind':kind,'offset':offset,'width':width,'mask':mask,'address':addr,'flags':flags,'reflection_flags':fn['flags'],'assembly':asm,'form':form,'target':hex(target) if target else None,'target_assembly':pure,'parent_size':parent_size,'own_size':own,'own_gpsc':own_proof[2],'parent_gpsc':parent_proof[2],'own_reflected_size':own_proof[1],'parent_reflected_size':parent_proof[1]})
 # All proposed intervals must agree, including partial overlaps.
 groups=collections.defaultdict(list)
 for r in results: groups[r['class']].append(r)
 accepted=[]
 for cls,rs in groups.items():
  bad=set()
  for i,a in enumerate(rs):
   for j in range(i):
    b=rs[j]
    if a['offset']<b['offset']+b['width'] and b['offset']<a['offset']+a['width'] and (a['offset'],a['width'],a['kind'])!=(b['offset'],b['width'],b['kind']): bad.update((i,j))
  rejects['conflicting_intervals']+=len(bad)
  accepted.extend(r for i,r in enumerate(rs) if i not in bad)
 entries=[]; fields={}
 for r in accepted:
  cls,name,kind,offset,addr=(r[x] for x in ('class','function','kind','offset','address'))
  typ='bool'; field=f'NativeRecoveredBits_0x{offset:x}'
  key=(cls,offset)
  if key not in fields:
   fields[key]={'class':cls,'function':field,'kind':'member_field','signature':f'uint8 {field}','signature_status':'proposed','access':'public','offset':hex(offset),'inc_path':None,'confidence':'high','address':None,'status':'ported','recovery_source':'bulk_sized_bit_runtime','notes':'Original read identifies storage; writers and construction remain unported. Source recovery only.'}
  stem=f'{addr:x}_{cls}_{name}'; inc=HERE/(stem+'.inc')
  body='// Original address: '+hex(addr)+'.\n// CONFIDENCE: HIGH\n// Mechanically matched complete native storage read; see bulk_sized_bit_runtime/README.md.\nreturn ('+field+' & '+hex(r['mask'])+') != 0;\n'
  if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs: '+str(inc))
  if not inc.exists(): inc.write_text(body)
  entries.append({'class':cls,'function':name,'kind':'exec','signature':f"{typ} {name}()"+(' const' if r['flags']&0x40000000 else ''),'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':r['reflection_flags'],'static':False,'access':ACCESS[r['flags']&0xe0000],'notes':f"Original {r['form']} native storage getter at {hex(addr)}, offset {hex(offset)}, mask {hex(r['mask'])}; writers and construction unported."})
 if args.revalidate_own:
  for e in manifest:
   if e.get('kind')=='member_field' and e.get('recovery_source')=='bulk_sized_bit_runtime':
    key=(e.get('class'),int(str(e.get('offset')),0))
    if key in fields and any(e.get(k)!=fields[key].get(k) for k in ('function','signature','offset','inc_path','access','confidence')): raise SystemExit('canonical recovered field mismatch: '+str(key))
 args.output_dir.mkdir(parents=True,exist_ok=True)
 (args.output_dir/'manifest-entries.json').write_text(json.dumps(list(fields.values())+entries,indent=2)+'\n')
 report={'image_sha256':digest,'inventory_records':total,'candidates':len(accepted),'fields':len(fields),'rejections':dict(sorted(rejects.items())),'results':accepted}
 (args.output_dir/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps({k:report[k] for k in ('inventory_records','candidates','fields','rejections')},indent=2))
if __name__=='__main__': main()
