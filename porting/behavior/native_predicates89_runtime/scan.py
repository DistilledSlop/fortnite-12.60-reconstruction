#!/usr/bin/env python3
"""Find whole no-argument exec getters of existing, accessible known fields.

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
 md=Cs(CS_ARCH_X86,CS_MODE_64); rows=[]; entries=[]
 prefix='mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
 wrap='push rbx; sub rsp, 0x20; mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; mov rbx, r8; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
 with IMAGE.open('rb') as f, mmap.mmap(f.fileno(),0,access=mmap.ACCESS_READ) as image:
  def decode(addr):
   pos=addr-BASE
   if pos<0 or pos+128>len(image): return None,None
   ins=[]; end=addr
   for i in md.disasm(image[pos:pos+128],addr):
    ins.append((i.mnemonic+' '+i.op_str).rstrip()); end=i.address+i.size
    if i.mnemonic=='ret' or i.mnemonic.startswith('j'): return '; '.join(ins),image[pos:pos+end-addr]
   return None,None
  def parse(code,store):
   # Direct load, normalized bit test, and shifted bit extract only.
   suffix={'inline':{'byte':'mov byte ptr [r8], al; ret','dword':'mov dword ptr [r8], eax; ret','float':'movss dword ptr [r8], xmm0; ret'},'leaf':{'byte':'ret','dword':'ret','float':'ret'}}[store]
   operand=r'(0x[0-9a-f]+|[0-9]+)'
   pats=[('byte',r'movzx eax, byte ptr \[rcx \+ '+operand+r'\]; '+re.escape(suffix['byte'])),('dword',r'mov eax, dword ptr \[rcx \+ '+operand+r'\]; '+re.escape(suffix['dword'])),('float',r'movss xmm0, dword ptr \[rcx \+ '+operand+r'\]; '+re.escape(suffix['float']))]
   for typ,pat in pats:
    m=re.fullmatch(pat,code)
    if m:return typ,int(m.group(1),0),None,False
   for pat in [r'test byte ptr \[rcx \+ '+operand+r'\], '+operand+r'; (sete|setne) al; ',r'movzx eax, byte ptr \[rcx \+ '+operand+r'\]; test (?:al|eax), '+operand+r'; (sete|setne) al; ']:
    m=re.fullmatch(pat+re.escape(suffix['byte']),code)
    if m:
     mask=int(m.group(2),0)
     if 0<mask<256 and mask&(mask-1)==0:return 'bit',int(m.group(1),0),mask,m.group(3)=='sete'
   m=re.fullmatch(r'movzx eax, byte ptr \[rcx \+ '+operand+r'\]; shr (al|eax), '+operand+r'; and \2, 1; '+re.escape(suffix['byte']),code)
   if m and 1<=int(m.group(3),0)<=7:return 'bit',int(m.group(1),0),1<<int(m.group(3),0),False
   # Whole-body scalar comparisons; capstone syntax preserves operand width and signed condition.
   op=r'(0x[0-9a-f]+|[0-9]+)'
   for width in ('byte','dword'):
    pattern=r'cmp '+width+r' ptr \[rcx \+ '+op+r'\], '+op+r'; (sete|setne|setl|setle|setg|setge|setb|setbe|seta|setae) al; '+re.escape(suffix['byte'])
    m=re.fullmatch(pattern,code)
    if m:return 'cmp'+width,int(m.group(1),0),int(m.group(2),0),m.group(3)
   for width,reg in (('byte','al'),('dword','eax')):
    pattern=r'mov'+('zx' if width=='byte' else '')+r' eax, '+width+r' ptr \[rcx \+ '+op+r'\]; test '+reg+', '+reg+r'; (sete|setne|setl|setle|setg|setge) al; '+re.escape(suffix['byte'])
    m=re.fullmatch(pattern,code)
    if m:return 'cmp'+width,int(m.group(1),0),0,m.group(2)
   return None
  for cls,funcs in sorted(native.items()):
   if cls not in selected or len(byname.get(cls,[]))!=1 or not header(cls): continue
   va=byname[cls][0]; parent=parents.get(va,{}); block=blocks.get(va.lower()); h=header(cls)[0]
   if not block or parent.get('name')!=cls or parent.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000: continue
   if re.findall(r'\bclass\s+\w+_API\s+'+re.escape(cls)+r'\s*:\s*public\s+(\w+)',h)!=[parent.get('parent_name')] or names.get(parent.get('parent_va'))!=parent.get('parent_name'): continue
   for name,fn in sorted(funcs.items()):
    if (cls,name) in ported or not isinstance(fn,dict): continue
    ps=fn.get('params')
    if not isinstance(ps,list) or len(ps)!=1 or ps[0].get('name')!='ReturnValue' or ps[0].get('array_dim')!=1: continue
    try: pf=int(ps[0].get('property_flags','0'),0); flags=int(fn['flags'],0); addr=int(fn['exec_addr'],0)
    except (ValueError,TypeError,KeyError): continue
    kind=ps[0].get('kind')
    if not pf&0x400 or pf&0x08000000 or kind not in ('Bool','Int','Float'): continue
    if flags & (~0xffffffcf|NET|0x08000000|0x20000000|0x2000) or not flags&0x400 or (flags&0xe0000) not in ACCESS: continue
    if not re.fullmatch(r'[A-Za-z_]\w*',name) or re.search(r'^\s*[^\n;]*\b'+re.escape(name)+r'\s*\(',h,re.M): continue
    asm,raw=decode(addr)
    if not asm: continue
    result=None; leaf=None; leafraw=None; target=None; form=None
    if asm.startswith(prefix):
     inner=asm[len(prefix):]; result=parse(inner,'inline'); form='inline'
     j=re.fullmatch(r'jmp (0x[0-9a-f]+)',inner)
     if j:
      target=int(j.group(1),16); leaf,leafraw=decode(target); result=parse(leaf or '', 'inline'); form='tailjump'
    elif asm.startswith(wrap):
     m=re.fullmatch(r'call (0x[0-9a-f]+); (mov (?:byte|dword) ptr \[rbx\], (?:al|eax)|movss dword ptr \[rbx\], xmm0); add rsp, 0x20; pop rbx; ret',asm[len(wrap):])
     if m:
      target=int(m.group(1),16); leaf,leafraw=decode(target); result=parse(leaf or '', 'leaf'); form='call'
      stores={'byte':'mov byte ptr [rbx], al','bit':'mov byte ptr [rbx], al','dword':'mov dword ptr [rbx], eax','float':'movss dword ptr [rbx], xmm0','cmpbyte':'mov byte ptr [rbx], al','cmpdword':'mov byte ptr [rbx], al'}
      if result and stores[result[0]]!=m.group(2): result=None
    if not result or not result[0].startswith('cmp'): continue
    typ,off,mask,neg=result; width=1 if typ in ('byte','bit','cmpbyte') else 4; ts=targets(cls,off,width); body=None
    if typ=='bit': ts=[t for t in ts if t[4]=='Bool' and t[3]==mask]
    if len(ts)==1:
     owner,field,fieldtype,fieldmask,fieldkind=ts[0]
     if kind=='Bool' and typ in ('cmpbyte','cmpdword') and fieldtype in ('uint8','int32','uint32'):
      cond={'sete':'==','setne':'!=','setl':'<','setle':'<=','setg':'>','setge':'>=','setb':'<','setbe':'<=','seta':'>','setae':'>='}[neg]
      signed=neg in ('setl','setle','setg','setge'); unsigned=neg in ('setb','setbe','seta','setae')
      cast=('int8' if typ=='cmpbyte' else 'int32') if signed else (('uint8' if typ=='cmpbyte' else 'uint32') if unsigned else fieldtype)
      value=mask; value=value-(1<<(width*8)) if signed and value >= (1<<(width*8-1)) else value
      body=f'static_cast<{cast}>({field}) {cond} {value}'
     elif kind=='Bool' and typ=='bit' and fieldkind=='Bool' and mask==fieldmask: body=('!' if neg else '')+field
     elif kind=='Bool' and typ=='byte' and fieldtype=='bool' and fieldkind=='native': body=field
     elif kind=='Int' and typ=='dword' and fieldtype=='int32': body=field
     elif kind=='Float' and typ in ('dword','float') and fieldtype=='float': body=field
    row={'class':cls,'function':name,'address':hex(addr),'assembly':asm,'bytes':raw.hex(),'sha256':hashlib.sha256(raw).hexdigest(),'target':hex(target) if target is not None else None,'leaf_assembly':leaf,'leaf_bytes':leafraw.hex() if leafraw else None,'leaf_sha256':hashlib.sha256(leafraw).hexdigest() if leafraw else None,'targets':ts,'accepted':bool(body)}; rows.append(row)
    if not body: continue
    if len(entries)>=20: row['accepted']=False; row['reason']='proposal_limit'; continue
    inc=HERE/(f'{addr:x}_{cls}_{name}.inc'); text='// CONFIDENCE: high; complete exec and leaf evidence in report.json.\nreturn '+body+';\n'
    if inc.exists() and inc.read_text()!=text: raise SystemExit('existing body differs: '+str(inc))
    inc.write_text(text); rettype={'Bool':'bool','Int':'int32','Float':'float'}[kind]
    entries.append({'class':cls,'function':name,'kind':'exec','signature':f'{rettype} {name}()'+(' const' if flags&0x40000000 else ''),'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':False,'access':ACCESS[flags&0xe0000],'notes':f'Complete {form} read of existing {owner}::{field} at {hex(off)}; see native_predicates89_runtime/report.json.'})
 (HERE/'manifest-entries.json').write_text(json.dumps(entries,indent=2)+'\n')
 (HERE/'scan-report.json').write_text(json.dumps({'image':str(IMAGE),'expected_image_sha256':SHA,'digest_verified_this_run':False,'matches':rows,'proposals':len(entries)},indent=2)+'\n')
 print('matches:',len(rows),'proposals:',len(entries))
if __name__=='__main__': main()
