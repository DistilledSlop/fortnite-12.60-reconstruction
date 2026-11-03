#!/usr/bin/env python3
"""Inventory reflected WorldItem subclasses, primary slots, and transitive item virtuals."""
import hashlib, importlib.util, json, sys, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
sys.path[:0]=['/tmp/common-input-capstone',str(ROOT/'porting/behavior')]
import build_vtable_slots as vt
spec=importlib.util.spec_from_file_location('proof',ROOT/'porting/behavior/bulk_virtual_scalar_runtime/prepare.py'); proof=importlib.util.module_from_spec(spec); spec.loader.exec_module(proof)
with proof.IMG.open('rb') as f: assert hashlib.file_digest(f,'sha256').hexdigest()==proof.SHA
im=vt.Image(str(proof.IMG))
parents=json.loads((ROOT/'porting/generated/reflection/class_parent_map.json').read_text())
classes={x['name'] for x in parents.values() if 'UFortWorldItem' in x['chain'] and x['chain_ok']}
assert classes=={'UFortWorldItem','UFortCreativeWorldItem','UFortPlaysetWorldItem'}
assert not [x for x in parents.values() if 'UFortWorldItem' in x['chain'] and not x['chain_ok']]
gpsc=vt.load_gpsc(vt.GPSC_INDEX)
expected={'UFortWorldItem':(0x142b5bd80,0x142b59740,0x14607a630),'UFortCreativeWorldItem':(0x1428c9f90,0x1428cb480,0x1463af658),'UFortPlaysetWorldItem':(0x142a9fe20,0x142aa06e0,0x1464e8cd8)}
# Prefixes establish the subclass thunk loads original this from [rcx], passes it
# to the WorldItem ctor, then stores the derived primary vptr through that same this.
for addr,raw in [(0x1428cb480,'40534883ec20488b194885db7470488bd1488bcbe8276b77ff488d05b841ae03c78360020000ffffffff488903'),(0x142aa06e0,'40534883ec20488b194885db7451488bd1488bcbe8c7185aff488d05d885a403c6835802000000488903')]:
 b=bytes.fromhex(raw); assert im.m[addr-vt.BASE:addr-vt.BASE+len(b)]==b
with tempfile.TemporaryDirectory() as t:
 for cls,(g,thunk,table) in expected.items():
  assert gpsc[cls]==g and vt.ctor_of(im,g,t)==0x142041fc0 and vt.LAST_THUNK[0]==thunk
  assert vt.vptr_candidates(im,0x142041fc0,t)==[0x14607a630]
  assert proof.proven_this_store(im,0x142041fc0,0x14607a630,t)
  assert vt.thunk_store_candidates(im,thunk,t)==([] if cls=='UFortWorldItem' else [table])
  assert len(vt.table_entries(im,table))>0x3e8//8
  for slot,target in [(0x268,0x14205a560),(0x2c8,0x142059c00),(0x2c0,0x14205ca30),(0x3e8,0x14205a2f0)]: assert im.u64(table+slot)==target
# The GetDurability / HasDurability / definition and level bodies and leaves
# are pinned by the sibling proofs, including the transitive read-only calls.
for addr,raw in [(0x142059c00,'40534883ec20488b01488bd9ff90c002000084c07411488d8be00000004883c4205be91937fdfff30f10056dfb94034883c4205bc3'),(0x14205ca30,'40534883ec20488b01ff90e8030000488bd84885c07431e8f4f3af00488b4b104883c030486350083b51387f1b488b4930483904d17511f68343030000017408b0014883c4205bc332c04883c4205bc3'),(0x14205a2f0,'4881c1e0000000e9247b76fe'),(0x1407c1e20,'488b4118c3'),(0x14202d340,'f30f104124c3'),(0x14205a560,'4881c1e0000000e9a46622ff'),(0x141280c10,'8b4128c3')]:
 b=bytes.fromhex(raw); assert im.m[addr-vt.BASE:addr-vt.BASE+len(b)]==b
sections=[ch for n,start,size,ch in im.secs if start<=0x1459a86f8-vt.BASE and 0x1459a86fc-vt.BASE<=start+size]
assert len(sections)==1 and sections[0]&0x40 and sections[0]&0x40000000 and not sections[0]&0x80000000
raw=bytes.fromhex('4889742410574883ec30488b01488bf1ff90e8030000488bf84885c07423e89d14b000488b57104c8d4030486340383b42387f0d488bc8488b42304c3904c8740233ff4885ff7468f6874303000001745f488b06488bce48895c2440488b1f0f297c2420ff90c8020000488b06488bce0f28f8ff90680200008bd0488bcfff93d8040000488b5c2440f30f5ef80f57c00f2ff8720bf30f5d3ddbdc94030f28c70f287c2420488b7424484883c4305fc3f30f1005c0dc9403488b7424484883c4305fc3')
assert im.m[0x14205a980-vt.BASE:0x14205aa43-vt.BASE]==raw
assert im.u64(0x14607a630+0x2d0)==0x14205a980
assert im.m[0x1459a86f8-vt.BASE:0x1459a86fc-vt.BASE]==bytes.fromhex('0000803f')
bits=json.loads((ROOT/'porting/generated/reflection/bool_bits.json').read_text())['by_class']['UFortWorldItemDefinition']
b=next(x for x in bits if x['name']=='bItemHasDurability'); assert b['offset']==0x343 and b['mask']==1
native=json.loads((ROOT/'porting/generated/reflection/native_funcs.json').read_text())['UFortItem']
for name in ['GetDurability','GetLevel','HasDurability','GetItemDefinitionBP']:
 flags=int(native[name]['flags'],0); assert not flags & 0x8000000 and not flags & 0x800
print('Three reflected native WorldItem classes have the same four read-only virtual targets')
