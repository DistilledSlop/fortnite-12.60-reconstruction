#!/usr/bin/env python3
"""Replay only the original-image windows establishing the transient outer."""
import os
import struct

from capstone import Cs, CS_ARCH_X86, CS_MODE_64

PATH = '/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'
BASE = 0x140000000
fd = os.open(PATH, os.O_RDONLY)
try:
    def read(address, size):
        data = os.pread(fd, size, address - BASE)
        if len(data) != size:
            raise RuntimeError(f'short read at {address:#x}')
        return data

    decoder = Cs(CS_ARCH_X86, CS_MODE_64)
    windows = (
        (0x1401c7e20, 0x1401c7e62), # LogCommonInput category construction
        (0x1401c7e90, 0x1401c7fbb), # CommonInput lazy name / factory registration
        (0x140680340, 0x14068038b), # delegate clone and execute
        (0x1406834e0, 0x140683554), # clone allocated into destination delegate
        (0x142cbc9d0, 0x142cbc9d5), # manager getter thunk
        (0x142cbd480, 0x142cbd4b6), # manager singleton return
        (0x14081d150, 0x14081d190),  # existing module startup calls outer getter
        (0x142e80820, 0x142e80828),  # getter of the shared global
        (0x142e839b6, 0x142e83a70), # named package creation and rooting
        (0x140e88cd0, 0x140e88cd5), # package class getter thunk
        (0x142e80640, 0x142e806f3), # class registration: Package / CoreUObject
        (0x142e8dca0, 0x142e8dd1a), # unroot and clear same global, RBX = zero
    )
    for start, end in windows:
        print(f'[{start:#x}, {end:#x})')
        for ins in decoder.disasm(read(start, end - start), start):
            print(f'{ins.address:x} {ins.mnemonic:7} {ins.op_str}')
    for address, value in (
        (0x146619cc8, '/Engine/Transient'),
        (0x146614ce2, 'Package'),
        (0x145a61098, '/Script/CoreUObject'),
        (0x145a0d5b8, 'CommonInput'),
        (0x1459eb748, 'LogCommonInput'),
    ):
        actual = read(address, (len(value) + 1) * 2).decode('utf-16le')
        if actual != value + '\0':
            raise RuntimeError(f'literal mismatch at {address:#x}')
        print(f'literal {address:x}: {value}')
    for address in (0x142e80820, 0x142e83a29, 0x142e8dd13):
        target = address + 7 + struct.unpack('<i', read(address + 3, 4))[0]
        if target != 0x148261550:
            raise RuntimeError(f'global target mismatch at {address:#x}')
        print(f'global reference {address:x} -> {target:x}')
    print('registration delegate vtable:')
    raw = read(0x1459a43f8, 0x50)
    for offset in range(0, len(raw), 8):
        print(f'+{offset:#x} {struct.unpack_from("<Q", raw, offset)[0]:#x}')
finally:
    os.close(fd)
