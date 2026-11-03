#!/usr/bin/env python3
"""Fail-closed direct reflected embedded struct Boolean leaf recovery."""
import collections, hashlib, json, re, sys, tempfile, importlib.util
from pathlib import Path
sys.path.insert(0,'/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
spec=importlib.util.spec_from_file_location('virt',Path(__file__).resolve().parents[1]/'bulk_virtual_scalar_runtime/prepare.py'); virt=importlib.util.module_from_spec(spec); spec.loader.exec_module(virt)
import build_vtable_slots as vt
ROOT=Path(__file__).resolve().parents[3]; REF=ROOT/'porting/generated/reflection'; HERE=Path(__file__).resolve().parent
IMAGE=Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'); BASE=0x140000000
SHA='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
load=lambda p:json.loads(p.read_text())
NET=0x80000000|0x01000000|0x00200000|0x4000|0x1000|0x100|0x80|0x40
ACCESS={0x20000:'public',0x40000:'private',0x80000:'protected'}
PREFIX='mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
WRAP='push rbx; sub rsp, 0x20; mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; mov rbx, r8; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
def main():
 image=IMAGE.read_bytes()
 if hashlib.sha256(image).hexdigest()!=SHA: raise SystemExit('image digest mismatch')
 md=Cs(CS_ARCH_X86,CS_MODE_64)
 def decode(addr):
  pos=addr-BASE
  if pos<0 or pos+128>len(image): return ''
  ins=[]
  for i in md.disasm(image[pos:pos+128],addr):
   ins.append((i.mnemonic+' '+i.op_str).rstrip())
   if i.mnemonic=='ret': return '; '.join(ins)
   if i.mnemonic.startswith('j'): return ''
  return ''
 native=load(REF/'native_funcs.json'); names=load(REF/'class_name_map.json'); parents=load(REF/'class_parent_map.json'); reflection=load(REF/'fortnite_game_reflection.json')
 blocks={x['block_va'].lower():x for x in reflection['classes_in_groups_with_fort_names']}; structs={x['block_va'].lower():x for x in load(REF/'struct_blocks.json')['structs']}; cmap=load(REF/'construct_fn_to_block.json')['map']; manifest=load(ROOT/'porting/behavior/MANIFEST.json')
 used={(e.get('class'),e.get('function')) for e in manifest}; selected={x.strip() for x in (ROOT/'porting/native_class_set.txt').read_text().splitlines() if x.strip() and not x.lstrip().startswith('#')}
 byname=collections.defaultdict(list)
 for va,n in names.items(): byname[n].append(va)
 headers=collections.defaultdict(list)
 for h in (ROOT/'porting/native/Source').rglob('*.h'): headers[h.stem].append(h)
 definitions=collections.defaultdict(set)
 for f in (ROOT/'porting/native/Source').rglob('*.cpp'):
  for c,n in re.findall(r'\b([A-Za-z_]\w*)::([A-Za-z_]\w*)\s*\(',f.read_text(errors='replace')): definitions[c].add(n)
 reject=collections.Counter(); rows=[]; total=0; index=load(REF/'native_class_index.json'); img=vt.Image(str(IMAGE)); cache={}; scratch=tempfile.TemporaryDirectory(prefix='struct-scalar-vt-')
 for cls,fs in sorted(native.items()):
  for name,fn in sorted(fs.items()):
   total+=1
   if not isinstance(fn,dict): continue
   ps=fn.get('params')
   if not isinstance(ps,list) or len(ps)!=1 or ps[0].get('name')!='ReturnValue' or ps[0].get('array_dim')!=1: continue
   try: pf=int(ps[0]['property_flags'],0); flags=int(fn['flags'],0); addr=int(fn['exec_addr'],0)
   except (KeyError,TypeError,ValueError): continue
   kind=ps[0].get('kind'); typ={'Bool':'bool'}.get(kind)
   if not typ or pf!=0x10000000000580 or ps[0].get('kind_raw')!='0x2c' or ps[0].get('offset') is not None or ps[0].get('element_size')!=1 or ps[0].get('size_of_outer')!=1 or ps[0].get('bitfield') is not True or ps[0].get('replicated') is not False or ps[0].get('rep_notify') is not None or ps[0].get('type_refs') or any(k in ps[0] for k in ('array_inner_of','map_key_of','map_value_of','set_element_of')) or flags&(~0xffffffcf|NET|0x08000000|0x20000000|0x2000) or not flags&0x400 or (flags&0xe0000) not in ACCESS: continue
   if (cls,name) in used or (cls,name+'_Implementation') in used or name in definitions[cls] or name+'_Implementation' in definitions[cls] or cls not in selected or not re.fullmatch(r'[A-Za-z_]\w*',name): continue
   vs=byname.get(cls,[])
   if len(vs)!=1: continue
   va=vs[0]; parent=parents.get(va,{}); block=blocks.get(va.lower()); hs=headers.get(cls,[])
   if not block or parent.get('name')!=cls or parent.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000 or len(hs)!=1: continue
   ch=hs[0].read_text()
   if re.search(r'\b'+re.escape(name)+r'(?:_Implementation)?\s*\(',ch): reject['generated_header_declaration']+=1; continue
   asm=decode(addr); target=None; code=''; form=None
   if asm.startswith(PREFIX):
    form='inline'; code=asm[len(PREFIX):]; suffix=r'; mov byte ptr \[r8\], al; ret'
   elif asm.startswith(WRAP):
    w=re.fullmatch(r'call (0x[0-9a-f]+); mov byte ptr \[rbx\], al; add rsp, 0x20; pop rbx; ret',asm[len(WRAP):])
    if w: target=int(w.group(1),16); code=decode(target); form='call'; suffix='; ret'
   if not form: reject['unmatched_wrapper']+=1; continue
   patterns=[(r'movzx eax, byte ptr \[rcx \+ (0x[0-9a-f]+)\]; shr (al|eax), (0x[0-9a-f]+|[0-9]+); and (al|eax), 1','shift'),(r'movzx eax, byte ptr \[rcx \+ (0x[0-9a-f]+)\]; and al, 1','bit'),(r'movzx eax, byte ptr \[rcx \+ (0x[0-9a-f]+)\]','byte')]
   m=None; mask=None; mode=None
   for pat,mode0 in patterns:
    z=re.fullmatch(pat+suffix,code)
    if not z: continue
    if mode0=='shift':
     shift=int(z.group(3),0)
     if z.group(2)!=z.group(4) or not 1<=shift<=7: continue
     mask=1<<shift
    else: mask=1 if mode0=='bit' else None
    m=z; mode=mode0; break
   if not m: reject['unmatched_complete_leaf']+=1; continue
   off=int(m.group(1),16); candidates=[]
   for p in block.get('properties',[]):
    if p.get('kind')!='Struct' or p.get('array_dim')!=1 or any(k in p for k in ('array_inner_of','map_key_of','map_value_of','set_element_of')) or p.get('kind_raw')!='0x19' or set(p.get('type_refs') or {})!={'struct'} or not isinstance(p.get('offset'),int): continue
    name1=p.get('name',''); fnref=(p.get('type_refs') or {}).get('struct','').lower(); rec=cmap.get(fnref,{}); s=structs.get(rec.get('block_va','').lower())
    if not s or rec.get('kind')!='struct' or not isinstance(s.get('size'),int) or not p['offset']<=off< p['offset']+s['size']: continue
    st='F'+s['name']; sh=headers.get(st,[])
    if len(sh)!=1 or sum(1 for z in structs.values() if z.get('name')==s['name'])!=1 or not re.fullmatch(r'[A-Za-z_]\w*',name1) or not re.search(r'\b'+re.escape(st)+r'\s+'+re.escape(name1)+r'\s*;',ch): continue
    stxt=sh[0].read_text(); classmod=hs[0].parent.parent.name; structmod=sh[0].parent.parent.name
    if parent.get('package')!='/Script/'+classmod or s.get('package')!='/Script/'+structmod: continue
    if classmod!=structmod:
     build=hs[0].parent.parent/(classmod+'.Build.cs')
     if not build.is_file() or not re.search(r'^\s*#include\s+"'+re.escape(sh[0].name)+r'"\s*$',ch,re.M): continue
     dep=re.search(r'\bPublicDependencyModuleNames\s*\.\s*AddRange\s*\(\s*new\[\]\s*\{([^}]*)\}\s*\)',build.read_text(),re.S)
     if not dep or '"'+structmod+'"' not in dep.group(1) or not re.search(r'\bstruct\s+'+re.escape(structmod.upper()+'_API')+r'\s+'+re.escape(st)+r'\b',stxt): continue
    # Read the original FStructParams property array and setter directly from the pinned image.
    sv=int(s['block_va'],0); pos=sv-BASE
    if pos<0 or pos+64>len(image): continue
    import struct as binary
    arr=binary.unpack_from('<Q',image,pos+48)[0]; count=binary.unpack_from('<i',image,pos+56)[0]
    if count!=len(s.get('properties',[])) or arr<BASE or arr-BASE+8*count>len(image): continue
    for q in s.get('properties',[]):
     name2=q.get('name',''); idx=q.get('index')
     if q.get('kind')!='Bool' or q.get('kind_raw')!='0x2c' or q.get('array_dim')!=1 or q.get('type_refs') or any(k in q for k in ('array_inner_of','map_key_of','map_value_of','set_element_of')) or not isinstance(idx,int) or not 0<=idx<count or not re.fullmatch(r'[A-Za-z_]\w*',name2): continue
     pp=binary.unpack_from('<Q',image,arr-BASE+8*idx)[0]; po=pp-BASE
     if po<0 or po+56>len(image): continue
     nva,rep,pflags,gflags,oflags,adim=binary.unpack_from('<QQQIII',image,po)
     no=nva-BASE
     if no<0 or no>=len(image): continue
     end=image.find(b'\x00',no,min(no+256,len(image)))
     if end<0 or image[no:end]!=name2.encode() or rep!=0 or pflags!=int(q['property_flags'],0) or gflags!=0x2c or oflags!=int(q['object_flags'],0) or adim!=1: continue
     elem,outer_size=binary.unpack_from('<IQ',image,po+36)
     if elem!=q['element_size'] or outer_size!=q['size_of_outer']: continue
     setter=binary.unpack_from('<Q',image,po+48)[0]; setter_code=decode(setter) if setter else ''
     sm=re.fullmatch(r'(or|mov) byte ptr \[rcx \+ (0x[0-9a-f]+)\], (0x[0-9a-f]+|[0-9]+); ret',setter_code)
     if not sm: continue
     inner=int(sm.group(2),0); setmask=int(sm.group(3),0)
     if p['offset']+inner!=off or q.get('element_size')!=1 or q.get('size_of_outer')!=s['size'] or q.get('bitfield') is not True: continue
     if mode=='byte':
      if sm.group(1)!='mov' or setmask!=1: continue
     elif sm.group(1)!='or' or setmask!=mask: continue
     if not re.search(r'\bbool\s+'+re.escape(name2)+r'\s*;',stxt): continue
     candidates.append((name1,name2,st,p['offset'],inner,hex(setter),setter_code,setmask))
   if len(candidates)!=1: reject['no_unique_generated_struct_member']+=1; continue
   a,b,st,outer,inner,setter,setter_code,setmask=candidates[0]; rows.append(dict(class_=cls,function=name,kind=kind,type=typ,exec_addr=hex(addr),target=hex(target) if target else None,form=form,flags=fn['flags'],access=ACCESS[flags&0xe0000],const=bool(flags&0x40000000),outer=a,member=b,struct=st,outer_offset=hex(outer),member_offset=hex(inner),setter=setter,setter_assembly=setter_code,mask=hex(setmask),assembly=asm,leaf=code))
 entries=[]
 for r in rows:
  inc=HERE/(r['exec_addr'][2:]+'_'+r['class_']+'_'+r['function']+'.inc'); body='// Exact original embedded struct Boolean leaf; see README.md.\n// CONFIDENCE: HIGH\nreturn '+r['outer']+'.'+r['member']+';\n'
  if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs')
  inc.write_text(body)
  entries.append(dict(class_=r['class_'],function=r['function'],kind='exec',signature=r['type']+' '+r['function']+'()'+(' const' if r['const'] else ''),signature_status='proposed',inc_path=str(inc.relative_to(ROOT)),confidence='high',address=r['exec_addr'],status='ported',reflection_flags=r['flags'],static=False,access=r['access'],notes='Complete original '+r['form']+' Boolean leaf reads reflected '+r['outer']+'.'+r['member']+'; see bulk_struct_bool_runtime/README.md.'))
 for e,r in zip(entries,rows):
  e['class']=e.pop('class_')
  if r['form']=='virtual': e['virtual']=True; e['native_body_address']=r['target']
 out=Path('/tmp/gameplay-bulk-struct-bool'); out.mkdir(exist_ok=True)
 (out/'report.json').write_text(json.dumps(dict(image_sha256=SHA,inventory_records=total,candidates=len(rows),rejections=reject,results=rows),indent=2)+'\n'); (out/'manifest-entries.json').write_text(json.dumps(entries,indent=2)+'\n')
 print(json.dumps(dict(inventory_records=total,candidates=len(rows),rejections=reject),indent=2))
if __name__=='__main__': main()
