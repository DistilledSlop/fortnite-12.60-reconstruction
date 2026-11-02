#!/usr/bin/env python3
"""Replay bounded original-image windows and PE initialized-data mapping."""
import hashlib
import json
import os
import struct
from pathlib import Path

HERE = Path(__file__).resolve().parent
IMAGE = '/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'
PE = '/mnt/vibrato/Fortnite/FortniteGame/Binaries/Win64/FortniteClient-Win64-Shipping.exe'
BASE = 0x140000000
windows = json.loads((HERE / 'windows.json').read_text())
fd = os.open(IMAGE, os.O_RDONLY)
for name, item in windows.items():
    data = os.pread(fd, int(item['size'], 16), int(item['va'], 16) - BASE)
    assert hashlib.sha256(data).hexdigest() == item['sha256'], name
assert os.pread(fd, 4, 0x783cb80) == b'\x01\0\0\0'
assert os.pread(fd, 42, 0x5a0d640).decode('utf-16le') == 'CommonInput.ShowKeys\0'
assert os.pread(fd, 108, 0x5a0d5d0).decode('utf-16le') == 'Should we show the keys for the current input device.\0'
os.close(fd)
fd = os.open(PE, os.O_RDONLY)
def read(off, n):
    result = os.pread(fd, n, off)
    assert len(result) == n
    return result
peoff = struct.unpack('<I', read(0x3c, 4))[0]
assert read(peoff, 4) == b'PE\0\0'
sections, optsize = struct.unpack('<H', read(peoff + 6, 2))[0], struct.unpack('<H', read(peoff + 20, 2))[0]
sectionoff = peoff + 24 + optsize
rva = 0x783cb80
matches = []
for i in range(sections):
    s = read(sectionoff + 40 * i, 40)
    name = s[:8].rstrip(b'\0')
    virtual_size, va, raw_size, raw = struct.unpack_from('<IIII', s, 8)
    if va <= rva and rva + 4 <= va + min(virtual_size, raw_size):
        matches.append((name, raw + rva - va))
assert matches == [(b'.data', 0x783b980)], matches
assert read(matches[0][1], 4) == b'\x01\0\0\0'
os.close(fd)
print('PASS: 11 bounded runtime windows; PE section mapping and initial int32=1')
