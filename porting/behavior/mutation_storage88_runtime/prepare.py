#!/usr/bin/env python3
"""Assess complete getter matches for unoccupied, evidenced storage."""
import collections, json, re, sys, mmap, struct, subprocess, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]; REF=ROOT/'porting/generated/reflection'; HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT/'porting'))
from gen_native_classes import split_properties
load=lambda p:json.loads(p.read_text())
names=load(REF/'class_name_map.json'); parents=load(REF/'class_parent_map.json'); bits=load(REF/'bool_bits.json')['by_class']; native=load(REF/'native_funcs.json')
blocks={b['block_va'].lower():b for b in load(REF/'fortnite_game_reflection.json')['classes_in_groups_with_fort_names']}
manifest=load(ROOT/'porting/behavior/MANIFEST.json'); pending=[]
for path in ('porting/behavior/leaf_getter_storage84_runtime/manifest-entries.json','porting/behavior/bulk_known_leaf_getters84_runtime/manifest-entries.json'):
 pending += load(ROOT/path)
byname=collections.defaultdict(list)
for va,name in names.items(): byname[name].append(va)
headers=collections.defaultdict(list)
for h in (ROOT/'porting/native/Source').glob('*/Generated/*.h'): headers[h.stem].append(h)
image_file=open('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin','rb')
image=mmap.mmap(image_file.fileno(),0,access=mmap.ACCESS_READ)
BASE=0x140000000; HELPER=0x142d698b0
registration={}; attempted=set()
def registered(va):
 if not isinstance(va,str): return None
 va=va.lower()
 if va in attempted: return registration.get(va,{}).get('size')
 attempted.add(va)
 if len(attempted)>40:return None
 b=blocks.get(va); name=names.get(va)
 if not b or not name or name[0] not in 'AU': return None
 try: nr=int(b['class_noregister_fn'],16)
 except (KeyError,ValueError,TypeError):return None
 def read(addr,n):
  off=addr-BASE
  return image[off:off+n] if 0<=off and off+n<=len(image) else b''
 builder=nr; jump=read(nr,5)
 if len(jump)==5 and jump[0]==0xe9:builder=nr+5+struct.unpack('<i',jump[1:])[0]
 body=read(builder,256)
 for i in range(len(body)-5):
  if body[i]!=0xe8 or builder+i+5+struct.unpack('<i',body[i+1:i+5])[0]!=HELPER:continue
  prefix=body[:i]
  # Require literal target, consecutive direct stack argument stores immediately before call.
  if len(prefix)<16 or prefix[-16:-12]!=b'\xc7\x44\x24\x28' or prefix[-8:-4]!=b'\xc7\x44\x24\x20':continue
  align=struct.unpack('<I',prefix[-12:-8])[0]; size=struct.unpack('<I',prefix[-4:])[0]
  if not (0<size<0x100000 and align in (1,2,4,8,16,32,64)):continue
  literals=[]
  for j in range(len(prefix)-6):
   if prefix[j:j+3]==b'\x48\x8d\x15':
    literal=builder+j+7+struct.unpack('<i',prefix[j+3:j+7])[0]
    if read(literal,len(name[1:].encode('utf-16le'))+2)==name[1:].encode('utf-16le')+b'\0\0':literals.append(literal)
  if len(literals)!=1:continue
  # Disassemble from the proven builder boundary; reject altered RDX or control
  # flow after the literal LEA. Thus the name is still the call's second arg.
  with tempfile.NamedTemporaryFile() as f:
   f.write(body[:i+5]); f.flush()
   dis=subprocess.check_output(['objdump','-D','-b','binary','-m','i386:x86-64','-M','intel',f'--adjust-vma={builder}',f.name],text=True)
  ins=[]
  for line in dis.splitlines():
   m=re.match(r'\s*([0-9a-f]+):\s+(?:[0-9a-f]{2}\s+)+\s*(\S+)\s*(.*)',line)
   if m:ins.append((int(m.group(1),16),m.group(2),m.group(3)))
  lea_addr=next((builder+j for j in range(len(prefix)-6) if prefix[j:j+3]==b'\x48\x8d\x15' and builder+j+7+struct.unpack('<i',prefix[j+3:j+7])[0]==literals[0]),None)
  if not any(a==lea_addr and op=='lea' and args.startswith('rdx,') for a,op,args in ins):continue
  intervening=[(op,args) for a,op,args in ins if lea_addr<a<builder+i]
  if any(op.startswith('j') or op in ('call','ret') or re.match(r'(?:r|e)dx\b|d[lhx]\b',args) for op,args in intervening):continue
  evidence=load(REF/'class_name_map.evidence.json')
  matches=[x for x in evidence['blocks'] if x['block_va'].lower()==va and name in x['candidates']]
  if len(matches)!=1:continue
  registration[va]={'class':name,'block':va,'noregister':hex(nr),'builder':hex(builder),'literal':hex(literals[0]),'call':hex(builder+i),'size':size,'align':align,'bytes':body[:i+5].hex()}
  return size
 return None
