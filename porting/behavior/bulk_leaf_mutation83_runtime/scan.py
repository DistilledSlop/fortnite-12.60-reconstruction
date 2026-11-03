#!/usr/bin/env python3
"""Find no-argument exec tail calls to complete constant-write leaves.

Never creates storage. Reads the previously pinned mapped image by mmap; caller
must independently establish its identity when the evidence image changes.
"""
import collections, hashlib, json, mmap, re, struct, sys
from pathlib import Path
sys.path.insert(0, '/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
ROOT = Path(__file__).resolve().parents[3]
REFL = ROOT/'porting/generated/reflection'
sys.path.insert(0, str(ROOT/'porting'))
from gen_native_classes import split_properties
HERE = Path(__file__).resolve().parent
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
BASE = 0x140000000
SHA = '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
load = lambda p: json.loads(p.read_text())
ACCESS = {0x20000:'public', 0x40000:'private', 0x80000:'protected'}
NET = 0x80000000|0x01000000|0x00200000|0x4000|0x1000|0x100|0x80|0x40
PREFIXES = ['mov rax, qword ptr [rdx + 0x20]; xor r8d, r8d; test rax, rax; setne r8b; add r8, rax; mov qword ptr [rdx + 0x20], r8; ', 'mov r8, qword ptr [rdx + 0x20]; xor eax, eax; test r8, r8; setne al; add rax, r8; mov qword ptr [rdx + 0x20], rax; ']
PAT = re.compile(r'(mov|and|or) (byte|dword) ptr \[rcx \+ (0x[0-9a-f]+)\], (0x[0-9a-f]+|[0-9]+); ret')

def main():
 native = load(REFL/'native_funcs.json'); names = load(REFL/'class_name_map.json'); parents = load(REFL/'class_parent_map.json')
 blocks = {b['block_va'].lower():b for b in load(REFL/'fortnite_game_reflection.json')['classes_in_groups_with_fort_names']}
 bits = load(REFL/'bool_bits.json')['by_class']; manifest = load(ROOT/'porting/behavior/MANIFEST.json')
 # These proposals are being integrated concurrently and must never be emitted twice.
 pending = ROOT/'porting/behavior/fresh_native82_runtime/manifest-additions.json'
 if pending.exists(): manifest += load(pending)
 ported = {(e.get('class'),e.get('function')) for e in manifest if e.get('status') == 'ported'}
 byname = collections.defaultdict(list)
 for va,name in names.items(): byname[name].append(va)
 selected = {x.strip() for x in (ROOT/'porting/native_class_set.txt').read_text().splitlines() if x.strip() and not x.lstrip().startswith('#')}
 headers = collections.defaultdict(list)
 for h in (ROOT/'porting/native/Source').glob('*/Generated/*.h'): headers[h.stem].append(h)
 header_cache = {}
 def header(cls):
  if cls in header_cache: return header_cache[cls]
  if len(headers.get(cls,[])) != 1: return None
  text = headers[cls][0].read_text(); fields = {}; access = 'private'
  for line in text.splitlines():
   m = re.match(r'^\s*(public|protected|private):\s*$',line)
   if m: access = m.group(1)
   m = re.match(r'^\s*(bool|uint8|int32|uint32|float)\s+(\w+)\s*(?::\s*1)?\s*;', line)
   if m: fields.setdefault(m.group(2),[]).append((m.group(1),access))
  header_cache[cls] = (text,fields); return text,fields
 def chain(cls):
  # Reject missing/ambiguous ancestry rather than guessing inherited access.
  result=[]; seen=set(); current=cls
  while current and current not in seen:
   seen.add(current); vs=byname.get(current,[])
   if len(vs)!=1: break
   va=vs[0]; b=blocks.get(va.lower()); h=header(current); p=parents.get(va,{})
   if not b or not h: break
   if p.get('name') != current: break
   result.append((current,b,h[1]))
   par=p.get('parent_name'); pva=p.get('parent_va')
   if par and names.get(pva)!=par: break
   current=par
  return result
 def targets(cls, offset, width):
  out=[]; earlier_names=set()
  for owner,block,fields in chain(cls):
   pairs,notes=split_properties(block.get('properties',[]))
   if notes: return []
   for prop,_ in pairs:
    name=prop.get('name'); kind=prop.get('kind'); dim=prop.get('array_dim',1)
    if dim!=1: continue
    if kind=='Bool':
     bm=[x for x in bits.get(owner,[]) if x['name']==name]
     if len(bm)!=1: continue
     loc=bm[0].get('offset'); mask=bm[0].get('mask'); typ='bool'; span=1
    else:
     loc=prop.get('offset'); mask=None; typ={'Byte':'uint8','Int':'int32','UInt32':'uint32','Float':'float'}.get(kind); span=1 if typ=='uint8' else 4
    if not typ or loc!=offset or span!=width or name in earlier_names: continue
    decl=fields.get(name,[])
    if len(decl)!=1 or decl[0][1]=='private' and owner!=cls: continue
    actual=decl[0][0]
    if kind=='Bool' and actual not in ('bool','uint8'): continue
    if kind!='Bool' and actual!=typ: continue
    out.append((owner,name,actual,mask,kind))
   for e in manifest:
    if e.get('class')!=owner or e.get('kind')!='member_field' or e.get('status')!='ported': continue
    try: loc=int(str(e['offset']),0)
    except (KeyError,ValueError,TypeError): continue
    m=re.fullmatch(r'(bool|uint8|int32|uint32|float)\s+(\w+)',e.get('signature',''))
    if not m or loc!=offset: continue
    typ,name=m.groups(); span=1 if typ in ('bool','uint8') else 4
    decl=fields.get(name,[])
    if span==width and name not in earlier_names and len(decl)==1 and decl[0][0]==typ and (decl[0][1]!='private' or owner==cls): out.append((owner,name,typ,None,'native'))
   earlier_names.update(fields)
  return out
 def expression(target,op,width,val):
  owner,name,typ,mask,kind=target
  if kind=='Bool':
   if op=='or' and mask and val==mask: return name+' = true;'
   if op=='and' and mask and val==(0xff ^ mask): return name+' = false;'
   if typ=='bool' and mask==1 and op=='mov' and val in (0,1): return name+' = '+('true' if val else 'false')+';'
   return None
  if typ=='bool': return name+' = '+('true' if val else 'false')+';' if op=='mov' and val in (0,1) else None
  if typ=='float':
   if op!='mov': return None
   f=struct.unpack('<f',struct.pack('<I',val))[0]
   if not __import__('math').isfinite(f): return None
   if f not in (0.0,1.0,-1.0): return None
   return name+' = '+repr(f)+'f;'
  if op!='mov': return None
  if typ=='int32': val=val if val<0x80000000 else val-0x100000000
  return name+' = '+str(val)+('u' if typ in ('uint8','uint32') else '')+';'
 md=Cs(CS_ARCH_X86,CS_MODE_64); rows=[]; entries=[]
 with IMAGE.open('rb') as f, mmap.mmap(f.fileno(),0,access=mmap.ACCESS_READ) as image:
  for cls,funcs in sorted(native.items()):
   if cls not in selected or len(byname.get(cls,[]))!=1 or not header(cls): continue
   va=byname[cls][0]; parent=parents.get(va,{}); block=blocks.get(va.lower()); h=header(cls)[0]
   if not block or parent.get('name')!=cls or parent.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000: continue
   decl=re.findall(r'\bclass\s+\w+_API\s+'+re.escape(cls)+r'\s*:\s*public\s+(\w+)',h)
   if decl!=[parent.get('parent_name')] or names.get(parent.get('parent_va'))!=parent.get('parent_name'): continue
   for name,fn in sorted(funcs.items()):
    if (cls,name) in ported or not isinstance(fn,dict) or fn.get('params')!=[]: continue
    try: flags=int(fn['flags'],0); addr=int(fn['exec_addr'],0)
    except (ValueError,TypeError,KeyError): continue
    if flags & (~0xffffffcf|NET|0x08000000|0x20000000|0x2000|0x40000000) or not flags&0x400 or (flags&0xe0000) not in ACCESS: continue
    if not re.fullmatch(r'[A-Za-z_]\w*',name) or re.search(r'^\s*[^\n;]*\b'+re.escape(name)+r'\s*\(',h,re.M): continue
    pos=addr-BASE
    if pos<0 or pos+128>len(image): continue
    ins=[]; end=addr
    for i in md.disasm(image[pos:pos+128],addr):
     ins.append((i.mnemonic+' '+i.op_str).rstrip()); end=i.address+i.size
     if i.mnemonic=='ret' or i.mnemonic.startswith('j'): break
    asm='; '.join(ins); leafaddr=None
    for prefix in PREFIXES:
     if asm.startswith(prefix):
      tail=asm[len(prefix):]
      m=re.fullmatch(r'jmp (0x[0-9a-f]+)',tail)
      if m: leafaddr=int(m.group(1),0)
      break
    if leafaddr is None: continue
    lp=leafaddr-BASE
    if lp<0 or lp+64>len(image): continue
    leaf=[]; leafend=leafaddr
    for i in md.disasm(image[lp:lp+64],leafaddr):
     leaf.append((i.mnemonic+' '+i.op_str).rstrip()); leafend=i.address+i.size
     if i.mnemonic=='ret' or i.mnemonic.startswith('j'): break
    leafasm='; '.join(leaf); match=PAT.fullmatch(leafasm)
    if not match: continue
    op,width,off,imm=match.groups(); off=int(off,0); val=int(imm,0); n=1 if width=='byte' else 4
    ts=targets(cls,off,n)
    eligible=[]
    for t in ts:
     candidate=expression(t,op,n,val)
     if not candidate: continue
     owner,field,typ,mask,kind=t
     decl=header(owner)[0]
     if kind=='Bool':
      # Packed-byte RMW requires a real bitfield; whole-byte MOV requires
      # an ordinary bool, so C++ assignment reproduces the full write.
      suffix=r'\s*:\s*1' if op in ('and','or') else r'\s*'
      if not re.search(r'^\s*'+re.escape(typ)+r'\s+'+re.escape(field)+suffix+r'\s*;',decl,re.M): continue
     eligible.append((t,candidate))
    body=eligible[0][1] if len(eligible)==1 else None
    if body: ts=[eligible[0][0]]
    raw=image[pos:pos+end-addr]
    row={'class':cls,'function':name,'address':hex(addr),'assembly':asm,'bytes':raw.hex(),'leaf_address':hex(leafaddr),'leaf_assembly':leafasm,'leaf_bytes':image[lp:lp+leafend-leafaddr].hex(),'leaf_sha256':hashlib.sha256(image[lp:lp+leafend-leafaddr]).hexdigest(),'sha256':hashlib.sha256(raw).hexdigest(),'targets':ts,'accepted':bool(body)}; rows.append(row)
    if not body: continue
    if len(entries)>=20: row['accepted']=False; row['reason']='proposal_limit'; continue
    inc=HERE/(f'{addr:x}_{cls}_{name}.inc'); text='// CONFIDENCE: high; complete native exec at '+hex(addr)+'.\n'+body+'\n'
    if inc.exists() and inc.read_text()!=text: raise SystemExit('existing body differs: '+str(inc))
    inc.write_text(text)
    entries.append({'class':cls,'function':name,'kind':'exec','signature':f'void {name}()','signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':False,'access':ACCESS[flags&0xe0000],'notes':f'Complete no-argument tail-call mutation of existing {ts[0][0]}::{ts[0][1]}; original {op} {width} offset {hex(off)}. See bulk_leaf_mutation83_runtime/report.json.'})
 (HERE/'manifest-entries.json').write_text(json.dumps(entries,indent=2)+'\n')
 (HERE/'report.json').write_text(json.dumps({'image':str(IMAGE),'expected_image_sha256':SHA,'digest_verified_this_run':False,'matches':rows,'proposals':len(entries)},indent=2)+'\n')
 print('matches:',len(rows),'proposals:',len(entries))
if __name__=='__main__': main()
