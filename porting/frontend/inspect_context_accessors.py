#!/usr/bin/env python3
"""Pin the original class identities used by BlueprintContext.GetContext.

The input is a memory image: use RVAs directly, never PE section raw offsets.
This only writes fresh evidence; it does not execute or modify the client.
"""
import argparse
import bisect
import hashlib
import json
import mmap
from pathlib import Path
import re
import struct
import subprocess

IMAGE_BASE = 0x140000000
IMAGE_SHA256 = '604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d'
ACCESSORS = {
    0x143DBE060: 'UserWidget',
    0x1447BB440: 'PlayerController',
    0x144786700: 'LocalPlayer',
    0x1447867C0: 'LocalPlayerSubsystem',
    0x14470A080: 'GameInstanceSubsystem',
    0x144801560: 'Subsystem',
    0x1408232C0: 'BlueprintContextBase',
}
LOOKUP_AND_MANAGER = (0x140822310, 0x140822490, 0x14081FCA0,
                      0x1408219D0, 0x140821B10, 0x140822A30, 0x140822C00)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dump', type=Path, default=Path(
        'porting/generated/dump/FortniteClient-Win64-Shipping.runtime.bin'))
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    with args.dump.open('rb') as source:
        if hashlib.file_digest(source, 'sha256').hexdigest() != IMAGE_SHA256:
            raise ValueError('original memory image SHA-256 mismatch')
        memory = mmap.mmap(source.fileno(), 0, access=mmap.ACCESS_READ)
        with memory:
            pe = struct.unpack_from('<I', memory, 0x3c)[0]
            optional = pe + 24
            if memory[:2] != b'MZ' or memory[pe:pe+4] != b'PE\0\0' or struct.unpack_from('<H', memory, optional)[0] != 0x20b:
                raise ValueError('expected PE32+ memory image')
            rva, size = struct.unpack_from('<II', memory, optional + 112 + 3 * 8)
            if size % 12 or rva + size > len(memory):
                raise ValueError('invalid exception directory')
            bounds = [struct.unpack_from('<III', memory, rva + i) for i in range(0, size, 12)]
            starts = [entry[0] for entry in bounds]
            if starts != sorted(starts):
                raise ValueError('exception entries are not ordered')
            args.out.mkdir(parents=True, exist_ok=False)
            functions = []
            for address in (*LOOKUP_AND_MANAGER, *ACCESSORS):
                offset = address - IMAGE_BASE
                index = bisect.bisect_right(starts, offset) - 1
                begin, end, _ = bounds[index]
                if begin != offset or not begin < end <= len(memory):
                    raise ValueError('missing exact function boundary')
                code = memory[begin:end]
                raw_path = args.out / f'{address:x}.bin'
                raw_path.write_bytes(code)
                disassembly = subprocess.check_output([
                    'objdump', '-D', '-b', 'binary', '-m', 'i386:x86-64',
                    '--adjust-vma=' + hex(address), str(raw_path)], text=True)
                asm_path = raw_path.with_suffix('.asm')
                asm_path.write_text(disassembly)
                entry = dict(address=hex(address), end=hex(IMAGE_BASE + end),
                             bytes=len(code), sha256=digest(code),
                             disassembly=str(asm_path), disassembly_sha256=digest(asm_path.read_bytes()))
                if address in ACCESSORS:
                    # This pinned accessor family passes its class literal in RDX
                    # to GetPrivateStaticClassBody, shared by all seven functions.
                    matches = list(re.finditer(b'\x48\x8d\x15....', code, re.DOTALL))
                    if len(matches) != 1 or not re.search(r'call\s+0x142d698b0\b', disassembly):
                        raise ValueError('unexpected static class accessor shape')
                    instruction = matches[0].start()
                    literal = begin + instruction + 7 + struct.unpack_from('<i', code, instruction + 3)[0]
                    if not 0 <= literal < len(memory) - 128:
                        raise ValueError('class literal out of bounds')
                    text = memory[literal:literal+128].decode('utf-16le').split('\0')[0]
                    if text != ACCESSORS[address]:
                        raise ValueError('class literal mismatch')
                    entry.update(class_name=text, literal_address=hex(IMAGE_BASE + literal),
                                 literal_utf16le_hex=memory[literal:literal+2*(len(text)+1)].hex())
                functions.append(entry)
    report = dict(schema='fortnite.frontend-context-accessors.v1',
                  input=str(args.dump.resolve()), input_sha256=IMAGE_SHA256,
                  addressing='memory image: VA = image base + file offset',
                  image_base=hex(IMAGE_BASE), functions=functions,
                  limitations=['Static class identities and bounded original code only.',
                               'Custom registry creation/lifetime and runtime GetContext remain unverified.'])
    (args.out / 'receipt.json').write_text(json.dumps(report, indent=2) + '\n')
    print(f'Verified {len(ACCESSORS)} original class identities and {len(LOOKUP_AND_MANAGER)} lookup/manager bodies: {args.out}')


if __name__ == '__main__':
    main()