def size(va):
 b=blocks.get(va.lower(),{}); pairs,notes=split_properties(b.get('properties',[]))
 ss={p.get('size_of_outer') for p,_ in pairs if p.get('kind')=='Bool' and isinstance(p.get('size_of_outer'),int) and p['size_of_outer']>0}
 return next(iter(ss)) if len(ss)==1 and not notes else registered(va)
structs=collections.defaultdict(list)
for s in load(REF/'struct_blocks.json')['structs']:
 for fn in s.get('construct_fns',[]): structs[fn.lower()].append(s)
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
  if kind=='Struct':
   ref=p.get('type_refs',{}).get('struct','').lower(); matches=structs.get(ref,[])
   if len(matches)==1 and (matches[0].get('struct_ops') or {}).get('size_matches') and isinstance(matches[0].get('size'),int) and matches[0]['size']>0: width=matches[0]['size']
  if kind=='member_field':width={'bool':1,'uint8':1,'int8':1,'int16':2,'uint16':2,'int32':4,'uint32':4,'float':4,'int64':8,'uint64':8,'double':8}.get((p.get('signature') or '').split(' ',1)[0])
  dim=p.get('array_dim',1)
  if not isinstance(dim,int) or dim<1:return None
  end=loc+width*dim if width else next(x for x in offsets if x>loc)
  if end>own:return None
  spans.append((loc,end,p.get('name') or p.get('function'),kind))
 return spans
