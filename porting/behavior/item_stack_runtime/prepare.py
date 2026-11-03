#!/usr/bin/env python3
"""Original evidence gate for stack limits and temporary ownership, no runtime tests."""
import hashlib, importlib.util, json, sys, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]; HERE=Path(__file__).resolve().parent
sys.path[:0]=['/tmp/common-input-capstone',str(ROOT/'porting/behavior')]
import build_vtable_slots as vt
s=importlib.util.spec_from_file_location('proof',ROOT/'porting/behavior/bulk_virtual_scalar_runtime/prepare.py');p=importlib.util.module_from_spec(s);s.loader.exec_module(p)
with p.IMG.open('rb') as f: assert hashlib.file_digest(f,'sha256').hexdigest()==p.SHA
im=vt.Image(str(p.IMG))
for a,raw in json.loads((HERE/'evidence.json').read_text()).items():
 b=bytes.fromhex(raw); assert im.m[int(a,0)-vt.BASE:int(a,0)-vt.BASE+len(b)]==b
with tempfile.TemporaryDirectory() as tmp:
 for cls,gpsc,ctor,table in [('UFortItem',0x142951e90,0x142024010,0x14606c3f0),('UFortItemDefinition',0x14295c2b0,0x1420241d0,0x14606bf00),('UFortWorldItem',0x142b5bd80,0x142041fc0,0x14607a630)]:
  assert vt.load_gpsc(vt.GPSC_INDEX)[cls]==gpsc
  assert vt.ctor_of(im,gpsc,tmp)==ctor
  assert vt.vptr_candidates(im,ctor,tmp)==[table]
  assert p.proven_this_store(im,ctor,table,tmp)
for table in [0x14606c3f0,0x14607a630,0x1463af658,0x1464e8cd8]:
 assert im.u64(table+0x370)==0x14202fe30
 assert im.u64(table+0x4b8)==0x142035020
assert im.u64(0x14606bf00+0x390)==0x14202fe70
assert vt.load_gpsc(vt.GPSC_INDEX)['UFortPlayerAttrSet']==0x142a365f0
load=lambda p:json.loads((ROOT/p).read_text())
names=load('porting/generated/reflection/class_name_map.json')
blocks={x['block_va']:x for x in load('porting/generated/reflection/fortnite_game_reflection.json')['classes_in_groups_with_fort_names']}
cls=blocks[next(k for k,v in names.items() if v=='UFortItemDefinition')]
f=next(x for x in cls['properties'] if x['name']=='MaxStackSize')
assert f['offset']==0x168 and f['kind']=='Struct' and f['type_refs']=={'struct':'0x140762fe0'}
b=next(x for x in load('porting/generated/reflection/bool_bits.json')['by_class']['UFortItem'] if x['name']=='bTemporaryItemOwningController')
assert b['offset']==0x80 and b['mask']==2
n=load('porting/generated/reflection/native_funcs.json')['UFortItem']
for name,addr,k in [('GetMaxStackSize','0x1429593f0','Int'),('IsTemporaryItem','0x14295a4b0','Bool')]:
 f=n[name];assert f['exec_addr']==addr and f['flags']=='0x54020400'
 ret=next(p for p in f['params'] if p['name']=='ReturnValue');assert ret['kind']==k and int(ret['property_flags'],0)==0x10000000000580
f=n['GetMaxStackSize'];parm=next(p for p in f['params'] if p['name']=='AbilityComp')
assert len(f['params'])==2 and parm['kind']=='Object' and parm['type_refs']=={'class':'0x14074f1d0'} and int(parm['property_flags'],0)==0x10000000080080
# Original helper performs checked lookup and source/snapshot=0/1 capture.
raw=bytes.fromhex(load('porting/behavior/item_stack_runtime/evidence.json')['0x14202fe70'])
assert raw[900:909].hex() == '66c7053bc110060001'
for filename,sha in load('porting/behavior/item_stack_runtime/source-hashes.json').items():
 assert hashlib.sha256((HERE/filename).read_bytes()).hexdigest()==sha
print('Stack/ownership original contracts and reviewed source verified')
