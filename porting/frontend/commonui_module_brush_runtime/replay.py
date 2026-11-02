#!/usr/bin/env python3
"""Replay bounded CommonUI module/settings identity evidence."""
import os
import struct
from capstone import Cs, CS_ARCH_X86, CS_MODE_64

BASE = 0x140000000
fd = os.open('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin', os.O_RDONLY)
def read(va, size):
    data = os.pread(fd, size, va - BASE)
    if len(data) != size:
        raise RuntimeError(f'short read at {va:#x}')
    return data
try:
    assert read(0x1459eadb0, 18) == 'CommonUI\0'.encode('utf-16le')
    vtable = struct.unpack('<11Q', read(0x1459fa0d8, 88))
    assert (vtable[1], vtable[4], vtable[10]) == (0x1407fcfe0, 0x1407fb490, 0x140680310)
    assert read(0x140680310, 5) == bytes.fromhex('48 8b 41 08 c3')
    def rip_target(va):
        ins = read(va, 7)
        return va + 7 + struct.unpack_from('<i', ins, 3)[0]
    assert rip_target(0x1401c57d3) == 0x1459eadb0
    assert rip_target(0x1401c581d) == 0x1407ce4b0
    assert rip_target(0x1407ce4c3) == 0x1459fa0d8
    decoder = Cs(CS_ARCH_X86, CS_MODE_64)
    for start, end in ((0x1401c57b0, 0x1401c587a), (0x1407ce4b0, 0x1407ce4df),
                       (0x140680310, 0x140680315), (0x1407fcfe0, 0x1407fd0c4),
                       (0x1407d0c30, 0x1407d0e9c), (0x1407d0ea0, 0x1407d105d),
                       (0x1407fb3e0, 0x1407fb488),
                       (0x1407fb490, 0x1407fb4fb)):
        print(f'[{start:#x}, {end:#x})')
        for ins in decoder.disasm(read(start, end-start), start):
            print(f'{ins.address:x} {ins.mnemonic:7} {ins.op_str}')
    print('literal, registration, factory, and vtable assertions passed')
finally:
    os.close(fd)
