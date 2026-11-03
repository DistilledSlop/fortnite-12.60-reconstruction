#!/usr/bin/env python3
"""Conservative whole-inventory getter recovery; writes only local proposals and scratch report."""
import argparse, collections, hashlib, json, math, re, struct, sys
from pathlib import Path
sys.path.insert(0, '/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
ROOT=Path(__file__).resolve().parents[3]; REFL=ROOT/'porting/generated/reflection'
HERE=Path(__file__).resolve().parent
IMAGE=Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
BASE=0x140000000; SHA='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
load=lambda p: json.loads(p.read_text())
PREFIX='mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
WRAP='push rbx; sub rsp, 0x20; mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; mov rbx, r8; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
NET=0x80000000|0x01000000|0x00200000|0x4000|0x1000|0x100|0x80|0x40
ACCESS={0x20000:'public',0x40000:'private',0x80000:'protected'}
def main():
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('--revalidate-own',action='store_true',help='rescan ported entries belonging to bulk_scalar_expansion_runtime')
 parser.add_argument('--output-dir',type=Path,default=Path('/tmp/gameplay-bulk-scalar-expansion'),help='directory for the proposal manifest and report')
 args=parser.parse_args()
 image=IMAGE.read_bytes(); digest=hashlib.sha256(image).hexdigest()
 if digest!=SHA: raise SystemExit('image digest mismatch')
 md=Cs(CS_ARCH_X86,CS_MODE_64)
 def decode(addr):
  pos=addr-BASE
  if pos<0 or pos+128>len(image): return ''
  ins=[]
  for i in md.disasm(image[pos:pos+128],addr):
   ins.append(i.mnemonic+' '+i.op_str)
   if i.mnemonic=='ret': return '; '.join(x.rstrip() for x in ins)
   if i.mnemonic.startswith('j'): return ''
  return ''
 native=load(REFL/'native_funcs.json'); names=load(REFL/'class_name_map.json'); parents=load(REFL/'class_parent_map.json')
 reflection=load(REFL/'fortnite_game_reflection.json')
 blocks={c['block_va'].lower():c for c in reflection['classes_in_groups_with_fort_names']}
 byname=collections.defaultdict(list)
 for va,name in names.items(): byname[name].append(va)
 selected={x.strip() for x in (ROOT/'porting/native_class_set.txt').read_text().splitlines() if x.strip() and not x.lstrip().startswith('#')}
 own_path=str(HERE.relative_to(ROOT))+'/'
 ported={(e.get('class'),e.get('function')) for e in load(ROOT/'porting/behavior/MANIFEST.json') if e.get('status')=='ported' and not (args.revalidate_own and (e.get('inc_path') or '').startswith(own_path))}
 results=[]; rejects=collections.Counter(); total=0
 for cls,funcs in sorted(native.items()):
  for name,fn in sorted(funcs.items()):
   total+=1
   def reject(reason): rejects[reason]+=1
   if not isinstance(fn,dict): reject('invalid_record'); continue
   ps=fn.get('params')
   if not isinstance(ps,list) or len(ps)!=1 or ps[0].get('name')!='ReturnValue' or int(ps[0].get('property_flags','0'),0)&0x400==0: reject('not_zero_arg_getter'); continue
   if ps[0].get('array_dim') != 1 or int(ps[0].get('property_flags','0'),0) & 0x08000000:
    reject('return_reference_or_array'); continue
   kind=ps[0].get('kind')
   if kind not in ('Int','Float','Byte','Bool'): reject('unsupported_return_type'); continue
   if kind=='Byte' and ps[0].get('type_refs'): reject('typed_byte_unsupported'); continue
   try: flags=int(fn['flags'],0); addr=int(fn['exec_addr'],0)
   except (ValueError,TypeError,KeyError): reject('missing_flags_or_address'); continue
   if flags & (~0xffffffcf|NET|0x08000000|0x20000000|0x2000) or not flags&0x400 or (flags&0xe0000) not in ACCESS: reject('flags'); continue
   if (cls,name) in ported: reject('already_ported'); continue
   vs=byname.get(cls,[])
   if len(vs)!=1 or cls not in selected: reject('class_not_unique_or_selected'); continue
   va=vs[0]; parent=parents.get(va,{}); block=blocks.get(va.lower())
   if not block or parent.get('name')!=cls or parent.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000: reject('class_or_package'); continue
   if not re.fullmatch(r'[A-Za-z_]\w*',name): reject('identifier'); continue
   # Exact exec wrapper and exact leaf: movss xmm0,[rip+disp32]; ret.
   asm=decode(addr); target=None
   if kind != 'Float' or not asm.startswith(WRAP): reject('not_float_wrapper'); continue
   m=re.fullmatch(r'call (0x[0-9a-f]+); movss dword ptr \[rbx\], xmm0; add rsp, 0x20; pop rbx; ret',asm[len(WRAP):])
   if not m: reject('unmatched_wrapper'); continue
   target=int(m.group(1),16); pos=target-BASE
   if pos<0 or pos+9>len(image) or image[pos:pos+4]!=bytes.fromhex('f30f1005') or image[pos+8]!=0xc3:
    reject('unmatched_leaf'); continue
   literal=target+8+struct.unpack_from('<i',image,pos+4)[0]; off=literal-BASE
   # The image is memory mapped; only the pinned read-only .rdata section is accepted.
   if not (0x58fc000 <= off and off+4 <= 0x58fc000+0x1f15c00): reject('literal_not_rdata'); continue
   raw=struct.unpack_from('<I',image,off)[0]
   # C++ decimal literals preserve finite float bits, except negative zero.
   if raw==0x80000000: reject('negative_zero'); continue
   typ='constant_dword'; offset=raw; mask=None; form='read_only_literal_wrapper'
   if typ.startswith('constant_'):
    width=typ.split('_',1)[1]; raw=offset
    if kind in ('Bool','Byte') and width=='byte' and 0<=raw<=255 and (kind!='Bool' or raw in (0,1)):
     value=('true' if raw else 'false') if kind=='Bool' else str(raw)
    elif kind=='Int' and width=='dword' and 0<=raw<=0xffffffff:
     n=raw if raw<0x80000000 else raw-0x100000000
     value='(-2147483647 - 1)' if n==-2147483648 else str(n)
    elif kind=='Float' and width=='dword' and 0<=raw<=0xffffffff:
     f=struct.unpack('<f',struct.pack('<I',raw))[0]
     if not math.isfinite(f): reject('nonfinite_float_constant'); continue
     value=format(f,'.9g')
     if '.' not in value and 'e' not in value: value+='.0'
     value+='f'
    else: reject('constant_return_width_or_value'); continue
    ret={'Int':'int32','Float':'float','Byte':'uint8','Bool':'bool'}[kind]
    stem=f'{addr:x}_{cls}_{name}'; inc=HERE/(stem+'.inc')
    body='// Original address: '+hex(addr)+'.\n// CONFIDENCE: HIGH\n// Mechanically matched complete constant return; see bulk_scalar_expansion_runtime/README.md.\nreturn '+value+';\n'
    if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs: '+str(inc))
    if not inc.exists(): inc.write_text(body)
    result={'class':cls,'function':name,'kind':'exec','signature':f"{ret} {name}()"+(' const' if flags&0x40000000 else ''),'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':False,'access':ACCESS[flags&0xe0000], 'notes':f'Original {form} constant {kind} getter at {hex(addr)}; source recovery only.'}
    results.append({'entry':result,'constant_bits':raw,'literal_address':hex(literal),'leaf_assembly':decode(target),'assembly':asm,'target':hex(target)}); continue
 args.output_dir.mkdir(parents=True,exist_ok=True)
 (args.output_dir/'manifest-entries.json').write_text(json.dumps([r['entry'] for r in results],indent=2)+'\n')
 report={'image_sha256':digest,'inventory_records':total,'candidates':len(results),'rejections':dict(sorted(rejects.items())),'results':results}
 (args.output_dir/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps({k:report[k] for k in ('inventory_records','candidates','rejections')},indent=2))
if __name__=='__main__': main()
