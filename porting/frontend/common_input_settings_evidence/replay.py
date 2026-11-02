#!/usr/bin/env python3
"""Disassemble bounded original-image ranges; addresses are runtime VAs."""
import os, sys
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
p='/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'
a,b=(int(x,16) for x in sys.argv[1:3]); assert 0 < b-a <= 0x4000
fd=os.open(p,os.O_RDONLY)
d=os.pread(fd,b-a,a-0x140000000); os.close(fd)
for q in Cs(CS_ARCH_X86,CS_MODE_64).disasm(d,a): print(f'{q.address:x} {q.mnemonic:7} {q.op_str}')
