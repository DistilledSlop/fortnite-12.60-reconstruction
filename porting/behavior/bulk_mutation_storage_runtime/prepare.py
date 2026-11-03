#!/usr/bin/env python3
"""Conservative whole-inventory getter recovery; writes only local proposals and scratch report."""
import argparse, collections, hashlib, json, math, re, struct, sys
from pathlib import Path
sys.path.insert(0, '/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
ROOT=Path(__file__).resolve().parents[3]; REFL=ROOT/'porting/generated/reflection'
sys.path.insert(0, str(ROOT/'porting'))
from gen_native_classes import split_properties
HERE=Path(__file__).resolve().parent
IMAGE=Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
BASE=0x140000000; SHA='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
load=lambda p: json.loads(p.read_text())
PREFIX='mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
WRAP='push rbx; sub rsp, 0x20; mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; mov rbx, r8; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
NET=0x80000000|0x01000000|0x00200000|0x4000|0x1000|0x100|0x80|0x40
ACCESS={0x20000:'public',0x40000:'private',0x80000:'protected'}
def main():
 image=IMAGE.read_bytes(); digest=hashlib.sha256(image).hexdigest()
 if digest!=SHA: raise SystemExit('image digest mismatch')
 native=load(REFL/'native_funcs.json'); names=load(REFL/'class_name_map.json'); parents=load(REFL/'class_parent_map.json')
 blocks={c['block_va'].lower():c for c in load(REFL/'fortnite_game_reflection.json')['classes_in_groups_with_fort_names']}
 bits=load(REFL/'bool_bits.json')['by_class']; manifest=load(ROOT/'porting/behavior/MANIFEST.json')
 byname=collections.defaultdict(list)
 for va,name in names.items(): byname[name].append(va)
 selected={x.strip() for x in (ROOT/'porting/native_class_set.txt').read_text().splitlines() if x.strip() and not x.lstrip().startswith('#')}
 headers=collections.defaultdict(list)
 for h in (ROOT/'porting/native/Source').glob('*/Generated/*.h'): headers[h.stem].append(h)
 class Args: revalidate_own=False
 args=Args()
 def size(va):
  b=blocks.get(va.lower(),{})
  pairs,_=split_properties(b.get('properties',[]))
  ss={p.get('size_of_outer') for p,_ in pairs if p.get('kind')=='Bool' and isinstance(p.get('size_of_outer'),int) and p['size_of_outer']>0}
  return next(iter(ss)) if len(ss)==1 else None
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
  fields=[e for e in manifest if e.get('class')==cls and e.get('kind')=='member_field' and e.get('status')=='ported' and not (args.revalidate_own and e.get('recovery_source')=='bulk_native_scalar_runtime')]
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
 md=Cs(CS_ARCH_X86,CS_MODE_64)
 prefixes=['mov rax, qword ptr [rdx + 0x20]; xor r8d, r8d; test rax, rax; setne r8b; add r8, rax; mov qword ptr [rdx + 0x20], r8; ', 'mov r8, qword ptr [rdx + 0x20]; xor eax, eax; test r8, r8; setne al; add rax, r8; mov qword ptr [rdx + 0x20], rax; ']
 pat=re.compile(r'(mov|and|or) (byte|dword) ptr \[rcx \+ (0x[0-9a-f]+)\], (0x[0-9a-f]+|[0-9]+); ret')
 ported={(e.get('class'),e.get('function')) for e in manifest if e.get('status')=='ported'}
 rows=[]; accepted=[]; total=0
 for cls,funcs in sorted(native.items()):
  for name,fn in sorted(funcs.items()):
   total+=1
   if not isinstance(fn,dict) or fn.get('params')!=[]: continue
   try: flags=int(fn['flags'],0); addr=int(fn['exec_addr'],0)
   except (ValueError,TypeError,KeyError): continue
   pos=addr-BASE
   if pos<0 or pos+128>len(image): continue
   ins=[]
   for i in md.disasm(image[pos:pos+128],addr):
    ins.append((i.mnemonic+' '+i.op_str).rstrip())
    if i.mnemonic=='ret' or i.mnemonic.startswith('j'): break
   asm='; '.join(ins); q=None
   for prefix in prefixes:
    if asm.startswith(prefix): q=pat.fullmatch(asm[len(prefix):]); break
   if not q: continue
   op,width,off,val=q.groups(); offset=int(off,0); value=int(val,0); n=1 if width=='byte' else 4
   row={'class':cls,'function':name,'address':hex(addr),'assembly':asm,'offset':offset,'operation':op,'width':width,'immediate':value}; reasons=[]
   if (cls,name) in ported: reasons.append('already_ported')
   if flags & (~0xffffffcf|NET|0x08000000|0x20000000|0x2000|0x40000000) or not flags&0x400 or (flags&0xe0000) not in ACCESS: reasons.append('unsupported_flags')
   if not re.fullmatch(r'[A-Za-z_]\w*',name) or not re.fullmatch(r'[A-Za-z_]\w*',cls): reasons.append('invalid_identifier')
   vs=byname.get(cls,[])
   if len(vs)!=1 or cls not in selected: reasons.append('class_not_unique_or_selected')
   if len(vs)==1:
    va=vs[0]; parent=parents.get(va,{}); block=blocks.get(va.lower()); pva=parent.get('parent_va')
    if not block or parent.get('name')!=cls or parent.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000: reasons.append('class_or_package')
    own=size(va); ps=size(pva) if isinstance(pva,str) else None; hs=headers.get(cls,[])
    row.update(own_size=own,parent_size=ps)
    if not own or not ps or len(hs)!=1: reasons.append('size_or_generated_header')
    else:
     declaration=re.findall(r'\bclass\s+\w+_API\s+'+re.escape(cls)+r'\s*:\s*public\s+(\w+)',hs[0].read_text())
     if declaration!=[parent.get('parent_name')] or names.get(pva)!=parent.get('parent_name'): reasons.append('generated_parent_mismatch')
     if re.search(r'^\s*[^\n;]*\b'+re.escape(name)+r'\s*\(',hs[0].read_text(),re.M) and not re.search(r'^\s*void\s+'+re.escape(name)+r'\(\)\s*;',hs[0].read_text(),re.M): reasons.append('function_header_mismatch')
     spans=occupancy(cls,block,own) if block else None
     if spans is None: reasons.append('unknown_occupancy')
     elif any(offset<end and start<offset+n for start,end in spans): reasons.append('occupied')
     if not (ps<=offset and offset+n<=own): reasons.append('outside_declaring_class')
     field=f'NativeRecoveredUInt{n*8}_0x{offset:x}'
     taken={p.get('name') for p in block['properties']} | set(funcs) | {e.get('function') for e in manifest if e.get('class')==cls}
     if field in taken: reasons.append('field_name_collision')
   row['reasons']=reasons; rows.append(row)
   if not reasons: accepted.append((row,flags,fn))
 # Reject partially overlapping proposals, even when individual intervals are vacant.
 bad=set()
 for i,(a,_,_) in enumerate(accepted):
  for j,(b,_,_) in enumerate(accepted[:i]):
   aw=1 if a['width']=='byte' else 4; bw=1 if b['width']=='byte' else 4
   if a['class']==b['class'] and a['offset']<b['offset']+bw and b['offset']<a['offset']+aw and (a['offset'],aw)!=(b['offset'],bw): bad.update((i,j))
 fields={}; entries=[]
 for i,(r,flags,fn) in enumerate(accepted):
  if i in bad: r['reasons'].append('conflicting_intervals'); continue
  cls=r['class']; name=r['function']; offset=r['offset']; n=1 if r['width']=='byte' else 4; typ=f'uint{n*8}'; field=f'NativeRecoveredUInt{n*8}_0x{offset:x}'; addr=int(r['address'],0); val=r['immediate'] & ((1<<(n*8))-1)
  fields[(cls,offset)]={'class':cls,'function':field,'kind':'member_field','signature':f'{typ} {field}','signature_status':'proposed','access':'public','offset':hex(offset),'inc_path':None,'confidence':'high','address':None,'status':'ported','recovery_source':'bulk_mutation_storage_runtime','notes':'Original complete mutation identifies whole-width storage; initialization and other writers remain unported.'}
  operator={'mov':'=','and':'&=','or':'|='}[r['operation']]
  inc=HERE/(f'{addr:x}_{cls}_{name}.inc'); body=f'// Original address: {hex(addr)}. Complete raw storage mutation.\n{field} {operator} 0x{val:x}u;\n'
  if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs: '+str(inc))
  inc.write_text(body)
  entries.append({'class':cls,'function':name,'kind':'exec','signature':f'void {name}()','signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':False,'access':ACCESS[flags&0xe0000],'notes':f'Complete original {r["operation"]} {r["width"]} at offset {hex(offset)}; raw storage, source recovery only.'})
 out=Path('/tmp/gameplay-bulk-mutation-storage'); out.mkdir(parents=True,exist_ok=True)
 (out/'manifest-entries.json').write_text(json.dumps(list(fields.values())+entries,indent=2)+'\n')
 report={'image_sha256':digest,'inventory_records':total,'candidates':len(entries),'fields':len(fields),'matches':rows}
 (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps({'inventory_records':total,'candidates':len(entries),'fields':len(fields),'matches':rows},indent=2))
if __name__=='__main__': main()
