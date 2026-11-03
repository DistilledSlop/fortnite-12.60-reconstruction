#!/usr/bin/env python3
"""Propose two direct reflected-object stores after complete thunk/leaf checks."""
import collections, hashlib, json, re, sys
from pathlib import Path
HERE=Path(__file__).resolve().parent; ROOT=HERE.parents[2]; REF=ROOT/'porting/generated/reflection'; OUT=Path('/tmp/gameplay-bulk-two-scalar')
sys.path.insert(0,str(ROOT/'porting')); import gen_native_classes as gen
sys.path.insert(0,'/tmp/common-input-capstone'); from capstone import Cs,CS_ARCH_X86,CS_MODE_64
IMAGE=Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'); SHA='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'; BASE=0x140000000
load=lambda p:json.loads(p.read_text()); img=IMAGE.read_bytes()
if hashlib.sha256(img).hexdigest()!=SHA: raise SystemExit('image digest mismatch')
md=Cs(CS_ARCH_X86,CS_MODE_64)
def decode(a):
 if not BASE<=a<BASE+len(img)-512: return []
 r=[]
 for i in md.disasm(img[a-BASE:a-BASE+512],a):
  r.append(i)
  if i.mnemonic=='ret': return r
 return []
# Exact full thunk shape; branch and leaf targets checked separately below.
template='mov qword ptr [rsp + 8], rbx; mov qword ptr [rsp + 0x18], rsi; push rdi; sub rsp, 0x20; xor edi, edi; mov rsi, rcx; mov rbx, rdx; mov qword ptr [rsp + 0x48], rdi; mov rcx, rdx; cmp qword ptr [rdx + 0x20], rdi; je ALT1; mov rdx, qword ptr [rdx + 0x18]; lea r8, [rsp + 0x48]; call 0x142ed9cf0; jmp JOIN1; mov r8, qword ptr [rdx + 0x80]; mov rax, qword ptr [r8 + 0x20]; mov qword ptr [rdx + 0x80], rax; lea rdx, [rsp + 0x48]; call 0x142ed9d20; mov rcx, rbx; mov qword ptr [rsp + 0x38], rdi; cmp qword ptr [rbx + 0x20], rdi; je ALT2; mov rdx, qword ptr [rbx + 0x18]; lea r8, [rsp + 0x38]; call 0x142ed9cf0; jmp JOIN2; mov r8, qword ptr [rbx + 0x80]; lea rdx, [rsp + 0x38]; mov rax, qword ptr [r8 + 0x20]; mov qword ptr [rbx + 0x80], rax; call 0x142ed9d20; mov rax, qword ptr [rbx + 0x20]; mov rcx, rsi; mov r8, qword ptr [rsp + 0x38]; test rax, rax; mov rdx, qword ptr [rsp + 0x48]; setne dil; add rdi, rax; mov qword ptr [rbx + 0x20], rdi; call LEAF; mov rbx, qword ptr [rsp + 0x30]; mov rsi, qword ptr [rsp + 0x40]; add rsp, 0x20; pop rdi; ret'.split('; ')
names=load(REF/'class_name_map.json'); refs=load(REF/'class_ref_map.json')['resolved']; parents=load(REF/'class_parent_map.json'); blocks={x['block_va'].lower():x for x in load(REF/'fortnite_game_reflection.json')['classes_in_groups_with_fort_names']}; native=load(REF/'native_funcs.json'); byname=collections.defaultdict(list)
for v,n in names.items(): byname[n].append(v)
selected=set((ROOT/'porting/native_class_set.txt').read_text().splitlines()); done={(x.get('class'),x.get('function')) for x in load(ROOT/'porting/behavior/MANIFEST.json') if x.get('status')=='ported'}; headers=collections.defaultdict(list)
for p in (ROOT/'porting/native/Source').glob('*/Generated/*.h'): headers[p.stem].append(p)
maintained=set()
for p in (ROOT/'porting/native/Source').rglob('*.cpp'):
 if 'Generated' not in p.parts: maintained.update(re.findall(r'\b([A-Za-z_]\w*)::([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?\{',p.read_text(errors='replace')))
