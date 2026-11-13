#!/usr/bin/env python3
"""Recover native constructor defaults of reflected classes from the decrypted 12.60 client image.

Our generated classes have no native constructors, so every primitive default the original constructor sets
(bReplicates-style flags, -1 sentinels, float tunables) is 0 on our server. Replication compares against the class
default object, so a wrong server default is either sent to the client (wrong value) or not sent (server and client
disagree). This tool follows each class from GetPrivateStaticClassBody (native_class_index.json gpsc_targets) to its
InternalConstructor argument, then to the constructor (`this = Initializer.Obj; jmp Ctor`), and records constant stores
to `this` that hit a reflected property:
  mov [this+off], imm        (int / byte / enum / bool byte / pointer-null)
  movss/movsd [this+off], xmmN with xmmN loaded from a rip-relative constant or zeroed
  mov [this+off], reg        with reg set to a known constant
  or/and [this+off], imm     (bitfields; matched against bool_bits.json masks)
Only the class's own constructor body is scanned (parent constructors are separate classes); a store that the
constructor later overwrites keeps the last value. Calls clobber volatile registers. Output: JSON
{class: {property: value}} for values that differ from zero, with the store address as evidence.

Usage: porting/generated/.venv/bin/python porting/tools/ctor_defaults.py [--class AFortGameStateZone]
       [--out FILE] [--emit-runtime FILE]
(runtime: scalar primitives only, restricted by reflection package to /Script/Fortnite*)
(needs capstone; generated/.venv has it)
"""
import argparse, json, math, struct
from pathlib import Path

import capstone
from capstone import x86

PORTING = Path(__file__).resolve().parents[1]
IMAGE = PORTING / 'generated/dump/FortniteClient-Win64-Shipping.runtime.exe'
BASE = 0x140000000
VOLATILE = {'rax', 'rcx', 'rdx', 'r8', 'r9', 'r10', 'r11'}
SIZES = {'Int': 4, 'Int8': 1, 'Int16': 2, 'Int64': 8, 'UInt8': 1,
         'UInt16': 2, 'UInt32': 4, 'UInt64': 8, 'Float': 4, 'Double': 8,
         'Byte': 1, 'Bool': 1}
SIGNED = {'Int', 'Int8', 'Int16', 'Int64'}
REG_FAMILY = {}
for names in [('rax', 'eax', 'ax', 'al', 'ah'), ('rbx', 'ebx', 'bx', 'bl', 'bh'),
              ('rcx', 'ecx', 'cx', 'cl', 'ch'), ('rdx', 'edx', 'dx', 'dl', 'dh'),
              ('rsi', 'esi', 'si', 'sil'), ('rdi', 'edi', 'di', 'dil'),
              ('rbp', 'ebp', 'bp', 'bpl'), ('rsp', 'esp', 'sp', 'spl')]:
    for name in names: REG_FAMILY[name] = names[0]
for number in range(8, 16):
    for suffix in ('', 'd', 'w', 'b'): REG_FAMILY[f'r{number}{suffix}'] = f'r{number}'

data = IMAGE.read_bytes()
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
md.detail = True


def insns(va, limit):
    off = va - BASE
    return md.disasm(data[off:off + limit], va)


def find_gpsc_ctor(gpsc_va):
    """Return the InternalConstructor argument passed to GetPrivateStaticClassBody (the 10th argument, a lea into a
    stack slot or register). Heuristic: the lea targets in the gpsc body, in order; the constructor is the one whose
    code starts with `mov rax,[rcx]; test rax,rax` (this = Initializer.Obj)."""
    for ins in insns(gpsc_va, 400):
        if ins.mnemonic == 'lea' and ins.operands[1].type == x86.X86_OP_MEM and ins.operands[1].mem.base == x86.X86_REG_RIP:
            target = ins.address + ins.size + ins.operands[1].mem.disp
            head = list(insns(target, 16))
            if len(head) >= 2 and head[0].mnemonic == 'mov' and head[0].op_str == 'rax, qword ptr [rcx]' and head[1].mnemonic == 'test':
                for h in insns(target, 32):
                    if h.mnemonic == 'jmp' and h.operands[0].type == x86.X86_OP_IMM:
                        return target, h.operands[0].imm
        if ins.mnemonic == 'ret':
            break
    return None, None


