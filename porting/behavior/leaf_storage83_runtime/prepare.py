#!/usr/bin/env python3
"""Assess the twelve unbound matched leaves using conservative occupancy."""
import collections, json, re, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]; REF=ROOT/'porting/generated/reflection'; HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT/'porting'))
from gen_native_classes import split_properties
load=lambda p:json.loads(p.read_text())
names=load(REF/'class_name_map.json'); parents=load(REF/'class_parent_map.json'); bits=load(REF/'bool_bits.json')['by_class']; native=load(REF/'native_funcs.json')
blocks={b['block_va'].lower():b for b in load(REF/'fortnite_game_reflection.json')['classes_in_groups_with_fort_names']}
manifest=load(ROOT/'porting/behavior/MANIFEST.json'); byname=collections.defaultdict(list)
for va,name in names.items(): byname[name].append(va)
headers=collections.defaultdict(list)
for h in (ROOT/'porting/native/Source').glob('*/Generated/*.h'): headers[h.stem].append(h)
def size(va):
 b=blocks.get(va.lower(),{}); pairs,notes=split_properties(b.get('properties',[]))
 ss={p.get('size_of_outer') for p,_ in pairs if p.get('kind')=='Bool' and isinstance(p.get('size_of_outer'),int) and p['size_of_outer']>0}
 return next(iter(ss)) if len(ss)==1 and not notes else None
def occupancy(cls,block,own):
 pairs,notes=split_properties(block.get('properties',[]))
 if notes:return None
 locs=[]
 for p,_ in pairs:
  if p.get('kind')=='Bool':
   bs=[b for b in bits.get(cls,[]) if b['name']==p['name']]
   if len(bs)!=1:return None
   loc=bs[0].get('offset')
  else:loc=p.get('offset')
  if not isinstance(loc,int) or not 0<=loc<own:return None
  locs.append((loc,p))
 for e in manifest:
  if e.get('class')!=cls or e.get('kind')!='member_field' or e.get('status')!='ported':continue
  try:loc=int(str(e['offset']),0)
  except (KeyError,ValueError,TypeError):return None
  if not 0<=loc<own:return None
  locs.append((loc,e))
 offsets=sorted({loc for loc,_ in locs}|{own}); spans=[]
 widths={'Bool':1,'Byte':1,'Int8':1,'Int16':2,'UInt16':2,'Int':4,'UInt32':4,'Float':4,'Int64':8,'UInt64':8,'Double':8,'Name':8,'Object':8,'Class':8,'Array':16,'Str':16}
 for loc,p in locs:
  kind=p.get('kind'); width=widths.get(kind)
  if kind=='member_field':width={'bool':1,'uint8':1,'int8':1,'int16':2,'uint16':2,'int32':4,'uint32':4,'float':4,'int64':8,'uint64':8,'double':8}.get((p.get('signature') or '').split(' ',1)[0])
  dim=p.get('array_dim',1)
  if not isinstance(dim,int) or dim<1:return None
  end=loc+width*dim if width else next(x for x in offsets if x>loc)
  if end>own:return None
  spans.append((loc,end,p.get('name') or p.get('function'),kind))
 return spans
