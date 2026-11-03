#!/usr/bin/env python3
"""Recover exact FName field getters from complete native execs; emit proposals only."""
import argparse, collections, hashlib, json, re, sys
from pathlib import Path
sys.path.insert(0, '/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
ROOT=Path(__file__).resolve().parents[3]; REFL=ROOT/'porting/generated/reflection'; HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT/'porting'))
from gen_native_classes import Resolver, TypeTables, decode_params
IMAGE=Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
BASE=0x140000000; SHA='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
load=lambda p: json.loads(p.read_text())
PREFIX='mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
WRAP='push rbx; sub rsp, 0x20; mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; mov rbx, r8; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
NET=0x80000000|0x01000000|0x00200000|0x4000|0x1000|0x100|0x80|0x40
ACCESS={0x20000:'public',0x40000:'private',0x80000:'protected'}
WIDTH={'Name':8}
def main():
 ap=argparse.ArgumentParser(description=__doc__); ap.add_argument('--revalidate-own',action='store_true'); ap.add_argument('--output-dir',type=Path,default=Path('/tmp/gameplay-bulk-name')); args=ap.parse_args()
 image=IMAGE.read_bytes(); digest=hashlib.sha256(image).hexdigest()
 if digest!=SHA: raise SystemExit('image digest mismatch')
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
 native=load(REFL/'native_funcs.json'); names=load(REFL/'class_name_map.json'); parents=load(REFL/'class_parent_map.json'); reflection=load(REFL/'fortnite_game_reflection.json')
 blocks={c['block_va'].lower():c for c in reflection['classes_in_groups_with_fort_names']}; byname=collections.defaultdict(list)
 for va,name in names.items(): byname[name].append(va)
 enums=load(REFL/'enum_blocks.json')['enums']; enum_by_fn=collections.defaultdict(list)
 for e in enums:
  for fn in e.get('construct_fns',[]): enum_by_fn[fn.lower()].append(e)
 # Same construct-function-to-C++-name data used by the generator's Resolver.
 tables=TypeTables(str(REFL))
 type_map={fn: info.cpp_name for fn,info in tables.by_fn.items() if info.kind=='enum' and len(enum_by_fn.get(fn,[]))==1 and info.rec==enum_by_fn[fn][0]}
 resolver=Resolver(type_map)
 selected={x.strip() for x in (ROOT/'porting/native_class_set.txt').read_text().splitlines() if x.strip() and not x.lstrip().startswith('#')}
 own=str(HERE.relative_to(ROOT))+'/'
 ported={(e.get('class'),e.get('function')) for e in load(ROOT/'porting/behavior/MANIFEST.json') if e.get('status')=='ported' and not (args.revalidate_own and (e.get('inc_path') or '').startswith(own))}
 results=[]; rejects=collections.Counter(); total=0
 # A complete leaf read has no extension/truncation: typed byte, 16-bit, 32-bit, or 64-bit.
 read=[(r'movzx eax, byte ptr \[rcx \+ (0x[0-9a-f]+)\]','byte','al',1),(r'movzx eax, word ptr \[rcx \+ (0x[0-9a-f]+)\]','word','ax',2),(r'mov eax, dword ptr \[rcx \+ (0x[0-9a-f]+)\]','dword','eax',4),(r'mov rax, qword ptr \[rcx \+ (0x[0-9a-f]+)\]','qword','rax',8),(r'movsd xmm0, qword ptr \[rcx \+ (0x[0-9a-f]+)\]','qword','xmm0',8)]
 for cls,funcs in sorted(native.items()):
  for name,fn in sorted(funcs.items()):
   total+=1
   def reject(s): rejects[s]+=1
   if not isinstance(fn,dict): reject('invalid_record'); continue
   ps=fn.get('params')
   if not isinstance(ps,list) or len(ps)!=1 or ps[0].get('name')!='ReturnValue' or int(ps[0].get('property_flags','0'),0)&0x400==0: reject('not_zero_arg_getter'); continue
   p=ps[0]; kind=p.get('kind'); refs=p.get('type_refs') or {}
   if p.get('array_dim')!=1 or int(p.get('property_flags','0'),0)&0x08000000: reject('return_reference_or_array'); continue
   if kind!='Name': reject('unsupported_return_type'); continue
   if refs: reject('unexpected_return_refs'); continue
   try: flags=int(fn['flags'],0); addr=int(fn['exec_addr'],0)
   except (ValueError,TypeError,KeyError): reject('missing_flags_or_address'); continue
   if flags & (~0xffffffcf|NET|0x08000000|0x20000000|0x2000) or not flags&0x400 or (flags&0xe0000) not in ACCESS: reject('flags'); continue
   if (cls,name) in ported: reject('already_ported'); continue
   vs=byname.get(cls,[])
   if len(vs)!=1 or cls not in selected: reject('class_not_unique_or_selected'); continue
   va=vs[0]; parent=parents.get(va,{}); block=blocks.get(va.lower())
   if not block or parent.get('name')!=cls or parent.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000: reject('class_or_package'); continue
   if not re.fullmatch(r'[A-Za-z_]\w*',name): reject('identifier'); continue
   asm=decode(addr); target=None; matched=None; form=None
   if asm.startswith(PREFIX):
    inner=asm[len(PREFIX):]
    for pat,size,reg,width in read:
     store=(f'movsd {size} ptr [r8], {reg}' if reg=='xmm0' else f'mov {size} ptr [r8], {reg}')
     m=re.fullmatch(pat+'; '+re.escape(store)+r'; ret',inner)
     if m: matched=(int(m.group(1),0),width,reg); form='inline'; break
   elif asm.startswith(WRAP):
    m=re.fullmatch(r'call (0x[0-9a-f]+); (.+); add rsp, 0x20; pop rbx; ret',asm[len(WRAP):])
    if m:
     target=int(m.group(1),16); leaf=decode(target)
     for pat,size,reg,width in read:
      store=(f'movsd {size} ptr [rbx], {reg}' if reg=='xmm0' else f'mov {size} ptr [rbx], {reg}')
      q=re.fullmatch(pat+r'; ret',leaf)
      if q and m.group(2)==store: matched=(int(q.group(1),0),width,reg); form='wrapper'; break
   if not matched: reject('unmatched_complete_code'); continue
   offset,width,reg=matched
   if width!=8 or reg!='rax': reject('return_width_or_register'); continue
   matches=[]
   for field in block['properties']:
    if field.get('kind')!=kind or field.get('offset')!=offset or field.get('array_dim')!=1 or any(k in field for k in ('array_inner_of','map_key_of','map_value_of','set_element_of')): continue
    if (field.get('type_refs') or {})!=refs or not re.fullmatch(r'[A-Za-z_]\w*',field.get('name','')): continue
    matches.append(field)
   if len(matches)!=1: reject('field_absent_or_ambiguous'); continue
   field=matches[0]
   header=ROOT/'porting/native/Source'/parent['package'].removeprefix('/Script/')/'Generated'/f'{cls}.h'
   if not header.is_file() or len(re.findall(r'\bFName\s+'+re.escape(field['name'])+r'\s*;',header.read_text()))!=1: reject('current_header_field_missing_or_ambiguous'); continue
   ret,arguments=decode_params(ps,resolver)
   fieldtype=resolver.ctype(field,{})
   if arguments or 'TODO_' in ret or 'TODO_' in fieldtype: reject('unresolved_cpp_type'); continue
   if ret!='FName' or fieldtype!='FName': reject('cpp_type_mismatch'); continue
   prop=field['name']; inc=HERE/f'{addr:x}_{cls}_{name}.inc'
   body=f'// Original address: {hex(addr)}.\n// CONFIDENCE: HIGH\n// Mechanically matched complete FName field copy; see bulk_name_runtime/README.md.\nreturn {prop};\n'
   if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs: '+str(inc))
   if not inc.exists(): inc.write_text(body)
   entry={'class':cls,'function':name,'kind':'exec','signature':f"{ret} {name}()"+(' const' if flags&0x40000000 else ''),'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':False,'access':ACCESS[flags&0xe0000], 'notes':f'Original {form} {kind} getter at {hex(addr)}, reflected field {prop} offset {hex(offset)}; source recovery only.'}
   results.append({'entry':entry,'field':prop,'offset':offset,'width':width,'return_type':ret,'field_type':fieldtype,'enum_reference':refs.get('enum'),'assembly':asm,'target':hex(target) if target else None})
 args.output_dir.mkdir(parents=True,exist_ok=True)
 (args.output_dir/'manifest-entries.json').write_text(json.dumps([r['entry'] for r in results],indent=2)+'\n')
 report={'image_sha256':digest,'inventory_records':total,'candidates':len(results),'rejections':dict(sorted(rejects.items())),'results':results}
 (args.output_dir/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps({k:report[k] for k in ('inventory_records','candidates','rejections')},indent=2))
if __name__=='__main__': main()
