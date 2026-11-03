#!/usr/bin/env python3
"""Fail-closed direct reflected embedded struct scalar leaf recovery."""
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
   kind=ps[0].get('kind'); typ={'Int':'int32','Float':'float'}.get(kind)
   if not typ or pf!=0x10000000000580 or ps[0].get('kind_raw')!={'Int':'0x3','Float':'0xa'}[kind] or ps[0].get('offset')!=0 or ps[0].get('replicated') is not False or ps[0].get('rep_notify') is not None or ps[0].get('type_refs') or any(k in ps[0] for k in ('array_inner_of','map_key_of','map_value_of','set_element_of')) or flags&(~0xffffffcf|NET|0x08000000|0x20000000|0x2000) or not flags&0x400 or (flags&0xe0000) not in ACCESS: continue
   if (cls,name) in used or (cls,name+'_Implementation') in used or name in definitions[cls] or name+'_Implementation' in definitions[cls] or cls not in selected or not re.fullmatch(r'[A-Za-z_]\w*',name): continue
   vs=byname.get(cls,[])
   if len(vs)!=1: continue
   va=vs[0]; parent=parents.get(va,{}); block=blocks.get(va.lower()); hs=headers.get(cls,[])
   if not block or parent.get('name')!=cls or parent.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000 or len(hs)!=1: continue
   ch=hs[0].read_text()
   if re.search(r'\b'+re.escape(name)+r'(?:_Implementation)?\s*\(',ch): reject['generated_header_declaration']+=1; continue
   asm=decode(addr); target=None; code=''; form=None; vproof=None
   # Virtual exec ends in an indirect call; decode the entire wrapper through ret.
   vm=re.fullmatch(re.escape(WRAP)+r'mov rax, qword ptr \[rcx\]; call qword ptr \[rax \+ (0x[0-9a-f]+|[0-9]+)\]; (mov dword ptr \[rbx\], eax|movss dword ptr \[rbx\], xmm0); add rsp, 0x20; pop rbx; ret',asm)
   if vm and ((kind=='Int' and vm.group(2)=='mov dword ptr [rbx], eax') or (kind=='Float' and vm.group(2)=='movss dword ptr [rbx], xmm0')):
    if cls not in cache:
     gs=index.get(cls,{}).get('gpsc_targets') or []; result=None
     if len(gs)==1:
      gpsc=int(gs[0],0); ctor=vt.ctor_of(img,gpsc,scratch.name); thunk=vt.LAST_THUNK[0]
      if ctor:
       stores=vt.vptr_candidates(img,ctor,scratch.name); extra=vt.thunk_store_candidates(img,thunk,scratch.name) if thunk else []
       if len(stores)==1 and not extra and virt.proven_this_store(img,ctor,stores[0],scratch.name): result=(gpsc,thunk,ctor,stores[0],len(vt.table_entries(img,stores[0])))
     cache[cls]=result
    v=cache[cls]; slot=int(vm.group(1),0)
    if v and slot%8==0 and slot//8<v[4]:
     gpsc,thunk,ctor,table,count=v; target=img.u64(table+slot); code=decode(target); form='virtual'; vproof=dict(gpsc=hex(gpsc),thunk=hex(thunk) if thunk else None,ctor=hex(ctor),vtable=hex(table),slots=count,slot=hex(slot))
    else: reject['virtual_vtable_unproven']+=1
   if form=='virtual': pass
   elif asm.startswith(PREFIX): code=asm[len(PREFIX):]; form='inline'
   elif asm.startswith(WRAP):
    m=re.fullmatch(r'call (0x[0-9a-f]+); (mov dword ptr \[rbx\], eax|movss dword ptr \[rbx\], xmm0); add rsp, 0x20; pop rbx; ret',asm[len(WRAP):])
    if m and ((kind=='Int' and m.group(2)=='mov dword ptr [rbx], eax') or (kind=='Float' and m.group(2)=='movss dword ptr [rbx], xmm0')): target=int(m.group(1),16); code=decode(target); form='call'
   pats={'inline':r'mov eax, dword ptr \[rcx \+ (0x[0-9a-f]+)\]; mov dword ptr \[r8\], eax; ret','call':r'mov '+('eax' if kind=='Int' else 'ss xmm0')+r', dword ptr \[rcx \+ (0x[0-9a-f]+)\]; ret'}
   # movss is one mnemonic, not "mov ss".
   if form=='call' and kind=='Float': pats['call']=r'movss xmm0, dword ptr \[rcx \+ (0x[0-9a-f]+)\]; ret'
   m=re.fullmatch(pats.get('call' if form=='virtual' else form,'(?!)'),code)
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
    for q in s.get('properties',[]):
     name2=q.get('name','')
     if q.get('kind')!=kind or q.get('array_dim')!=1 or any(k in q for k in ('array_inner_of','map_key_of','map_value_of','set_element_of')) or q.get('type_refs') or q.get('kind_raw')!={'Int':'0x3','Float':'0xa'}[kind] or q.get('offset')!=off-p['offset'] or not re.fullmatch(r'[A-Za-z_]\w*',name2): continue
     if not re.search(r'\b'+typ+r'\s+'+re.escape(name2)+r'\s*;',stxt): continue
     candidates.append((name1,name2,st,p['offset'],q['offset']))
   if len(candidates)!=1: reject['no_unique_generated_struct_member']+=1; continue
   a,b,st,outer,inner=candidates[0]; rows.append(dict(class_=cls,function=name,kind=kind,type=typ,exec_addr=hex(addr),target=hex(target) if target else None,form=form,flags=fn['flags'],access=ACCESS[flags&0xe0000],const=bool(flags&0x40000000),outer=a,member=b,struct=st,outer_offset=hex(outer),member_offset=hex(inner),assembly=asm,leaf=code,vtable_proof=vproof))
 entries=[]
 for r in rows:
  inc=HERE/(r['exec_addr'][2:]+'_'+r['class_']+'_'+r['function']+'.inc'); body='// Exact original embedded struct scalar leaf; see README.md.\n// CONFIDENCE: HIGH\nreturn '+r['outer']+'.'+r['member']+';\n'
  if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs')
  inc.write_text(body)
  entries.append(dict(class_=r['class_'],function=r['function'],kind='exec',signature=r['type']+' '+r['function']+'()'+(' const' if r['const'] else ''),signature_status='proposed',inc_path=str(inc.relative_to(ROOT)),confidence='high',address=r['exec_addr'],status='ported',reflection_flags=r['flags'],static=False,access=r['access'],notes='Complete original '+r['form']+' scalar leaf reads reflected '+r['outer']+'.'+r['member']+'; see bulk_struct_scalar_runtime/README.md.'))
 for e,r in zip(entries,rows):
  e['class']=e.pop('class_')
  if r['form']=='virtual': e['virtual']=True; e['native_body_address']=r['target']
 out=Path('/tmp/gameplay-bulk-struct-scalar'); out.mkdir(exist_ok=True)
 (out/'report.json').write_text(json.dumps(dict(image_sha256=SHA,inventory_records=total,candidates=len(rows),rejections=reject,results=rows),indent=2)+'\n'); (out/'manifest-entries.json').write_text(json.dumps(entries,indent=2)+'\n')
 print(json.dumps(dict(inventory_records=total,candidates=len(rows),rejections=reject),indent=2))
if __name__=='__main__': main()