def read_constant(va, size):
    off = va - BASE
    return int.from_bytes(data[off:off + size], 'little')


def scan_ctor(ctor_va, limit=0x3000):
    """Constant stores to this+disp, in program order (straight-line approximation; branches are not followed
    separately, so a store inside a rarely taken branch is still reported)."""
    alias = {'rcx'}
    regconst = {}
    xmmconst = {}
    stores = []
    for ins in insns(ctor_va, limit):
        m, ops = ins.mnemonic, ins.operands
        if m == 'ret' or m == 'int3':
            break
        if m == 'jmp' and ops and ops[0].type == x86.X86_OP_IMM and not (ctor_va <= ops[0].imm < ctor_va + limit):
            break  # tail call out of the body
        if m == 'call':
            alias -= VOLATILE
            for r in list(regconst):
                if REG_FAMILY.get(r, r) in VOLATILE: regconst.pop(r)
            xmmconst = {k: v for k, v in xmmconst.items() if k in ('xmm6', 'xmm7', 'xmm8', 'xmm9', 'xmm10', 'xmm11', 'xmm12', 'xmm13', 'xmm14', 'xmm15')}
            continue
        if len(ops) == 2 and ops[0].type == x86.X86_OP_REG:
            dst = ins.reg_name(ops[0].reg)
            # Forget overlapping subregisters too: eax/ax/al writes invalidate a saved rax constant or alias.
            if ops[0].access & capstone.CS_AC_WRITE:
                family = REG_FAMILY.get(dst, dst)
                for r in list(regconst):
                    if r != dst and REG_FAMILY.get(r, r) == family: regconst.pop(r)
                if dst != family: alias.discard(family)
            if m == 'mov' and ops[1].type == x86.X86_OP_REG:
                src = ins.reg_name(ops[1].reg)
                if ops[0].size == 8 and src in alias: alias.add(dst)
                else: alias.discard(dst)
                if src in regconst: regconst[dst] = regconst[src]
                else: regconst.pop(dst, None)
                continue
            if m in ('mov', 'movabs') and ops[1].type == x86.X86_OP_IMM:
                regconst[dst] = ops[1].imm; alias.discard(dst); continue
            if m == 'xor' and ops[1].type == x86.X86_OP_REG and ops[0].reg == ops[1].reg:
                regconst[dst] = 0; alias.discard(dst); continue
            if m in ('movss', 'movsd', 'movd', 'movq') and ops[1].type == x86.X86_OP_MEM and ops[1].mem.base == x86.X86_REG_RIP:
                xmmconst[dst] = read_constant(ins.address + ins.size + ops[1].mem.disp, ops[1].size); continue
            if m in ('xorps', 'pxor') and ops[1].type == x86.X86_OP_REG and ops[0].reg == ops[1].reg:
                xmmconst[dst] = 0; continue
            if m in ('movaps', 'movapd', 'movss', 'movsd') and ops[1].type == x86.X86_OP_REG:
                src = ins.reg_name(ops[1].reg)
                if src in xmmconst: xmmconst[dst] = xmmconst[src]
                else: xmmconst.pop(dst, None)
                continue
            # Comparisons do not write their first operand.
            if ops[0].access & capstone.CS_AC_WRITE:
                alias.discard(dst); regconst.pop(dst, None); xmmconst.pop(dst, None)
            continue
        if len(ops) == 2 and ops[0].type == x86.X86_OP_MEM:
            mem = ops[0].mem
            if mem.base == 0 or mem.index != 0:
                continue
            base = ins.reg_name(mem.base)
            if base not in alias:
                continue
            disp, size = mem.disp, ops[0].size
            if m == 'mov' and ops[1].type == x86.X86_OP_IMM:
                stores.append((disp, size, 'int', ops[1].imm, ins.address))
            elif m == 'mov' and ops[1].type == x86.X86_OP_REG and ins.reg_name(ops[1].reg) in regconst:
                stores.append((disp, size, 'int', regconst[ins.reg_name(ops[1].reg)], ins.address))
            elif m in ('movss', 'movsd') and ops[1].type == x86.X86_OP_REG and ins.reg_name(ops[1].reg) in xmmconst:
                stores.append((disp, size, 'int', xmmconst[ins.reg_name(ops[1].reg)], ins.address))
            elif m == 'or' and ops[1].type == x86.X86_OP_IMM:
                stores.append((disp, size, 'or', ops[1].imm, ins.address))
            elif m == 'and' and ops[1].type == x86.X86_OP_IMM:
                stores.append((disp, size, 'and', ops[1].imm, ins.address))
        # One-operand/implicit writes (pop, inc, mul, etc.) must not leave stale constants behind.
        for reg in ins.regs_access()[1]:
            dst = ins.reg_name(reg)
            family = REG_FAMILY.get(dst, dst)
            alias.discard(family)
            for r in list(regconst):
                if REG_FAMILY.get(r, r) == family: regconst.pop(r)
            xmmconst.pop(dst, None)
    return stores


