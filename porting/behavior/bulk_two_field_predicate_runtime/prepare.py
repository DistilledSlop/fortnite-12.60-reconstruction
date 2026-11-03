#!/usr/bin/env python3
"""Recover complete two-int32 comparison leaves behind standard bool wrappers."""
import argparse, collections, hashlib, importlib.util, json, re, sys
from pathlib import Path
sys.path.insert(0, '/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
ROOT = Path(__file__).resolve().parents[3]
REFL = ROOT / 'porting/generated/reflection'
HERE = Path(__file__).resolve().parent
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
SHA = '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
BASE = 0x140000000
WRAP = 'push rbx; sub rsp, 0x20; mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; mov rbx, r8; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
NET = 0x80000000 | 0x01000000 | 0x00200000 | 0x4000 | 0x1000 | 0x100 | 0x80 | 0x40
ACCESS = {0x20000:'public', 0x40000:'private', 0x80000:'protected'}
load = lambda p: json.loads(p.read_text())

def main():
 ap=argparse.ArgumentParser(description=__doc__); ap.add_argument('--output-dir',type=Path,default=Path('/tmp/gameplay-bulk-two-field-predicate')); ap.add_argument('--revalidate-own',action='store_true'); args=ap.parse_args()
 image=IMAGE.read_bytes(); digest=hashlib.sha256(image).hexdigest()
 if digest != SHA: raise SystemExit('image digest mismatch')
 md=Cs(CS_ARCH_X86, CS_MODE_64)
 def decode(a):
  pos=a-BASE
  if pos<0 or pos+128>len(image): return ''
  out=[]
  for ins in md.disasm(image[pos:pos+128],a):
   out.append((ins.mnemonic+' '+ins.op_str).rstrip())
   if ins.mnemonic=='ret': return '; '.join(out)
   if ins.mnemonic.startswith('j'): return ''
  return ''
 native=load(REFL/'native_funcs.json'); names=load(REFL/'class_name_map.json'); parents=load(REFL/'class_parent_map.json')
 blocks={c['block_va'].lower():c for c in load(REFL/'fortnite_game_reflection.json')['classes_in_groups_with_fort_names']}
 byname=collections.defaultdict(list)
 for va,name in names.items(): byname[name].append(va)
 selected={x.strip() for x in (ROOT/'porting/native_class_set.txt').read_text().splitlines() if x.strip() and not x.lstrip().startswith('#')}
 own=str(HERE.relative_to(ROOT))+'/'
 canonical={(e.get('class'),e.get('function')) for e in load(ROOT/'porting/behavior/MANIFEST.json') if not (args.revalidate_own and (e.get('inc_path') or '').startswith(own))}
 headers=collections.defaultdict(list)
 for module in ('FortniteGame','FortniteUI'):
  for p in (ROOT/'porting/native/Source'/module/'Generated').glob('*.h'): headers[p.stem].append(p)
 spec=importlib.util.spec_from_file_location('gen_native_classes', ROOT/'porting/gen_native_classes.py'); generator=importlib.util.module_from_spec(spec); sys.modules[spec.name]=generator; spec.loader.exec_module(generator)
 rejected=collections.Counter(); results=[]; total=0
 for cls,funcs in sorted(native.items()):
  for name,fn in sorted(funcs.items()):
   total+=1
   if not isinstance(fn,dict): rejected['invalid_record']+=1; continue
   ps=fn.get('params')
   if not isinstance(ps,list) or len(ps)!=1 or ps[0].get('name')!='ReturnValue' or ps[0].get('kind')!='Bool': rejected['not_parameterless_bool']+=1; continue
   p=ps[0]
   try: pf=int(p['property_flags'],0); flags=int(fn['flags'],0); addr=int(fn['exec_addr'],0)
   except (KeyError,TypeError,ValueError): rejected['missing_metadata']+=1; continue
   if pf != 0x10000000000580 or p.get('kind_raw') != '0x2c' or p.get('array_dim')!=1 or p.get('element_size')!=1 or p.get('size_of_outer')!=1 or p.get('type_refs') or p.get('bitfield') is not True: rejected['return_shape']+=1; continue
   if flags & (~0xffffffcf | NET | 0x08000000 | 0x20000000 | 0x2000) or not flags & 0x400 or (flags & 0xe0000) not in ACCESS: rejected['flags']+=1; continue
   if (cls,name) in canonical: rejected['canonical_pair']+=1; continue
   vs=byname.get(cls,[])
   if len(vs)!=1 or cls not in selected: rejected['class_not_unique_or_selected']+=1; continue
   va=vs[0]; parent=parents.get(va,{}); block=blocks.get(va.lower())
   if not block or parent.get('name')!=cls or parent.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000: rejected['class_or_package']+=1; continue
   if not re.fullmatch(r'[A-Za-z_]\w*',name): rejected['identifier']+=1; continue
   asm=decode(addr); m=re.fullmatch(re.escape(WRAP)+r'call (0x[0-9a-f]+); mov byte ptr \[rbx\], al; add rsp, 0x20; pop rbx; ret',asm)
   if not m: rejected['wrapper']+=1; continue
   target=int(m[1],0); leaf=decode(target)
   q=re.fullmatch(r'mov eax, dword ptr \[rcx \+ (0x[0-9a-f]+)\]; cmp dword ptr \[rcx \+ (0x[0-9a-f]+)\], eax; (sete|setne|setl|setle|setg|setge) al; ret',leaf)
   if not q: rejected['leaf']+=1; continue
   offsets=[int(q[2],0),int(q[1],0)]
   if offsets[0]==offsets[1]: rejected['same_offset']+=1; continue
   fields=[]
   for offset in offsets:
    matches=[(x,children) for x,children in generator.split_properties(block['properties'])[0] if x.get('offset')==offset]
    if len(matches)!=1: break
    x,children=matches[0]
    if children or x.get('kind')!='Int' or x.get('kind_raw')!='0x3' or x.get('array_dim')!=1 or x.get('type_refs') or not re.fullmatch(r'[A-Za-z_]\w*',x.get('name','')): break
    matches=[x]
    fields.append(matches[0])
   if len(fields)!=2: rejected['field_absent_or_ambiguous']+=1; continue
   if len(headers[cls])!=1: rejected['header']+=1; continue
   header=headers[cls][0].read_text(); good=True
   for field in fields:
    resolver=generator.Resolver({}); decl=list(re.finditer(r'^\s*int32\s+'+re.escape(field['name'])+r'\s*;',header,re.M))
    if resolver.ctype(field,{})!='int32' or resolver.todos or len(decl)!=1: good=False; break
    labels=list(re.finditer(r'^\s*(public|protected|private):',header[:decl[0].start()],re.M))
    if not labels: good=False; break
   if not good: rejected['field_header_or_access']+=1; continue
   sigs=re.findall(r'^\s*bool\s+'+re.escape(name)+r'\(\)( const)?\s*;',header,re.M)
   if sigs and (len(sigs)!=1 or bool(sigs[0])!=bool(flags&0x40000000)): rejected['signature']+=1; continue
   # Exclude existing maintained or generated class-qualified definitions.
   pat=re.compile(r'\b'+re.escape(cls)+r'::'+re.escape(name)+r'\s*\(')
   if any(pat.search(src.read_text(errors='replace')) for src in (ROOT/'porting/native/Source').rglob('*.cpp')): rejected['existing_definition']+=1; continue
   op={'sete':'==','setne':'!=','setl':'<','setle':'<=','setg':'>','setge':'>='}[q[3]]
   inc=HERE/f'{addr:x}_{cls}_{name}.inc'; expression=f"{fields[0]['name']} {op} {fields[1]['name']}"
   body=f'// Original address: {hex(addr)}.\n// CONFIDENCE: HIGH\n// Complete signed int32 field comparison; see bulk_two_field_predicate_runtime/README.md.\nreturn {expression};\n'
   if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs: '+str(inc))
   inc.write_text(body)
   entry={'class':cls,'function':name,'kind':'exec','signature':f'bool {name}()'+(' const' if flags&0x40000000 else ''),'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':False,'access':ACCESS[flags&0xe0000],'notes':f'Original wrapper {hex(addr)} and complete signed int32 field comparison leaf {hex(target)}; {fields[0]["name"]} {hex(offsets[0])}, {fields[1]["name"]} {hex(offsets[1])}; source recovery only.'}
   results.append({'entry':entry,'assembly':asm,'target':hex(target),'leaf':leaf,'fields':[{'name':f['name'],'offset':hex(o)} for f,o in zip(fields,offsets)]})
 args.output_dir.mkdir(parents=True,exist_ok=True)
 (args.output_dir/'manifest-entries.json').write_text(json.dumps([x['entry'] for x in results],indent=2)+'\n')
 report={'image_sha256':digest,'inventory_records':total,'candidates':len(results),'rejections':dict(sorted(rejected.items())),'results':results}; (args.output_dir/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps({k:report[k] for k in ('inventory_records','candidates','rejections')},indent=2))
if __name__=='__main__': main()
