#!/usr/bin/env python3
"""Recover exact constant return getters from complete native execs; emit proposals only."""
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
WIDTH={'Byte':1,'Int8':1,'Int16':2,'UInt16':2,'UInt32':4,'Int64':8,'UInt64':8,'Double':8}
def main():
 ap=argparse.ArgumentParser(description=__doc__); ap.add_argument('--revalidate-own',action='store_true'); ap.add_argument('--output-dir',type=Path,default=Path('/tmp/gameplay-bulk-constant-return')); args=ap.parse_args()
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
 for cls,funcs in sorted(native.items()):
  for name,fn in sorted(funcs.items()):
   total+=1
   def reject(s): rejects[s]+=1
   if not isinstance(fn,dict): reject('invalid_record'); continue
   ps=fn.get('params')
   if not isinstance(ps,list) or len(ps)!=1 or ps[0].get('name')!='ReturnValue' or int(ps[0].get('property_flags','0'),0)&0x400==0: reject('not_zero_arg_getter'); continue
   p=ps[0]; kind=p.get('kind'); refs=p.get('type_refs') or {}
   if p.get('array_dim')!=1 or int(p.get('property_flags','0'),0)&0x08000000: reject('return_reference_or_array'); continue
   if kind not in ('Byte','Int64','UInt64','Double','Float'): reject('unsupported_return_type'); continue
   if kind=='Byte' and refs and (set(refs)!={'enum'} or len(enum_by_fn.get(refs['enum'].lower(),[]))!=1 or not re.fullmatch(r'[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*',type_map.get(refs['enum'].lower(),'') or '')): reject('unresolved_enum'); continue
   if kind!='Byte' and refs: reject('unexpected_return_refs'); continue
   try: flags=int(fn['flags'],0); addr=int(fn['exec_addr'],0)
   except (ValueError,TypeError,KeyError): reject('missing_flags_or_address'); continue
   if flags & (~0xffffffcf|NET|0x08000000|0x20000000) or not flags&0x400 or (flags&0xe0000) not in ACCESS: reject('flags'); continue
   if (cls,name) in ported: reject('already_ported'); continue
   vs=byname.get(cls,[])
   if len(vs)!=1 or cls not in selected: reject('class_not_unique_or_selected'); continue
   va=vs[0]; parent=parents.get(va,{}); block=blocks.get(va.lower())
   if not block or parent.get('name')!=cls or parent.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000: reject('class_or_package'); continue
   if not re.fullmatch(r'[A-Za-z_]\w*',name): reject('identifier'); continue
   asm=decode(addr); target=None; raw=None; width=None; form=None
   prefixes=[PREFIX, 'mov rax, qword ptr [rdx + 0x20]; xor ecx, ecx; test rax, rax; setne cl; add rcx, rax; mov qword ptr [rdx + 0x20], rcx; ']
   for prefix in prefixes:
    if asm.startswith(prefix):
     m=re.fullmatch(r'mov (byte|dword|qword) ptr \[r8\], (0x[0-9a-f]+|[0-9]+); ret',asm[len(prefix):])
     if m: width={'byte':1,'dword':4,'qword':8}[m.group(1)]; raw=int(m.group(2),0); form='inline'
   if asm.startswith(WRAP):
    m=re.fullmatch(r'call (0x[0-9a-f]+); (mov (?:byte|dword|qword) ptr \[rbx\], (?:al|eax|rax)|movs[sd] (?:dword|qword) ptr \[rbx\], xmm0); add rsp, 0x20; pop rbx; ret',asm[len(WRAP):])
    if m:
     target=int(m.group(1),16); leaf=decode(target); store=m.group(2)
     q=re.fullmatch(r'(?:xor (al|eax|rax), \1|mov (al|eax|rax), (0x[0-9a-f]+|[0-9]+)); ret',leaf)
     if q:
      reg=q.group(1) or q.group(2); raw=int(q.group(3),0) if q.group(3) else 0
      widths={'mov byte ptr [rbx], al':1,'mov dword ptr [rbx], eax':4,'mov qword ptr [rbx], rax':8}
      width=widths.get(store)
      if width and reg not in ({1:('al','eax','rax'),4:('eax','rax'),8:('rax',)}[width]): width=None
     elif leaf in ('xorps xmm0, xmm0; ret','xorpd xmm0, xmm0; ret'):
      width={'movss dword ptr [rbx], xmm0':4,'movsd qword ptr [rbx], xmm0':8}.get(store); raw=0
     form='wrapper'
   wanted={'Byte':1,'Float':4,'Double':8,'Int64':8,'UInt64':8}[kind]
   if width!=wanted or raw is None or not 0<=raw<1<<(8*width): reject('unmatched_complete_code'); continue
   ret,arguments=decode_params(ps,resolver)
   if arguments or 'TODO_' in ret: reject('unresolved_cpp_type'); continue
   if kind=='Byte': value=f'static_cast<{ret}>({raw})' if refs else str(raw)
   elif kind in ('Int64','UInt64'):
    n=raw if kind=='UInt64' or raw<2**63 else raw-2**64
    value=('(-9223372036854775807LL - 1)' if n==-2**63 else str(n)+('ULL' if kind=='UInt64' else 'LL'))
   else:
    import struct, math
    f=struct.unpack('<f' if width==4 else '<d',raw.to_bytes(width,'little'))[0]
    if not math.isfinite(f) or (f==0 and raw!=0): reject('nonfinite_or_negative_zero'); continue
    value=format(f,'.9g' if width==4 else '.17g')
    if '.' not in value and 'e' not in value: value+='.0'
    if kind=='Float': value+='f'
   inc=HERE/f'{addr:x}_{cls}_{name}.inc'
   body=f'// Original address: {hex(addr)}.\n// CONFIDENCE: HIGH\n// Complete constant return; see bulk_constant_return_runtime/README.md.\nreturn {value};\n'
   if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs: '+str(inc))
   if not inc.exists(): inc.write_text(body)
   entry={'class':cls,'function':name,'kind':'exec','signature':f"{ret} {name}()"+(' const' if flags&0x40000000 else ''),'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':bool(flags&0x2000),'access':ACCESS[flags&0xe0000], 'notes':f'Original {form} constant {kind} getter at {hex(addr)}; source recovery only.'}
   results.append({'entry':entry,'constant_bits':raw,'return_type':ret,'assembly':asm,'target':hex(target) if target else None,'leaf':decode(target) if target else None})
 args.output_dir.mkdir(parents=True,exist_ok=True)
 (args.output_dir/'manifest-entries.json').write_text(json.dumps([r['entry'] for r in results],indent=2)+'\n')
 report={'image_sha256':digest,'inventory_records':total,'candidates':len(results),'rejections':dict(sorted(rejects.items())),'results':results}
 (args.output_dir/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps({k:report[k] for k in ('inventory_records','candidates','rejections')},indent=2))
if __name__=='__main__': main()
