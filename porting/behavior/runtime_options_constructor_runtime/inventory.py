#!/usr/bin/env python3
"""SHA-pinned raw constructor inventory; output is evidence, not a source lift."""
import collections, hashlib, json, sys
from pathlib import Path
sys.path[:0] = ['/tmp/common-input-capstone', 'porting/behavior']
from capstone import Cs, CS_ARCH_X86, CS_MODE_64, CS_GRP_CALL
import build_vtable_slots as vt
path = '/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'
with open(path, 'rb') as f:
    digest = hashlib.file_digest(f, 'sha256').hexdigest()
assert digest == '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
image = vt.Image(path)
lo, hi = image.func_of(0x1426eb630)
assert (lo, hi) == (0x1426eb630, 0x1426f3bed)
md = Cs(CS_ARCH_X86, CS_MODE_64); md.detail = True
ins = list(md.disasm(image.m[lo-vt.BASE:hi-vt.BASE], lo))
assert sum(i.size for i in ins) == hi-lo
calls = collections.Counter(i.op_str for i in ins if i.group(CS_GRP_CALL))
out = Path('/tmp/runtime-options-constructor'); out.mkdir(exist_ok=True)
(out/'instructions.txt').write_text(''.join(f'{i.address:#x}: {i.mnemonic} {i.op_str}\n' for i in ins))
(out/'calls.json').write_text(json.dumps({'sha256': digest, 'start':hex(lo), 'end_exclusive':hex(hi), 'instruction_count':len(ins), 'call_count':sum(calls.values()), 'targets':dict(sorted(calls.items()))}, indent=2)+'\n')
print(f'{len(ins)} instructions, {sum(calls.values())} calls, {len(calls)} distinct call operands; evidence in {out}')