source=load(ROOT/'porting/behavior/leaf_storage83_runtime/report.json')
old=load(ROOT/'porting/behavior/disable_filters84_runtime/report.json')['row']
inputs=[r for r in source['rows'] if r['reasons']]+[old]
rows=[]; entries=[]; chosen=[]
for original in inputs:
 row={k:v for k,v in original.items() if k in ('class','function','wrapper_address','wrapper_bytes','wrapper_sha256','leaf_address','leaf_bytes','leaf_sha256','leaf_assembly')}; row['reasons']=[]; rows.append(row)
 cls=row['class']; fn=row['function']; asm=row['leaf_assembly']; writes=[]
 for part in asm.split('; '):
  if part=='ret': continue
  m=re.fullmatch(r'(mov byte|or dword) ptr \[rcx \+ (0x[0-9a-f]+)\], (0x[0-9a-f]+|[0-9]+)',part)
  if not m:row['reasons'].append('unsupported_instruction'); break
  writes.append((int(m[2],0),1 if m[1]=='mov byte' else 4,m[1],int(m[3],0)))
 row['writes']=writes
 if not writes or row['reasons']:continue
 if any(e.get('class')==cls and e.get('function')==fn and e.get('status')=='ported' for e in manifest):row['reasons'].append('already_ported');continue
 vs=byname.get(cls,[])
 if len(vs)!=1:row['reasons'].append('ambiguous_class');continue
 va=vs[0]; parent=parents.get(va,{}); pva=parent.get('parent_va'); block=blocks.get(va.lower()); own=size(va); ps=size(pva) if isinstance(pva,str) else None
 row.update(own_size=own,parent_size=ps,parent=parent.get('parent_name'))
 if not block or not own or not ps or len(headers.get(cls,[]))!=1:row['reasons'].append('missing_class_parent_size_or_header');continue
 h=headers[cls][0].read_text(); decl=re.findall(r'\bclass\s+\w+_API\s+'+re.escape(cls)+r'\s*:\s*public\s+(\w+)',h)
 if names.get(pva)!=parent.get('parent_name') or decl!=[parent.get('parent_name')]:row['reasons'].append('parent_mismatch');continue
 pb=blocks.get(pva.lower()); pspans=occupancy(parent['parent_name'],pb,ps) if pb else None; spans=occupancy(cls,block,own)
 if pspans is None or spans is None:row['reasons'].append('unknown_occupancy');continue
 row['parent_span_count']=len(pspans); row['occupancy']=[]
 for off,width,op,val in writes:
  overlap=[s for s in spans if off<s[1] and s[0]<off+width]
  row['occupancy'].append({'offset':off,'overlaps':overlap,'nearby_spans':[s for s in spans if s[0]<off+width+16 and s[1]>off-16]})
  if not ps<=off or off+width>own:row['reasons'].append('outside_class')
  if overlap:row['reasons'].append('occupied')
  if any(c==cls and off<e and b<off+width for c,b,e in chosen):row['reasons'].append('proposal_overlap')
  field=f'NativeRecovered{"Byte" if width==1 else "Dword"}_0x{off:x}'
  if re.search(r'\b'+field+r'\b',h) or any(e.get('class')==cls and e.get('function')==field for e in manifest):row['reasons'].append('name_collision')
 if row['reasons']:continue
 flags=native[cls][fn]['flags']; params=native[cls][fn]['params']
 if params:row['reasons'].append('unexpected_parameters');continue
 access={0x20000:'public',0x40000:'private',0x80000:'protected'}[int(flags,0)&0xe0000]
 lines=[]
 for off,width,op,val in writes:
  field=f'NativeRecovered{"Byte" if width==1 else "Dword"}_0x{off:x}'; typ='uint8' if width==1 else 'uint32'
  entries.append({'class':cls,'function':field,'kind':'member_field','signature':f'{typ} {field}','signature_status':'proposed','access':'public','offset':hex(off),'inc_path':None,'confidence':'high','address':None,'status':'ported','recovery_source':'mutation_storage88_runtime','notes':'Complete leaf proves raw width and operation only; no initialization or other writer claims.'})
  lines.append(f'{field} {"=" if op=="mov byte" else "|="} {val};'); chosen.append((cls,off,off+width))
 addr=int(row['wrapper_address'],0); inc=HERE/f'{addr:x}_{cls}_{fn}.inc'; inc.write_text('// CONFIDENCE: high; complete tail-call raw storage operation.\n'+'\n'.join(lines)+'\n')
 entries.append({'class':cls,'function':fn,'kind':'exec','signature':f'void {fn}()'+(' const' if int(flags,0)&0x40000000 else ''),'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':flags,'static':False,'access':access,'notes':'Complete tail-call raw storage operation; see mutation_storage88_runtime/report.json.'})
(HERE/'manifest-entries.json').write_text(json.dumps(entries,indent=2)+'\n')
(HERE/'report.json').write_text(json.dumps({'expected_image_sha256':source['expected_image_sha256'],'digest_verified_this_run':False,'source_reports':['leaf_storage83_runtime/report.json','disable_filters84_runtime/report.json'],'registrations':registration,'attempted_registration_blocks':sorted(attempted),'proposals':sum(e['kind']=='exec' for e in entries),'rows':rows},indent=2)+'\n')
print('assessed',len(rows),'proposals',sum(e['kind']=='exec' for e in entries))
