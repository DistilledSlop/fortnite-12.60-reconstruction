"""Bounded memory-layout original-image reader; never reads/hashes the full image."""
import sys
from pathlib import Path
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
BASE = 0x140000000

def read(address, size):
    if address < BASE or not 0 < size <= 65536:
        raise ValueError('invalid bounded image window')
    with IMAGE.open('rb') as image:
        image.seek(address - BASE)
        data = image.read(size)
    if len(data) != size:
        raise ValueError('short original image read')
    return data

if __name__ == '__main__':
    start, end = (int(arg, 0) for arg in sys.argv[1:])
    for ins in Cs(CS_ARCH_X86, CS_MODE_64).disasm(read(start, end-start), start):
        print(f'{ins.address:x} {ins.mnemonic:7} {ins.op_str}')
