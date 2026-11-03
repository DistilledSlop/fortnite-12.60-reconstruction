#!/usr/bin/env python3
"""Conservative whole-inventory getter recovery; writes only local proposals and scratch report."""
import argparse, collections, hashlib, json, math, re, struct, sys
from pathlib import Path
sys.path.insert(0, '/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
ROOT=Path(__file__).resolve().parents[3]; REFL=ROOT/'porting/generated/reflection'
sys.path.insert(0, str(ROOT/'porting'))
from gen_native_classes import split_properties, Resolver
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
 parser.add_argument('--revalidate-own',action='store_true',help='rescan ported entries belonging to bulk_native_object_runtime')
 parser.add_argument('--output-dir',type=Path,default=Path('/tmp/gameplay-bulk-native-object'),help='directory for the proposal manifest and report')
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
 refs=load(REFL/'class_ref_map.json')['resolved']; reflection=load(REFL/'fortnite_game_reflection.json'); bits=load(REFL/'bool_bits.json')['by_class']
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
  fields=[e for e in manifest if e.get('class')==cls and e.get('kind')=='member_field' and e.get('status')=='ported' and not (args.revalidate_own and e.get('recovery_source')=='bulk_native_object_runtime')]
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
   if kind != 'Object': reject('unsupported_return_type'); continue
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
   own=own_proof[0]; parent_size=parent_proof[0]
   hs=headers.get(cls,[])
   if not own or not parent_size or len(hs)!=1: reject('size_or_generated_header'); continue
   header=hs[0].read_text()
   declaration=re.findall(r'\bclass\s+\w+_API\s+'+re.escape(cls)+r'\s*:\s*public\s+(\w+)',header)
   if declaration!=[parent.get('parent_name')] or names.get(pva)!=parent.get('parent_name'): reject('generated_parent_mismatch'); continue
   spans=occupancy(cls,block,own)
   if spans is None: reject('unknown_occupancy'); continue
   # Every declaration in the generated header must be accounted for by reflected
   # properties or canonical member fields; unknown declarations can conceal storage.
   known={p.get('name') for p in block['properties']} | {e.get('function') for e in manifest if e.get('class')==cls and e.get('kind')=='member_field' and e.get('status')=='ported'}
   decls=re.findall(r'^\s*([^\n;{}]+);\s*$',header,re.M)
   unknown=False
   for d in decls:
    d=d.strip()
    if '(' in d or d.startswith(('class ','struct ','enum ','using ','typedef ','friend ','static_assert')): continue
    m=re.search(r'([A-Za-z_]\w*)\s*(?:\[[^]]+\])?\s*(?::\s*\d+)?$',d)
    if not m or m.group(1) not in known: unknown=True; break
   if unknown: reject('unaccounted_header_field'); continue
   type_refs=ps[0].get('type_refs') or {}; ref=type_refs.get('class')
   target_va=refs.get(ref.lower()) if isinstance(ref,str) else None
   target_name=names.get(target_va.lower()) if target_va else None
   target_block=blocks.get(target_va.lower()) if target_va else None
   if set(type_refs)!={'class'} or not target_name or len(byname[target_name])!=1 or not target_block or int(target_block['class_flags'],0)&0x4000: reject('unresolved_reflected_type'); continue
   resolver=Resolver({ref:target_name}); ret=resolver.ctype(ps[0],{})
   if resolver.todos or ret!=target_name+'*': reject('generator_type_unresolved'); continue
   sigs=re.findall(r'^\s*([AU][A-Za-z_0-9]*\s*\*)\s*'+re.escape(name)+r'\(\)( const)?\s*;',header,re.M)
   if sigs and (len(sigs)!=1 or re.sub(r'\s+','',sigs[0][0])!=ret or bool(sigs[0][1])!=bool(flags&0x40000000)): reject('header_signature_mismatch'); continue
   asm=decode(addr); target=None; pure=None; offset=None
   if asm.startswith(PREFIX):
    q=re.fullmatch(r'mov rax, qword ptr \[rcx \+ (0x[0-9a-f]+)\]; mov qword ptr \[r8\], rax; ret',asm[len(PREFIX):])
    if q: offset=int(q.group(1),0); form='inline'
   elif asm.startswith(WRAP):
    q=re.fullmatch(r'call (0x[0-9a-f]+); mov qword ptr \[rbx\], rax; add rsp, 0x20; pop rbx; ret',asm[len(WRAP):])
    if q:
     target=int(q.group(1),0); pure=decode(target)
     m=re.fullmatch(r'mov rax, qword ptr \[rcx \+ (0x[0-9a-f]+)\]; ret',pure)
     if m: offset=int(m.group(1),0); form='wrapper'
   if offset is None: reject('unmatched_complete_code'); continue
   width=8
   if not (parent_size<=offset and offset+width<=own): reject('outside_declaring_class'); continue
   if any(offset<end and start<offset+width for start,end in spans): reject('occupied'); continue
   field_name=f'NativeRecoveredObject_0x{offset:x}'
   taken=known | set(funcs) | {e.get('function') for e in manifest if e.get('class')==cls and not (args.revalidate_own and e.get('recovery_source')=='bulk_native_object_runtime')}
   if field_name in taken: reject('field_name_collision'); continue
   results.append({'class':cls,'function':name,'kind':ret,'type_ref':ref,'target_class':target_name,'offset':offset,'width':width,'address':addr,'flags':flags,'reflection_flags':fn['flags'],'assembly':asm,'form':form,'target':hex(target) if target else None,'target_assembly':pure,'parent_size':parent_size,'own_size':own,'own_gpsc':own_proof[2],'parent_gpsc':parent_proof[2],'own_reflected_size':own_proof[1],'parent_reflected_size':parent_proof[1]})
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
  typ=kind; field=f'NativeRecoveredObject_0x{offset:x}'
  key=(cls,offset)
  if key not in fields:
   fields[key]={'class':cls,'function':field,'kind':'member_field','signature':f'{typ} {field}','signature_status':'proposed','access':'public','offset':hex(offset),'inc_path':None,'confidence':'high','address':None,'status':'blocked_native_storage_ownership','recovery_source':'bulk_native_object_runtime','notes':'Raw pointer representation follows the proven read; GC ownership, writers and construction remain unported. Source recovery only.'}
  stem=f'{addr:x}_{cls}_{name}'; inc=HERE/(stem+'.inc')
  body='// Original address: '+hex(addr)+'.\n// CONFIDENCE: HIGH\n// Mechanically matched complete native storage read; see bulk_native_object_runtime/README.md.\nreturn '+field+';\n'
  if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs: '+str(inc))
  if not inc.exists(): inc.write_text(body)
  entries.append({'class':cls,'function':name,'kind':'exec','signature':f"{typ} {name}()"+(' const' if r['flags']&0x40000000 else ''),'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'blocked_native_storage_ownership','reflection_flags':r['reflection_flags'],'static':False,'access':ACCESS[r['flags']&0xe0000],'notes':f"Original {r['form']} native storage getter at {hex(addr)}, offset {hex(offset)}; raw pointer read only; GC ownership, writers and construction unported."})
 if args.revalidate_own:
  for e in manifest:
   if e.get('kind')=='member_field' and e.get('recovery_source')=='bulk_native_object_runtime':
    key=(e.get('class'),int(str(e.get('offset')),0))
    if key in fields and any(e.get(k)!=fields[key].get(k) for k in ('function','signature','offset','inc_path','access','confidence')): raise SystemExit('canonical recovered field mismatch: '+str(key))
 args.output_dir.mkdir(parents=True,exist_ok=True)
 (args.output_dir/'manifest-entries.json').write_text(json.dumps(list(fields.values())+entries,indent=2)+'\n')
 report={'image_sha256':digest,'inventory_records':total,'candidates':len(accepted),'fields':len(fields),'rejections':dict(sorted(rejects.items())),'results':accepted}
 (args.output_dir/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps({k:report[k] for k in ('inventory_records','candidates','fields','rejections')},indent=2))
if __name__=='__main__': main()
