#!/usr/bin/env python3
"""Pin the known ammo-definition dispatch; deliberately does not certify the loader."""
import hashlib, importlib.util, json, sys, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
sys.path[:0]=['/tmp/common-input-capstone',str(ROOT/'porting/behavior')]
import build_vtable_slots as vt
spec=importlib.util.spec_from_file_location('proof',ROOT/'porting/behavior/bulk_virtual_scalar_runtime/prepare.py'); proof=importlib.util.module_from_spec(spec); spec.loader.exec_module(proof)
with proof.IMG.open('rb') as f: assert hashlib.file_digest(f,'sha256').hexdigest()==proof.SHA
im=vt.Image(str(proof.IMG))
with tempfile.TemporaryDirectory() as t:
 assert vt.load_gpsc(vt.GPSC_INDEX)['UFortWorldItemDefinition']==0x142b5be40
 assert vt.ctor_of(im,0x142b5be40,t)==0x142042090 and vt.LAST_THUNK[0]==0x142b59760
 assert vt.vptr_candidates(im,0x142042090,t)==[0x14606f8f8] and not vt.thunk_store_candidates(im,0x142b59760,t)
 assert proof.proven_this_store(im,0x142042090,0x14606f8f8,t)
assert len(vt.table_entries(im,0x14606f8f8)) > 0x4e8//8
assert im.u64(0x14606f8f8+0x4e8)==0x140680300
assert im.u64(0x1461fea98+0x4e8)==0x142669040
for a,h in [(0x142b5d0d0,'40534883ec20488b42204533c94885c0498bd8410f95c14c03c84c894a20e8ddb84fff4889034883c4205bc3'),(0x1420589d0,'488b0148ffa0e8040000'),(0x140680300,'33c0c3'),(0x142669040,'4881c178090000b201e9f256e6ff')]:
 b=bytes.fromhex(h); assert im.m[a-vt.BASE:a-vt.BASE+len(b)]==b
f=json.loads((ROOT/'porting/generated/reflection/native_funcs.json').read_text())['UFortWorldItemDefinition']['GetAmmoWorldItemDefinition_BP']
assert f['exec_addr']=='0x142b5d0d0' and f['flags']=='0x54020401' and len(f['params'])==1
p=f['params'][0]; assert p['name']=='ReturnValue' and p['kind']=='Object' and p['type_refs']['class']=='0x142b56ce0' and p['offset']==0 and p['array_dim']==1
print('Base slot, reflected wrapper, dispatch thunk and weapon override entry verified; loader remains open')
