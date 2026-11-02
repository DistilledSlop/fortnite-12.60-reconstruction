#!/usr/bin/env python3
"""Check bounded original mapped-image construct and package identity windows."""
import os
import struct
BASE = 0x140000000
IMAGE = '/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'
fd = os.open(IMAGE, os.O_RDONLY)
def read(va, size):
    data = os.pread(fd, size, va - BASE)
    if len(data) != size: raise ValueError('short image read')
    return data
def lea_target(va):
    data = read(va, 7)
    if data[:3] != b'\x48\x8d\x15': raise ValueError('unexpected LEA')
    return va + 7 + struct.unpack('<i', data[3:])[0]
assert lea_target(0x140809730) == 0x14782f0f0
assert lea_target(0x140812ef0) == 0x145a0aa68
assert struct.unpack('<Q', read(0x14782f0f0, 8))[0] == 0x140812ee0
assert read(0x145a0aa88, 17) == b'/Script/CommonUI\0'
os.close(fd)
print('PASS: bounded construct, delegate block, outer and package windows')
