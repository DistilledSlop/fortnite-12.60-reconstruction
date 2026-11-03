#!/usr/bin/env python3
"""Recover one-bool setters (direct pure-leaf call and inline pure store) from exact FFrame exec thunks.

Extends bulk_parameter_setter_runtime (int32) to bool. Same pinned image, branch-target and class/flag guards.
Source generation only; writes proposals/evidence under /tmp/gameplay-bulk-byte-setter and local .inc files."""
import collections,hashlib,json,re,importlib.util
from pathlib import Path
import sys
sys.path.insert(0,'/tmp/common-input-capstone')
from capstone import Cs,CS_ARCH_X86,CS_MODE_64
ROOT=Path(__file__).resolve().parents[3]; HERE=Path(__file__).resolve().parent; REFL=ROOT/'porting/generated/reflection'; OUT=Path('/tmp/gameplay-bulk-native-setter')
sys.path.insert(0,str(ROOT/'porting'))
import gen_native_classes as GEN   # split_properties: excludes enum/array/map/set metadata children
IMAGE=Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'); SHA='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'; BASE=0x140000000
load=lambda p:json.loads(p.read_text()); ACCESS={0x20000:'public',0x40000:'private',0x80000:'protected'}; NET=0x80000000|0x01000000|0x00200000|0x4000|0x1000|0x100|0x80|0x40
H=r'(0x[0-9a-f]+|[0-9]+)'
# Exact bool parameter extraction and normalized byte argument.
HEAD=(r'mov qword ptr \[rsp \+ H\], rbx; mov qword ptr \[rsp \+ H\], rsi; push rdi; sub rsp, 0x20; xor edi, edi; mov rsi, rcx; mov rbx, rdx; mov dword ptr \[rsp \+ H\], edi; mov rcx, rdx; cmp qword ptr \[rdx \+ 0x20\], rdi; je (0x[0-9a-f]+); mov rdx, qword ptr \[rdx \+ 0x18\]; lea r8, \[rsp \+ H\]; call 0x142ed9cf0; jmp (0x[0-9a-f]+); mov r8, qword ptr \[rdx \+ 0x80\]; mov rax, qword ptr \[r8 \+ 0x20\]; mov qword ptr \[rdx \+ 0x80\], rax; lea rdx, \[rsp \+ H\]; call 0x142ed9d20; mov rax, qword ptr \[rbx \+ 0x20\]; mov rcx, rsi; test rax, rax; setne dil; add rdi, rax; cmp dword ptr \[rsp \+ H\], 0; mov qword ptr \[rbx \+ 0x20\], rdi; setne dl; ').replace('H',H)
DIRECT=re.compile(HEAD+r'call (0x[0-9a-f]+); mov rbx, qword ptr \[rsp \+ H\]; mov rsi, qword ptr \[rsp \+ H\]; add rsp, 0x20; pop rdi; ret'.replace('H',H))
LEAF=re.compile(r'mov byte ptr \[rcx( \+ 0x[0-9a-f]+)?\], dl; ret')
def load_scanner(name):
 spec=importlib.util.spec_from_file_location(name, ROOT/'porting/behavior'/name/'prepare.py')
 module=importlib.util.module_from_spec(spec); spec.loader.exec_module(module); return module
