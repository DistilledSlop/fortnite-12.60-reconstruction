#!/usr/bin/env python3
"""Find complete small two-argument direct-call leaves, without proposing bodies."""
import collections,hashlib,importlib.util,json,sys,re
from pathlib import Path
HERE=Path(__file__).resolve().parent
p=HERE.parent/'bulk_float_setter_runtime'/'prepare.py'
s=importlib.util.spec_from_file_location('base',p); b=importlib.util.module_from_spec(s); s.loader.exec_module(b)
sys.path.insert(0,'/tmp/common-input-capstone')
from capstone import Cs,CS_ARCH_X86,CS_MODE_64
img=b.IMAGE.read_bytes(); digest=hashlib.sha256(img).hexdigest()
if digest!=b.SHA: raise SystemExit('image digest mismatch')
md=Cs(CS_ARCH_X86,CS_MODE_64)
def decode(a):
 if not b.BASE<=a<b.BASE+len(img)-512: return []
 ins=[]
 for i in md.disasm(img[a-b.BASE:a-b.BASE+512],a):
  ins.append(i)
  if i.mnemonic=='ret': return ins
 return []
def fmt(ins): return '; '.join((i.mnemonic+' '+i.op_str).rstrip() for i in ins)
native=json.loads((b.REFL/'native_funcs.json').read_text()); out=[]; counts=collections.Counter()
for cls,funcs in sorted(native.items()):
 for name,fn in sorted(funcs.items()):
  if not isinstance(fn,dict) or not isinstance(fn.get('params'),list) or len(fn['params'])!=2: continue
  try: flags=int(fn['flags'],0); a=int(fn['exec_addr'],0)
  except (KeyError,TypeError,ValueError): continue
  if flags & (~0xffffffcf|b.NET|0x08000000|0x20000000|0x2000|0x40000000) or not flags&0x400 or (flags&0xe0000) not in b.ACCESS: continue
  ps=fn['params']
  if any(p.get('kind') not in ('Int','Float','Byte','Bool','Object') or p.get('array_dim')!=1 or int(p.get('property_flags','0'),0)&(0x400|0x100|0x08000000) for p in ps): continue
  counts['eligible']+=1; ins=decode(a); calls=[i for i in ins if i.mnemonic=='call']; steps={'0x142ed9cf0','0x142ed9d20'}
  if collections.Counter(i.op_str for i in calls if i.op_str in steps)!=collections.Counter({'0x142ed9cf0':2,'0x142ed9d20':2}) or len(calls)!=5: continue
  call=next(i for i in calls if i.op_str not in steps)
  try: target=int(call.op_str,0)
  except ValueError: continue
  leaf=decode(target)
  if not leaf or len(leaf)>8 or any(i.mnemonic in ('call','jmp') or i.mnemonic.startswith('j') for i in leaf): continue
  if not any(i.mnemonic in ('mov','or','and','xor') and i.op_str.startswith(('byte ptr [rcx','dword ptr [rcx','qword ptr [rcx')) for i in leaf): continue
  counts['small_leaf_sightings']+=1
  out.append({'class':cls,'function':name,'params':ps,'exec_addr':hex(a),'target':hex(target),'exec':fmt(ins),'leaf':fmt(leaf)})
path=Path('/tmp/gameplay-bulk-two-scalar'); path.mkdir(exist_ok=True)
(path/'probe.json').write_text(json.dumps({'image_sha256':digest,'counts':counts,'sightings':out},indent=2)+'\n')
print(json.dumps({'counts':counts,'sightings':[{k:x[k] for k in ('class','function','exec_addr','target','leaf')} for x in out]},indent=2))
