#!/usr/bin/env python3
"""Check immutable original image and Gadget original-this slot provenance."""
import hashlib, importlib.util, sys, tempfile
from pathlib import Path
ROOT = Path(__file__).resolve().parents[3]
sys.path[:0] = ['/tmp/common-input-capstone', str(ROOT / 'porting/behavior')]
import build_vtable_slots as vt
spec = importlib.util.spec_from_file_location('proof', ROOT / 'porting/behavior/bulk_virtual_scalar_runtime/prepare.py')
proof = importlib.util.module_from_spec(spec); spec.loader.exec_module(proof)
with proof.IMG.open('rb') as f:
    assert hashlib.file_digest(f, 'sha256').hexdigest() == '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
im = vt.Image(str(proof.IMG))
for address, length, digest in [(0x142008520, 891, '86d70ec2c986b96f3d775a3a69d0dea5ba8b30977b4e95b79eb2f9e2d6723335'), (0x141680900, 487, '502a104bc0f61cce9796ae51f5be7ccde4666fb985ad10d39591a83074af7e34')]:
    assert hashlib.sha256(im.m[address-vt.BASE:address-vt.BASE+length]).hexdigest() == digest
with tempfile.TemporaryDirectory() as d:
    assert vt.load_gpsc(vt.GPSC_INDEX)['UFortGadgetItemDefinition'] == 0x1428e9db0
    assert vt.ctor_of(im, 0x1428e9db0, d) == 0x141ffc6f0 and vt.LAST_THUNK[0] == 0x1428e9910
    assert vt.vptr_candidates(im, 0x141ffc6f0, d) == [0x1460658e0]
    assert not vt.thunk_store_candidates(im, 0x1428e9910, d)
    assert proof.proven_this_store(im, 0x141ffc6f0, 0x1460658e0, d)
    assert im.u64(0x1460658e0 + 0x4e8) == 0x142008520
print('Gadget ammo original image, helper and constructor slot verified')