INT=load_scanner('bulk_parameter_setter_runtime'); FLOAT=load_scanner('bulk_float_setter_runtime')
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
 cache={}; rejects=collections.Counter(); shapes=collections.Counter(); results=[]; rows=[]; total=0
 bits=load(REFL/'bool_bits.json')['by_class']; manifest=load(ROOT/'porting/behavior/MANIFEST.json')
 def size(va):
  pairs,_=GEN.split_properties(blocks.get(va.lower(),{}).get('properties',[]))
  ss={p.get('size_of_outer') for p,_ in pairs if p.get('kind')=='Bool' and isinstance(p.get('size_of_outer'),int) and p['size_of_outer']>0}
  return next(iter(ss)) if len(ss)==1 else None
 def occupancy(cls,block,own):
  pairs,notes=GEN.split_properties(block['properties'])
  if notes: return None
  locations=[]
  for p,_ in pairs:
   if p.get('kind')=='Bool':
    bs=[b for b in bits.get(cls,[]) if b['name']==p['name']]
    if len(bs)!=1 or not isinstance(bs[0].get('offset'),int): return None
    loc=bs[0]['offset']
   else: loc=p.get('offset')
   if not isinstance(loc,int) or loc<0 or loc>=own: return None
   locations.append((loc,p))
  for e in manifest:
   if e.get('class')!=cls or e.get('kind')!='member_field' or e.get('status')!='ported': continue
   try: loc=int(str(e['offset']),0)
   except (KeyError,ValueError,TypeError): return None
   if loc<0 or loc>=own: return None
   locations.append((loc,e))
  offsets=sorted({loc for loc,_ in locations}|{own}); spans=[]
  widths={'Bool':1,'Byte':1,'Int8':1,'Int16':2,'UInt16':2,'Int':4,'UInt32':4,'Float':4,'Int64':8,'UInt64':8,'Double':8,'Name':8,'Object':8,'Class':8,'Array':16,'Str':16}
  for loc,p in locations:
   kind=p.get('kind'); width=widths.get(kind)
   if kind=='member_field': width={'bool':1,'uint8':1,'int8':1,'int16':2,'uint16':2,'int32':4,'uint32':4,'float':4,'int64':8,'uint64':8,'double':8}.get((p.get('signature') or '').split(' ',1)[0])
   dim=p.get('array_dim',1)
   if not isinstance(dim,int) or dim<1: return None
   end=loc+width*dim if width else next(x for x in offsets if x>loc)
   if end>own: return None
   spans.append((loc,end))
  return spans
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
   kind=p.get('kind')
   if kind not in ('Bool','Int','Float'): rejects['parameter_not_scalar']+=1; continue
   pf=int(p.get('property_flags','0'),0)
   if p.get('array_dim')!=1 or p.get('type_refs') or pf&(0x400|0x100|0x08000000) or not pf&0x80 or not re.fullmatch(r'[A-Za-z_]\w*',pname): rejects['parameter_flags_or_name']+=1; continue
   try: flags=int(fn['flags'],0); addr=int(fn['exec_addr'],0)
   except (KeyError,ValueError,TypeError): rejects['address_or_flags']+=1; continue
   if flags & (~0xffffffcf|NET|0x08000000|0x20000000|0x2000|0x40000000) or not flags&0x400 or (flags&0xe0000) not in ACCESS: rejects['function_flags']+=1; continue
   if (cls,name) in ported: rejects['already_ported']+=1; continue
   vs=byname.get(cls,[])
   if len(vs)!=1 or cls not in selected or not re.fullmatch(r'[A-Za-z_]\w*',name): rejects['class_or_identifier']+=1; continue
   par=parents.get(vs[0],{}); block=blocks.get(vs[0].lower())
   if not block or par.get('name')!=cls or par.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000: rejects['class_metadata']+=1; continue
   ins,asm=decode(addr); target=None; imm=None; mode='direct'; leaf=''
   if kind=='Bool':
    m=DIRECT.fullmatch(asm)
    if not m: rejects['unmatched_thunk_shape']+=1; continue
    g=m.groups(); loc=g[2]
    if not loc==g[4]==g[6]==g[7] or int(g[0],0)+0x28!=int(g[9],0) or int(g[1],0)+0x28!=int(g[10],0) or int(loc,0) in (int(g[9],0),int(g[10],0)) or not 0x20<=int(loc,0)<0x40: rejects['inconsistent_stack_local']+=1; continue
    target=int(g[8],0); leaf=decode(target)[1]; q=LEAF.fullmatch(leaf)
    if not q: rejects['callee_not_pure_single_store']+=1; continue
    imm=q.group(1); offset=int(imm.split('+')[1],16) if imm else 0
   elif kind=='Int':
    m=INT.PAT.fullmatch(asm)
    if not m: rejects['unmatched_thunk_shape']+=1; continue
    g=m.groups(); loc=g[3]
    if g[2:5]!=('dword',loc,'edi') or g[6]!=loc or g[8]!=loc or g[9:11]!=('edx','dword') or g[11]!=loc or int(g[0],0)+0x28!=int(g[13],0) or int(g[1],0)+0x28!=int(g[14],0) or int(loc,0) in (int(g[13],0),int(g[14],0)) or not 0x20<=int(loc,0)<0x40: rejects['inconsistent_stack_local']+=1; continue
    target=int(g[12],0); leaf=decode(target)[1]; q=re.fullmatch(r'mov dword ptr \[rcx \+ (0x[0-9a-f]+)\], edx; ret',leaf)
    if not q: rejects['callee_not_pure_single_store']+=1; continue
    offset=int(q.group(1),0)
   else:
    m=FLOAT.DIRECT.fullmatch(asm)
    if not m: m=FLOAT.INLINE.fullmatch(asm); mode='inline'
    if not m: rejects['unmatched_thunk_shape']+=1; continue
    g=m.groups(); save,loc,reg=g[0],g[1],g[6]
    if not loc==g[3]==g[5]==g[7]: rejects['inconsistent_stack_local']+=1; continue
    if mode=='direct':
     target=int(g[8],0); restore=g[9]
     if reg!='xmm1': rejects['register_mismatch']+=1; continue
     leaf=decode(target)[1]; q=FLOAT.LEAF.fullmatch(leaf)
     if not q: rejects['callee_not_pure_single_store']+=1; continue
     imm=q.group(1)
    else:
     restore=g[8]; imm=g[9]; target=addr; leaf='(inline)'
     if reg!='xmm0': rejects['register_mismatch']+=1; continue
    if int(save,0)+0x28!=int(restore,0) or int(loc,0)==int(restore,0) or not 0x20<=int(loc,0)<0x40: rejects['inconsistent_stack_local']+=1; continue
    offset=int(imm.split('+')[1],16) if imm else 0
   je=next((i for i in ins if i.mnemonic=='je'),None); jmp=next((i for i in ins if i.mnemonic=='jmp'),None)
   alt=next((i for i in ins if i.mnemonic=='mov' and i.op_str=='r8, qword ptr [rdx + 0x80]'),None); join=next((ins[k+1] for k,i in enumerate(ins[:-1]) if i.mnemonic=='call' and i.op_str=='0x142ed9d20'),None)
   if not (je and jmp and alt and join and int(je.op_str,0)==alt.address and int(jmp.op_str,0)==join.address): rejects['bad_branch_targets']+=1; continue
   width=1 if kind=='Bool' else 4; typ={'Bool':'bool','Int':'int32','Float':'float'}[kind]
   row={'class':cls,'function':name,'address':hex(addr),'offset':offset,'exec':asm,'writer':leaf,'target':hex(target),'kind':kind,'width':width}; reasons=[]
   own=size(vs[0]); pva=par.get('parent_va'); ps=size(pva) if isinstance(pva,str) else None; hs=headers.get(cls,[])
   row.update(own_size=own,parent_size=ps)
   if not own or not ps or len(hs)!=1: reasons.append('size_or_generated_header')
   else:
    hdr=cache.setdefault(cls,hs[0].read_text())
    declaration=re.findall(r'\bclass\s+\w+_API\s+'+re.escape(cls)+r'\s*:\s*public\s+(\w+)',hdr)
    if declaration!=[par.get('parent_name')] or names.get(pva)!=par.get('parent_name'): reasons.append('generated_parent_mismatch')
    sig=f'void {name}({typ} {pname})'
    decl=re.search(r'^\s*[^\n;]*\b'+re.escape(name)+r'\s*\(',hdr,re.M)
    if decl and not re.search(r'^\s*'+re.escape(sig)+r'\s*;',hdr,re.M): reasons.append('function_header_mismatch')
    spans=occupancy(cls,block,own)
    if spans is None: reasons.append('unknown_occupancy')
    elif any(offset<end and start<offset+width for start,end in spans): reasons.append('occupied')
    if not (ps<=offset and offset+width<=own): reasons.append('outside_declaring_class')
    field=f'NativeRecovered{kind}_0x{offset:x}'
    taken={x.get('name') for x in block['properties']} | set(funcs) | {e.get('function') for e in manifest if e.get('class')==cls}
    if field in taken: reasons.append('field_name_collision')
   row['reasons']=reasons; rows.append(row)
   if not reasons: results.append((row,flags,fn,pname))
 # Conflicting storage proposals must all be rejected.
 bad=set()
 for i,(a,*_) in enumerate(results):
  for j,(b,*_) in enumerate(results[:i]):
   if a['class']==b['class'] and a['offset']<b['offset']+b['width'] and b['offset']<a['offset']+a['width'] and (a['offset'],a['width'],a['kind'])!=(b['offset'],b['width'],b['kind']): bad.update((i,j))
 fields={}; entries=[]
 for i,(r,flags,fn,pname) in enumerate(results):
  if i in bad: r['reasons'].append('conflicting_intervals'); continue
  cls=r['class']; name=r['function']; offset=r['offset']; kind=r['kind']; typ={'Bool':'bool','Int':'int32','Float':'float'}[kind]; addr=int(r['address'],0); field=f'NativeRecovered{kind}_0x{offset:x}'
  fields[(cls,offset)]={'class':cls,'function':field,'kind':'member_field','signature':f'{typ} {field}','signature_status':'proposed','access':'public','offset':hex(offset),'inc_path':None,'confidence':'high','address':None,'status':'ported','recovery_source':'bulk_native_setter_runtime','notes':'Complete original setter identifies scalar storage; initialization and other writers remain unported.'}
  inc=HERE/f'{addr:x}_{cls}_{name}.inc'; body=f'// Original exec {hex(addr)}; pure writer {r["target"]}.\n// CONFIDENCE: HIGH\nthis->{field} = {pname};\n'
  if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs: '+str(inc))
  inc.write_text(body)
  entries.append({'class':cls,'function':name,'kind':'exec','signature':f'void {name}({typ} {pname})','signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':False,'access':ACCESS[flags&0xe0000],'notes':f'Exact FFrame extraction and pure scalar store at {r["target"]}, offset {hex(offset)}; source recovery only.'})
 OUT.mkdir(parents=True,exist_ok=True)
 (OUT/'manifest-entries.json').write_text(json.dumps(list(fields.values())+entries,indent=2)+'\n')
 report={'image_sha256':digest,'inventory_records':total,'candidates':len(entries),'fields':len(fields),'rejections':dict(rejects),'matches':rows}
 (OUT/'report.json').write_text(json.dumps(report,indent=2)+'\n'); print(json.dumps({k:report[k] for k in ('inventory_records','candidates','fields','rejections')},indent=2))
if __name__=='__main__': main()