def load_props():
    """class name (AFoo/UFoo) -> list of properties with offsets, from fortnite_game_reflection.json joined by block_va."""
    idx = json.loads((PORTING / 'generated/reflection/native_class_index.json').read_text())
    by_block = {v.get('block_va'): k for k, v in idx.items() if isinstance(v, dict)}
    refl = json.loads((PORTING / 'generated/reflection/fortnite_game_reflection.json').read_text())
    props = {}
    stack = [refl]
    while stack:
        o = stack.pop()
        if isinstance(o, dict):
            if isinstance(o.get('properties'), list) and o.get('block_va') in by_block:
                # Enum storage is given by its separate UnderlyingType record, not by the store width.
                underlying = {a['name']: b['kind'] for a, b in zip(o['properties'], o['properties'][1:])
                              if a.get('kind') == 'Enum' and b.get('name') == 'UnderlyingType'}
                props[by_block[o['block_va']]] = [dict(p, storage_kind=underlying.get(p.get('name'), p.get('kind')))
                                                 for p in o['properties']]
            else:
                stack.extend(o.values())
        elif isinstance(o, list):
            stack.extend(o)
    bools = json.loads((PORTING / 'generated/reflection/bool_bits.json').read_text())['by_class']
    # A constructor also sets inherited properties (AFortWeapon sets AActor::bReplicates): give each class its chain.
    parents = json.loads((PORTING / 'generated/reflection/class_parent_map.json').read_text())
    chains = {v['name']: v.get('chain') or [v['name']] for v in parents.values() if v.get('name')}
    inh_props = {c: [p for a in chains.get(c, [c]) for p in props.get(a, [])] for c in idx}
    inh_bools = {c: [b for a in chains.get(c, [c]) for b in bools.get(a, [])] for c in idx}
    packages = {v['name']: v.get('package', '') for v in parents.values() if v.get('name')}
    return idx, inh_props, inh_bools, packages


