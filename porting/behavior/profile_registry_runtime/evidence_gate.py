#!/usr/bin/env python3
"""Verify bounded original-image receipts; does not validate compiled behavior."""
import hashlib
import struct
from pathlib import Path

IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
BASE = 0x140000000
WINDOWS = {
    (0x1410f65e0, 0x1410f66a3): 'ba6959fa80584a2d5ef33b416503ab8e78291617a8b7f9370b938db46e91a541',
    (0x1410eca90, 0x1410ecb59): '83c08096989f6f348672b11c7ec8d8b1d3eeb7e9cc4db26d5c58f109be027a3f',
    (0x141104980, 0x141104be4): 'e44c8a6f1d650c2caa33dd7750471a03db87a1eb0fbe723b353a808708c78061',
    (0x140b77eb0, 0x140b77f9c): 'c4e25aee5f1aeb23ba8f0899b9d0f98f0b9c5c153ed0d75f43bc6b36be265827',
}
with IMAGE.open('rb') as f:
    assert hashlib.file_digest(f, 'sha256').hexdigest() == '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
    for (start, end), expected in WINDOWS.items():
        f.seek(start - BASE)
        assert hashlib.sha256(f.read(end - start)).hexdigest() == expected, hex(start)
    f.seek(0x145bfc2e0 + 0x2a0 - BASE)
    assert struct.unpack('<Q', f.read(8))[0] == 0x1410f65e0
print('Original image, four bounded windows, and factory vtable slot verified')
