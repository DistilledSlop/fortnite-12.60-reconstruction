#!/usr/bin/env python3
"""Extract only the fully matched MSVC registration family; fail on reflected-size conflicts."""
import collections
import hashlib
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, '/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
from capstone.x86_const import X86_OP_MEM, X86_REG_RIP

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / 'porting'))
from gen_native_classes import split_properties

IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
SHA256 = '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
BASE = 0x140000000
HELPER = 0x142d698b0
REFL = ROOT / 'porting/generated/reflection'
load = lambda p: json.loads(p.read_text())

# Entire fall-through registration body, up to and including the common return.
PATTERN = [
 r'mov r11, rsp', r'sub rsp, 0x88', r'mov rax, qword ptr \[rip [+-] 0x[0-9a-f]+\]',
 r'test rax, rax', r'jne 0x[0-9a-f]+', r'xor ecx, ecx',
 r'lea rax, \[rip [+-] 0x[0-9a-f]+\]', r'mov qword ptr \[r11 - 0x10\], rcx',
 r'lea r9, \[rip [+-] 0x[0-9a-f]+\]', r'mov byte ptr \[rsp \+ 0x70\], cl',
 r'lea r8, \[rip [+-] 0x[0-9a-f]+\]', r'mov qword ptr \[r11 - 0x20\], rax',
 r'lea rdx, \[rip [+-] 0x[0-9a-f]+\]', r'lea rax, \[rip [+-] 0x[0-9a-f]+\]',
 r'mov qword ptr \[r11 - 0x28\], rax', r'lea rax, \[rip [+-] 0x[0-9a-f]+\]',
 r'mov qword ptr \[r11 - 0x30\], rax', r'lea rax, \[rip [+-] 0x[0-9a-f]+\]',
 r'mov qword ptr \[r11 - 0x38\], rax', r'lea rax, \[rip [+-] 0x[0-9a-f]+\]',
 r'mov qword ptr \[r11 - 0x40\], rax', r'lea rax, \[rip [+-] 0x[0-9a-f]+\]',
 r'mov qword ptr \[r11 - 0x48\], rax', r'mov qword ptr \[r11 - 0x50\], rcx',
 r'lea rcx, \[rip [+-] 0x[0-9a-f]+\]', r'mov dword ptr \[rsp \+ 0x30\], 0x[0-9a-f]+',
 r'mov dword ptr \[rsp \+ 0x28\], (?:0x[0-9a-f]+|[0-9]+)',
 r'mov dword ptr \[rsp \+ 0x20\], (?:0x[0-9a-f]+|[0-9]+)',
 r'call 0x142d698b0', r'mov rax, qword ptr \[rip [+-] 0x[0-9a-f]+\]',
 r'add rsp, 0x88', r'ret']


def extract(image, md, va, name, package):
    off = va - BASE
    if off < 0 or off + 256 > len(image): return None
    ins = list(md.disasm(image[off:off + 256], va, count=len(PATTERN)))
    if len(ins) != len(PATTERN) or any(not re.fullmatch(p, i.mnemonic + (' ' + i.op_str if i.op_str else '')) for p, i in zip(PATTERN, ins)): return None
    def target(i):
        m = next((o.mem for o in i.operands if o.type == X86_OP_MEM and o.mem.base == X86_REG_RIP), None)
        return i.address + i.size + m.disp if m else None
    def wide(addr):
        pos = addr - BASE
        if pos < 0 or pos + 512 > len(image): return None
        b = image[pos:pos + 512]
        for i in range(0, len(b), 2):
            if b[i:i+2] == b'\0\0':
                try: return b[:i].decode('utf-16le')
                except UnicodeDecodeError: return None
        return None
    if int(ins[4].op_str, 0) != ins[30].address: return None
    if target(ins[2]) != target(ins[10]) or target(ins[2]) != target(ins[29]): return None
    if wide(target(ins[12])) != name[1:] or wide(target(ins[24])) != package: return None
    size = int(ins[27].op_str.rsplit(', ', 1)[1], 0)
    if not 0 < size <= 0xffffffff: return None
    return {'size': size, 'gpsc': hex(va), 'call': hex(ins[28].address),
            'size_store': hex(ins[27].address), 'package': package,
            'instructions': [f'{i.address:#x}: {i.mnemonic} {i.op_str}'.rstrip() for i in ins]}


def main():
    out = Path('/tmp/gameplay-native-class-size/report.json')
    out.unlink(missing_ok=True)  # A failed run must not leave an apparently current report.
    image = IMAGE.read_bytes()
    if hashlib.sha256(image).hexdigest() != SHA256: raise SystemExit('pinned image hash mismatch')
    index = load(REFL / 'native_class_index.json')
    parents = load(REFL / 'class_parent_map.json')
    names = load(REFL / 'class_name_map.json')
    blocks = {b['block_va'].lower(): b for b in load(REFL / 'fortnite_game_reflection.json')['classes_in_groups_with_fort_names']}
    targets = collections.Counter(g.lower() for v in index.values() for g in v.get('gpsc_targets', []))
    md = Cs(CS_ARCH_X86, CS_MODE_64); md.detail = True
    proven = {}; rejects = collections.Counter(); conflicts = []
    for name, entry in sorted(index.items()):
        gs = entry.get('gpsc_targets') or []
        if len(gs) != 1 or targets[gs[0].lower()] != 1: rejects['nonunique_target'] += 1; continue
        block = entry.get('block_va', '').lower(); parent = parents.get(block) or {}
        if names.get(block) != name or parent.get('name') != name or not parent.get('package') or block not in blocks:
            rejects['identity'] += 1; continue
        result = extract(image, md, int(gs[0], 0), name, parent['package'])
        if result is None: rejects['instruction_family'] += 1; continue
        pairs, _ = split_properties(blocks[block].get('properties', []))
        reflected = sorted({p['size_of_outer'] for p, _ in pairs if p.get('kind') == 'Bool' and isinstance(p.get('size_of_outer'), int) and p['size_of_outer'] > 0})
        if len(reflected) > 1 or reflected and reflected[0] != result['size']:
            conflicts.append({'class': name, 'block': block, 'size': result['size'], 'bool_sizes': reflected}); continue
        result.update(block=block, bool_sizes=reflected)
        proven[name] = result
    if conflicts:
        raise SystemExit('Bool size invariant conflict: ' + json.dumps(conflicts, sort_keys=True))
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps({'image_sha256': SHA256, 'helper': hex(HELPER), 'proven': proven, 'rejected': dict(rejects)}, indent=2) + '\n')
    print(json.dumps({'report': str(out), 'proven': len(proven), 'rejected': rejects}))

if __name__ == '__main__': main()
