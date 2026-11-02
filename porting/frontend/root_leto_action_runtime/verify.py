#!/usr/bin/env python3
"""Bounded original image checks for the Leto action source boundary."""
import os
import sys
sys.path.insert(0, '/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64

BASE = 0x140000000
fd = os.open('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin', os.O_RDONLY)
def read(va, size):
    data = os.pread(fd, size, va - BASE)
    assert len(data) == size
    return data
def insns(va, size):
    return {i.address: (i.mnemonic, i.op_str) for i in Cs(CS_ARCH_X86, CS_MODE_64).disasm(read(va, size), va)}
def expect(va, mnemonic, operand):
    got = insns(va, 16)[va]
    assert got == (mnemonic, operand), (hex(va), got)
def literal(va, value):
    assert read(va, (len(value)+1)*2) == (value+'\0').encode('utf-16le')
for va, op, arg in [
    (0x14516be53,'call','qword ptr [rax + 0x290]'),
    (0x14516be8d,'cmp','byte ptr [rdi + 0x2aa], r12b'),
    (0x14516bec9,'call','0x1407d00b0'),
    (0x14516c0c5,'call','0x1407ed3f0'),
    (0x14516c0cc,'mov','dword ptr [rdi + 0x360], ecx'),
    (0x14516c1bf,'call','0x1408018c0'),
    (0x145155c3e,'cmp','byte ptr [rax + 0x68], 1'),
    (0x145155cc5,'call','0x14516e780'),
    (0x145155cd4,'call','0x14516e780'),
    (0x1405ffc6c,'call','qword ptr [rax + 0x40]'),
]: expect(va,op,arg)
for va, value in [(0x146e43220,'FortUI.EnableP2Frontend'), (0x146e461b0,'LetoTakeControlActionPrompt'),(0x146e461e8,'FortRootViewportLayout_Athena'),(0x146e46228,'Take Control'),(0x146e46248,'LetoLoginActionPrompt'),(0x146e46278,'Log in')]: literal(va,value)
print('PASS: bounded Leto action instructions and literals')