def resolve(cls, stores, props, bools):
    plain = {}
    for p in props.get(cls, []):
        if (p.get('offset') is not None and not p.get('array_inner_of') and not p.get('enum_underlying_of')
                and p.get('name') != 'UnderlyingType'
                and p.get('array_dim', 1) == 1 and p.get('storage_kind', p['kind']) in SIZES):
            plain.setdefault(p['offset'], p)
    bits = bools.get(cls, [])
    out = {}
    for disp, size, kind, val, addr in stores:
        if kind in ('or', 'and'):
            for b in bits:
                if disp <= b['offset'] < disp + size:
                    mask = b['mask'] << ((b['offset'] - disp) * 8)
                    if kind == 'or' and val & mask:
                        out[b['name']] = {'value': True, 'at': hex(addr)}
                    elif kind == 'and' and not (val & mask) and mask:
                        out[b['name']] = {'value': False, 'at': hex(addr)}
            continue
        if kind == 'float':  # also accept callers with the old scanner's float store representation
            val = int.from_bytes(struct.pack('<f', val), 'little')
        # Interpret a little-endian store as bytes, splitting it across every fully covered scalar.
        # Never infer a property's size from the instruction: word stores often contain two byte enums.
        raw = (int(val) & ((1 << (size * 8)) - 1)).to_bytes(size, 'little')
        for off, p in plain.items():
            k = p.get('storage_kind', p['kind'])
            width = SIZES[k]
            if not (disp <= off and off + width <= disp + size):
                if off < disp + size and disp < off + width:
                    out.pop(p['name'], None)  # partial overwrite invalidates the earlier complete value
                continue
            part = raw[off - disp:off - disp + width]
            if k in ('Float', 'Double'):
                v = struct.unpack('<f' if k == 'Float' else '<d', part)[0]
                if not math.isfinite(v):
                    out.pop(p['name'], None)
                    continue
            else:
                v = int.from_bytes(part, 'little', signed=k in SIGNED)
                if k == 'Bool': v = bool(v)
            out[p['name']] = {'value': v, 'at': hex(addr)}
        # A packed store can cover both ordinary fields and bool bytes; resolve both independently.
        for b in bits:
            if disp <= b['offset'] < disp + size:
                byte = raw[b['offset'] - disp]
                out[b['name']] = {'value': bool(byte & b['mask']), 'at': hex(addr)}
    return {k: v for k, v in out.items() if v['value'] not in (0, False, 0.0)}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--class', dest='cls')
    ap.add_argument('--out')
    ap.add_argument('--emit-runtime', metavar='FILE', help='write compact primitive defaults for /Script/Fortnite* classes')
    a = ap.parse_args()
    idx, props, bools, packages = load_props()
    result, stats = {}, {'classes': 0, 'ctor_found': 0, 'with_defaults': 0, 'values': 0}
    for cls, v in sorted(idx.items()):
        if a.cls and cls != a.cls:
            continue
        if not isinstance(v, dict) or not v.get('gpsc_targets'):
            continue
        stats['classes'] += 1
        internal, ctor = find_gpsc_ctor(int(v['gpsc_targets'][0], 16))
        if not ctor:
            continue
        stats['ctor_found'] += 1
        vals = resolve(cls, scan_ctor(ctor), props, bools)
        if vals:
            stats['with_defaults'] += 1
            stats['values'] += len(vals)
            result[cls] = {'ctor': hex(ctor), 'defaults': vals}
    out = {'_meta': {'source': 'porting/tools/ctor_defaults.py', 'stats': stats}, 'by_class': result}
    if a.emit_runtime:
        runtime = {cls[1:]: {p: v['value'] for p, v in entry['defaults'].items()}
                   for cls, entry in result.items() if packages.get(cls, '').startswith('/Script/Fortnite')}
        Path(a.emit_runtime).write_text(json.dumps(runtime, separators=(',', ':'), sort_keys=True, allow_nan=False) + '\n')
        stats['runtime_classes'] = len(runtime)
        stats['runtime_values'] = sum(len(v) for v in runtime.values())
    if a.out:
        Path(a.out).write_text(json.dumps(out, indent=1, sort_keys=True, allow_nan=False) + '\n')
        print(json.dumps(stats))
    elif not a.emit_runtime:
        print(json.dumps(out, indent=1, sort_keys=True, allow_nan=False)[:4000])
    if a.emit_runtime and not a.out:
        print(json.dumps(stats))


if __name__ == '__main__':
    main()