results=[]; rejects=collections.Counter(); ACCESS={0x20000:'public',0x40000:'private',0x80000:'protected'}; NET=0x80000000|0x01000000|0x00200000|0x4000|0x1000|0x100|0x80|0x40
for cls,funcs in sorted(native.items()):
 for name,fn in sorted(funcs.items()):
  if not isinstance(fn,dict) or not isinstance(fn.get('params'),list): continue
  pairs,notes=gen.split_properties(fn['params']); ps=sorted((p for p,ch in pairs),key=lambda p:p.get('offset') if isinstance(p.get('offset'),int) else -1)
  if notes or len(ps)!=2 or any(not isinstance(p.get('offset'),int) or p['offset']<0 for p in ps) or ps[0]['offset']==ps[1]['offset'] or any(ch for _,ch in pairs) or any(p.get('kind')!='Object' or p.get('array_dim')!=1 or int(p.get('property_flags','0'),0)&(0x400|0x100|0x08000000) or not re.fullmatch(r'[A-Za-z_]\w*',p.get('name','')) for p in ps): continue
  try: flags=int(fn['flags'],0); addr=int(fn['exec_addr'],0)
  except (ValueError,TypeError,KeyError): continue
  if flags & (~0xffffffcf|NET|0x08000000|0x20000000|0x2000|0x40000000) or not flags&0x400 or (flags&0xe0000) not in ACCESS or (cls,name) in done or (cls,name) in maintained or cls not in selected or len(byname[cls])!=1 or len(headers[cls])!=1 or not re.fullmatch(r'[A-Za-z_]\w*',name): continue
  va=byname[cls][0]; par=parents.get(va,{}); block=blocks.get(va.lower())
  if not block or par.get('name')!=cls or par.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000: continue
  ins=decode(addr)
  if len(ins)!=len(template): continue
  targets={'ALT1':15,'JOIN1':20,'ALT2':28,'JOIN2':33}; leaf=None
  for i,w in zip(ins,template):
   actual=(i.mnemonic+' '+i.op_str).rstrip()
   for key,idx in targets.items(): w=w.replace(key,hex(ins[idx].address))
   if w=='call LEAF':
    if i.mnemonic!='call' or not re.fullmatch(r'0x[0-9a-f]+',i.op_str): break
    leaf=int(i.op_str,0); w=actual
   if actual!=w: break
  else:
   code='; '.join((i.mnemonic+' '+i.op_str).rstrip() for i in decode(leaf)); m=re.fullmatch(r'mov qword ptr \[rcx \+ (0x[0-9a-f]+)\], rdx; mov qword ptr \[rcx \+ (0x[0-9a-f]+)\], r8; ret',code)
   if not m: continue
   resolver=gen.Resolver({k:names.get(v.lower(),'') for k,v in refs.items()}); ret,args=gen.decode_params(fn['params'],resolver)
   if resolver.todos or ret!='void' or len(args)!=2: rejects['generator_type']+=1; continue
   fields=[]
   for p,off in zip(ps,map(lambda x:int(x,0),m.groups())):
    matches=[]
    for ancestor in par.get('chain',[]):
     av=byname.get(ancestor,[])
     if len(av)!=1 or len(headers[ancestor])!=1: continue
     b=blocks.get(av[0].lower())
     if not b: continue
     fp,notes=gen.split_properties(b['properties'])
     if notes: continue
     matches.extend((ancestor,x) for x,ch in fp if not ch and x.get('offset')==off and x.get('array_dim')==1)
    if len(matches)!=1: break
    owner,field=matches[0]
    if field.get('kind')!='Object' or field.get('type_refs')!=p.get('type_refs'): break
    prop=field.get('name',''); ref=(p.get('type_refs') or {}).get('class','').lower(); tv=refs.get(ref,'').lower(); typ=names.get(tv); tb=blocks.get(tv); hdr=headers[owner][0].read_text()
    if not typ or len(byname[typ])!=1 or not tb or int(tb['class_flags'],0)&0x4000 or not re.fullmatch(r'[A-Za-z_]\w*',prop): break
    decl=re.search(r'^\s*'+re.escape(typ)+r'\s*\*\s*'+re.escape(prop)+r'\s*;',hdr,re.M)
    if not decl: break
    labels=list(re.finditer(r'^\s*(public|protected|private)\s*:',hdr[:decl.start()],re.M))
    if owner!=cls and (not labels or labels[-1].group(1)=='private'): break
    fields.append((owner,prop,off,typ))
   if len(fields)!=2: rejects['field']+=1; continue
   sig=f'void {name}('+', '.join(args)+')'; header=headers[cls][0].read_text(); cpp=headers[cls][0].with_suffix('.cpp')
   if (cpp.exists() and re.search(r'\b'+re.escape(cls)+r'::'+re.escape(name)+r'\s*\(',cpp.read_text())) or re.search(r'\b'+re.escape(name)+r'\s*\([^;{}]*\)\s*(?:const\s*)?\{',header): continue
   declarations=re.findall(r'^\s*[^\n;]*\b'+re.escape(name)+r'\s*\([^;{}\n]*\)[^;{}\n]*;',header,re.M)
   if declarations and (len(declarations)!=1 or not re.fullmatch(r'\s*'+re.escape(sig)+r'\s*;',declarations[0])): continue
   inc=HERE/f'{addr:x}_{cls}_{name}.inc'; body=f'// Original exec {hex(addr)}, complete two-store leaf {hex(leaf)}.\n// CONFIDENCE: HIGH\n'+''.join(f'this->{field[1]} = {p["name"]};\n' for field,p in zip(fields,ps))
   if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs')
   inc.write_text(body); entry={'class':cls,'function':name,'kind':'exec','signature':sig,'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':False,'access':ACCESS[flags&0xe0000],'notes':f'Complete two-object FFrame thunk and two reflected pointer stores at {hex(leaf)}; source recovery only.'}; results.append({'entry':entry,'fields':fields,'leaf':code,'exec':[(i.mnemonic+' '+i.op_str).rstrip() for i in ins]})
OUT.mkdir(exist_ok=True); (OUT/'manifest-entries.json').write_text(json.dumps([x['entry'] for x in results],indent=2)+'\n'); (OUT/'report.json').write_text(json.dumps({'image_sha256':SHA,'rejects':rejects,'results':results},indent=2)+'\n'); print(json.dumps({'candidates':len(results),'rejects':rejects,'functions':[(x['entry']['class'],x['entry']['function']) for x in results]},indent=2))
