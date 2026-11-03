#!/usr/bin/env python3
"""Read-only original evidence gate for the assigned dispatch contracts."""
import hashlib, json, sys, tempfile
from pathlib import Path
ROOT = Path(__file__).resolve().parents[3]
HERE = Path(__file__).resolve().parent
sys.path[:0] = ['/tmp/common-input-capstone', str(ROOT / 'porting/behavior')]
import build_vtable_slots as vt
IMG = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
with IMG.open('rb') as f:
    assert hashlib.file_digest(f, 'sha256').hexdigest() == '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
im = vt.Image(str(IMG))
for addr, raw in json.loads((HERE / 'native-evidence.json').read_text()).items():
    expected = bytes.fromhex(raw)
    offset = int(addr, 0) - vt.BASE
    assert im.m[offset:offset + len(expected)] == expected
for table in (0x14606c3f0, 0x14607a630, 0x1463af658, 0x1464e8cd8):
    assert im.u64(table + 0x318) == 0x1420337a0
assert im.u64(0x14606c3f0 + 0x468) == 0x140693ec0
for table in (0x14607a630, 0x1463af658, 0x1464e8cd8):
    assert im.u64(table + 0x468) == 0x14205a3d0
for table in (0x14606bf00, 0x14606f8f8):
    assert im.u64(table + 0x298) == 0x14202f190
with tempfile.TemporaryDirectory() as tmp:
    gpsc = vt.load_gpsc(vt.GPSC_INDEX)['AFortInventory']
    assert gpsc == 0x142952aa0
    ctor = vt.ctor_of(im, gpsc, tmp)
    assert ctor == 0x141ff9970
    assert vt.vptr_candidates(im, ctor, tmp) == [0x14605b6b8]
assert im.u64(0x14605b6b8 + 0x630) == 0x1420159a0
for name, digest in json.loads((HERE / 'source-hashes.json').read_text()).items():
    assert hashlib.sha256((HERE / name).read_bytes()).hexdigest() == digest
print('Original windows, dispatch slots, and proposal source hashes verified')
