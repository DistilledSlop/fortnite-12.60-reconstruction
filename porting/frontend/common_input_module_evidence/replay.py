#!/usr/bin/env python3
"""Print bounded original-image windows used by the CommonInput module investigation."""
import os
import struct

from capstone import Cs, CS_ARCH_X86, CS_MODE_64

path = '/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'
base = 0x140000000
fd = os.open(path, os.O_RDONLY)
try:
    def read(addr, size):
        data = os.pread(fd, size, addr - base)
        if len(data) != size:
            raise RuntimeError(f'short image window at {addr:#x}')
        return data

    decoder = Cs(CS_ARCH_X86, CS_MODE_64)
    for start, end in ((0x1407c4fc0, 0x1407c5054), (0x14081bef0, 0x14081bf1f),
                       (0x14081d150, 0x14081d230), (0x142e80820, 0x142e80828),
                       (0x142ed7360, 0x142ed73c5)):
        print(f'[{start:#x}, {end:#x})')
        for ins in decoder.disasm(read(start, end - start), start):
            print(f'{ins.address:x} {ins.mnemonic:7} {ins.op_str}')
    print('module vtable:')
    raw = read(0x145a0db48, 0x48)
    for offset in range(0, len(raw), 8):
        print(f'+{offset:#x} {struct.unpack_from("<Q", raw, offset)[0]:#x}')
    print('assertion text:', read(0x1459a4ca0, 0x100).decode('utf-16le').split('\0', 1)[0])
    print('module name:', read(0x1459ef360, 12).split(b'\0', 1)[0].decode('ascii'))
finally:
    os.close(fd)