source=load(ROOT/'porting/behavior/bulk_leaf_mutation83_runtime/report.json'); rows=[]; candidates=[]
for r in source['matches']:
 if r['accepted']:continue
 cls=r['class']; fn=r['function']; match=re.fullmatch(r'(mov) byte ptr \[rcx \+ (0x[0-9a-f]+)\], (0x[0-9a-f]+|[0-9]+); ret|(?:or) dword ptr \[rcx \+ (0x[0-9a-f]+)\], (0x[0-9a-f]+|[0-9]+); ret',r['leaf_assembly'])
 row={'class':cls,'function':fn,'wrapper_address':r['address'],'wrapper_bytes':r['bytes'],'wrapper_sha256':r['sha256'],'leaf_address':r['leaf_address'],'leaf_bytes':r['leaf_bytes'],'leaf_sha256':r['leaf_sha256'],'leaf_assembly':r['leaf_assembly'],'reasons':[]}; rows.append(row)
 if not match:row['reasons'].append('unsupported_leaf');continue
 g=match.groups(); isbyte=bool(g[0]); offset=int(g[1] if isbyte else g[3],0); value=int(g[2] if isbyte else g[4],0); width=1 if isbyte else 4
 row.update(offset=offset,width=width)
 if cls=='AFortPlayerPawn':row['reasons'].append('separately_owned');continue
 vs=byname.get(cls,[])
 if len(vs)!=1:row['reasons'].append('ambiguous_class');continue
 va=vs[0]; parent=parents.get(va,{}); pva=parent.get('parent_va'); block=blocks.get(va.lower()); own=size(va); ps=size(pva) if isinstance(pva,str) else None
 row.update(own_size=own,parent_size=ps)
 if not block or not own or not ps or len(headers.get(cls,[]))!=1:row['reasons'].append('missing_class_parent_size_or_header');continue
 h=headers[cls][0].read_text(); decl=re.findall(r'\bclass\s+\w+_API\s+'+re.escape(cls)+r'\s*:\s*public\s+(\w+)',h)
 if names.get(pva)!=parent.get('parent_name') or decl!=[parent.get('parent_name')]:row['reasons'].append('parent_mismatch');continue
 if not (ps<=offset and offset+width<=own):row['reasons'].append('outside_class')
 parentblock=blocks.get(pva.lower())
 parentspans=occupancy(parent['parent_name'],parentblock,ps) if parentblock else None
 if parentspans is None:row['reasons'].append('unknown_parent_occupancy');continue
 row['parent_span_count']=len(parentspans)
 spans=occupancy(cls,block,own)
 if spans is None:row['reasons'].append('unknown_occupancy');continue
 overlap=[s for s in spans if offset<s[1] and s[0]<offset+width]
 row['overlaps']=overlap
 row['nearby_spans']=sorted([s for s in spans if s[0]<offset+width+16 and s[1]>offset-16])
 if overlap:row['reasons'].append('occupied')
 field=('NativeRecoveredByte' if isbyte else 'NativeRecoveredUInt32')+f'_0x{offset:x}'
 if re.search(r'\b'+field+r'\b',h) or any(e.get('class')==cls and e.get('function')==field for e in manifest):row['reasons'].append('name_collision')
 if not row['reasons']:candidates.append((row,field,value))
entries=[]
for row,field,value in candidates:
 cls=row['class']; fn=row['function']; off=row['offset']; width=row['width']; typ='uint8' if width==1 else 'uint32'; addr=int(row['wrapper_address'],0); flags=native[cls][fn]['flags']; access={0x20000:'public',0x40000:'private',0x80000:'protected'}[int(flags,0)&0xe0000]
 entries.append({'class':cls,'function':field,'kind':'member_field','signature':f'{typ} {field}','signature_status':'proposed','access':'public','offset':hex(off),'inc_path':None,'confidence':'high','address':None,'status':'ported','recovery_source':'leaf_storage83_runtime','notes':'Complete leaf proves raw width and operation only; no initialization or other writer claims.'})
 inc=HERE/f'{addr:x}_{cls}_{fn}.inc'; inc.write_text(f'// CONFIDENCE: high; complete constant-write leaf.\n// Original wrapper {hex(addr)}; direct leaf {row["leaf_address"]}.\n{field} {"=" if width==1 else "|="} 0x{value:x}u;\n')
 entries.append({'class':cls,'function':fn,'kind':'exec','signature':f'void {fn}()','signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':flags,'static':False,'access':access,'notes':f'Complete tail-call raw storage operation; see leaf_storage83_runtime/report.json.'})
(HERE/'manifest-entries.json').write_text(json.dumps(entries,indent=2)+'\n')
(HERE/'report.json').write_text(json.dumps({'expected_image_sha256':source['expected_image_sha256'],'digest_verified_this_run':False,'source_report':'bulk_leaf_mutation83_runtime/report.json','candidates':len(candidates),'rows':rows},indent=2)+'\n')
print('assessed',len(rows),'proposals',len(candidates))
