#!/usr/bin/env python3
"""Inventory potential two-parameter inline empty execs; never emits bodies."""
import collections, hashlib, importlib.util, json, sys
from pathlib import Path
HERE=Path(__file__).resolve().parent
p=HERE.parent/'bulk_float_setter_runtime'/'prepare.py'
s=importlib.util.spec_from_file_location('float_scanner',p); f=importlib.util.module_from_spec(s); s.loader.exec_module(f)
sys.path.insert(0,'/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64

def main():
 img=f.IMAGE.read_bytes(); digest=hashlib.sha256(img).hexdigest()
 if digest!=f.SHA: raise SystemExit('image digest mismatch')
 native=json.loads((f.REFL/'native_funcs.json').read_text()); md=Cs(CS_ARCH_X86,CS_MODE_64)
 eligible=0; direct_empty=[]; inline=[]
 for cls,funcs in sorted(native.items()):
  for name,fn in sorted(funcs.items()):
   if not isinstance(fn,dict) or not isinstance(fn.get('params'),list) or len(fn['params'])!=2: continue
   try: flags=int(fn['flags'],0); a=int(fn['exec_addr'],0)
   except (KeyError,ValueError,TypeError): continue
   if flags & (~0xffffffcf|f.NET|0x08000000|0x20000000|0x2000|0x40000000) or not flags&0x400 or (flags&0xe0000) not in f.ACCESS: continue
   if not f.BASE<=a<f.BASE+len(img)-512: continue
   eligible+=1; ins=[]
   for i in md.disasm(img[a-f.BASE:a-f.BASE+512],a):
    ins.append(i)
    if i.mnemonic=='ret': break
   else: continue
   calls=[i for i in ins if i.mnemonic=='call']; steps={'0x142ed9cf0','0x142ed9d20'}
   item={'class':cls,'function':name,'exec_addr':hex(a),'params':fn['params'],'asm':'; '.join((i.mnemonic+' '+i.op_str).rstrip() for i in ins)}
   for call in calls:
    try: target=int(call.op_str,0)
    except ValueError: continue
    if f.BASE<=target<f.BASE+len(img) and img[target-f.BASE]==0xc3: direct_empty.append({**item,'target':hex(target)})
   if len(calls)==4 and collections.Counter(i.op_str for i in calls)==collections.Counter({'0x142ed9cf0':2,'0x142ed9d20':2}): inline.append(item)
 out=Path('/tmp/gameplay-bulk-ignored-multi'); out.mkdir(parents=True,exist_ok=True)
 report={'image_sha256':digest,'eligible_two_reflection_records':eligible,'direct_ret_call_sightings':direct_empty,'inline_step_only_sightings':inline}
 (out/'probe.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps({'eligible_two_reflection_records':eligible,'direct_ret_call_sightings':len(direct_empty),'inline_step_only_sightings':len(inline),'inline':[(x['class'],x['function'],x['exec_addr']) for x in inline]},indent=2))
if __name__=='__main__': main()
