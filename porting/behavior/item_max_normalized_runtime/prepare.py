#!/usr/bin/env python3
"""Evidence gate for the UFortWorldItemDefinition GetMaxDurability base body."""
import hashlib, importlib.util, json, sys, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]; HERE=Path(__file__).resolve().parent
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
assert len(vt.table_entries(im,0x14606f8f8)) > 0x4d8//8
assert im.u64(0x14606f8f8+0x4d8)==0x1407c46d0
sections=[(n,ch) for n,start,size,ch in im.secs if start<=0x1459a86f8-vt.BASE and 0x1459a86f8-vt.BASE+4<=start+size]
assert len(sections)==1 and sections[0][1]&0x40 and sections[0][1]&0x40000000 and not sections[0][1]&0x80000000
for addr,raw in [(0x142b5d270,'48895c241048896c24184889742420574883ec2033ff488bf1498be8488bda488bca897c243048397a207410488b52184c8d442430e846ca3700eb1c4c8b8280000000498b402048898280000000488d542430e858ca3700488b4320488bce8b5424304885c0400f95c74803f848897b20488b06ff90d8040000488b5c2438488b742448f30f114500488b6c24404883c4205fc3'),(0x1407c46d0,'f30f100520401e05c3'),(0x1459a86f8,'0000803f')]:
 b=bytes.fromhex(raw); assert im.m[addr-vt.BASE:addr-vt.BASE+len(b)]==b
f=json.loads((ROOT/'porting/generated/reflection/native_funcs.json').read_text())['UFortWorldItemDefinition']['GetMaxDurability']
assert f['exec_addr']=='0x142b5d270' and f['flags']=='0x54020400' and len(f['params'])==2
for p,n,k,raw,flags,off in zip(f['params'],['ReturnValue','ItemLevel'],['Float','Int'],['0xa','0x3'],[0x10000000000580,0x10000000000080],[4,0]):
 assert p['name']==n and p['kind']==k and p['kind_raw']==raw and int(p['property_flags'],0)==flags and p['offset']==off and p['array_dim']==1 and not p.get('type_refs')
print('GetMaxDurability wrapper, primary slot, base body and reflection verified')
