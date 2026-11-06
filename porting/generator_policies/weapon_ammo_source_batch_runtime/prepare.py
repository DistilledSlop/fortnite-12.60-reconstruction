#!/usr/bin/env python3
"""Verify image pins and constructor-to-primary-table provenance; no files mutated."""
import hashlib, importlib.util, json, sys, tempfile
from pathlib import Path
ROOT = Path(__file__).resolve().parents[3]
HERE = Path(__file__).resolve().parent
sys.path[:0] = ['/tmp/common-input-capstone', str(ROOT / 'porting/behavior')]
import build_vtable_slots as vt
spec = importlib.util.spec_from_file_location('proof', ROOT / 'porting/behavior/bulk_virtual_scalar_runtime/prepare.py')
proof = importlib.util.module_from_spec(spec); spec.loader.exec_module(proof)
pins = json.loads((HERE / 'native-byte-pins.json').read_text())
with proof.IMG.open('rb') as f:
    assert hashlib.file_digest(f, 'sha256').hexdigest() == pins['image_sha256']
im = vt.Image(str(proof.IMG))
for pin in pins['spans']:
    pos = int(pin['address'], 16) - vt.BASE
    if 'bytes' in pin:
        expected = bytes.fromhex(pin['bytes']); assert im.m[pos:pos + len(expected)] == expected
    else:
        assert hashlib.sha256(im.m[pos:pos + pin['length']]).hexdigest() == pin['sha256']
with tempfile.TemporaryDirectory() as d:
    for cls, gpsc, ctor, thunk, table, target in [
        ('UFortWorldItemDefinition', 0x142b5be40, 0x142042090, 0x142b59760, 0x14606f8f8, 0x140680300),
        ('UFortWeaponItemDefinition', 0x142b4a5d0, 0x14265c0b0, 0x142b4a230, 0x1461fea98, 0x142669040),
    ]:
        assert vt.load_gpsc(vt.GPSC_INDEX)[cls] == gpsc
        assert vt.ctor_of(im, gpsc, d) == ctor and vt.LAST_THUNK[0] == thunk
        assert vt.vptr_candidates(im, ctor, d) == [table]
        assert not vt.thunk_store_candidates(im, thunk, d)
        assert proof.proven_this_store(im, ctor, table, d)
        assert im.u64(table + 0x4e8) == target
f = json.loads((ROOT / 'porting/generated/reflection/native_funcs.json').read_text())['UFortWorldItemDefinition']['GetAmmoWorldItemDefinition_BP']
assert f['exec_addr'] == '0x142b5d0d0' and f['flags'] == '0x54020401' and len(f['params']) == 1
p = f['params'][0]
assert p['name'] == 'ReturnValue' and p['kind'] == 'Object' and p['type_refs']['class'] == '0x142b56ce0' and p['offset'] == 0 and p['array_dim'] == 1
assert 'TSoftObjectPtr<UFortWorldItemDefinition> AmmoData;' in (ROOT / 'porting/native/Source/FortniteGame/Generated/UFortWeaponItemDefinition.h').read_text()
print('Ammo definition image pins, reflection, and two constructor tables verified')
