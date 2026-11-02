#!/usr/bin/env python3
"""Decode bounded original-image constructor and registration windows."""
import os
from capstone import Cs, CS_ARCH_X86, CS_MODE_64

p = '/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'
windows = ((0x14081df50, 0x14081dff3), (0x14081e220, 0x14081e234),
           (0x14081a400, 0x14081a693), (0x14081c1e0, 0x14081c260),
           (0x142e80820, 0x142e80828))
fd = os.open(p, os.O_RDONLY)
try:
    decoder = Cs(CS_ARCH_X86, CS_MODE_64)
    for start, end in windows:
        data = os.pread(fd, end-start, start-0x140000000)
        if len(data) != end-start:
            raise RuntimeError(f'short read at {start:#x}')
        print(f'[{start:#x}, {end:#x})')
        for ins in decoder.disasm(data, start):
            print(f'{ins.address:x} {ins.mnemonic:7} {ins.op_str}')
finally:
    os.close(fd)
