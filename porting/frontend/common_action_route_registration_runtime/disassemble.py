"""Bounded mapped-image decoder; never reads the complete original image."""
import os
import sys
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
start, end = (int(x, 0) for x in sys.argv[1:])
assert 0x140000000 <= start < end and end - start <= 0x2000
fd = os.open('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin', os.O_RDONLY)
try:
    data = os.pread(fd, end - start, start - 0x140000000)
finally:
    os.close(fd)
for ins in Cs(CS_ARCH_X86, CS_MODE_64).disasm(data, start):
    print(f'{ins.address:x}: {ins.mnemonic} {ins.op_str}')
