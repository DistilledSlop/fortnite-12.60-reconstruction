#!/usr/bin/env python3
"""Extract finite original release paths; no source promotion or runtime tests."""
import hashlib, json, sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[3]
sys.path[:0] = ['/tmp/common-input-capstone', str(ROOT / 'porting/behavior')]
import build_vtable_slots as vt
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
SHA = '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
with IMAGE.open('rb') as f:
    assert hashlib.file_digest(f, 'sha256').hexdigest() == SHA
im = vt.Image(str(IMAGE))
cs = Cs(CS_ARCH_X86, CS_MODE_64)
records = {}
for va in [0x141e0de80, 0x140742690, 0x141da4450, 0x141ded253]:
    lo, hi = im.func_of(va)
    raw = bytes(im.m[lo-vt.BASE:hi-vt.BASE])
    records[hex(va)] = {'begin': hex(lo), 'end': hex(hi), 'bytes': raw.hex(),
        'instructions': [{'address': hex(x.address), 'mnemonic': x.mnemonic, 'operands': x.op_str}
                         for x in cs.disasm(raw, lo)]}
Path(__file__).with_name('evidence.json').write_text(json.dumps({'image_sha256': SHA, 'functions': records}, indent=2) + '\n')
print('Recovered four original code ranges; no manifest/native changes.')
