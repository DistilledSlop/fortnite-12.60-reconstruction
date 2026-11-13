#!/usr/bin/env python3
"""Recover native constructor defaults of reflected classes from the decrypted 12.60 client image.

Our generated classes have no native constructors, so every primitive default the original constructor sets
(bReplicates-style flags, -1 sentinels, float tunables) is 0 on our server. Replication compares against the class
default object, so a wrong server default is either sent to the client (wrong value) or not sent (server and client
disagree). This tool follows each class from GetPrivateStaticClassBody (native_class_index.json gpsc_targets) to its
InternalConstructor argument, then to the constructor (`this = Initializer.Obj; jmp Ctor`), and records constant stores
to `this` that hit a reflected property:
  mov [this+off], imm        (int / byte / enum / bool byte / pointer-null)
  movss [this+off], xmmN     with xmmN loaded from a rip-relative float constant or zeroed
  mov [this+off], reg        with reg set to a known constant
  or/and [this+off], imm     (bitfields; matched against bool_bits.json masks)
Only the class's own constructor body is scanned (parent constructors are separate classes); a store that the
constructor later overwrites keeps the last value. Calls clobber volatile registers. Output: JSON
{class: {property: value}} for values that differ from zero, with the store address as evidence.

Usage: generated/.venv/bin/python porting/tools/ctor_defaults.py [--class AFortGameStateZone] [--out FILE]
(needs capstone; generated/.venv has it)
"""
import argparse, json, struct, sys
from pathlib import Path

import capstone
from capstone import x86

PORTING = Path(__file__).resolve().parents[1]
IMAGE = PORTING / 'generated/dump/FortniteClient-Win64-Shipping.runtime.exe'
BASE = 0x140000000
VOLATILE = {'rax', 'rcx', 'rdx', 'r8', 'r9', 'r10', 'r11'}

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


def read_float(va):
    off = va - BASE
    return struct.unpack('<f', data[off:off + 4])[0]


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
            for r in VOLATILE: regconst.pop(r, None)
            xmmconst = {k: v for k, v in xmmconst.items() if k in ('xmm6', 'xmm7', 'xmm8', 'xmm9', 'xmm10', 'xmm11', 'xmm12', 'xmm13', 'xmm14', 'xmm15')}
            continue
        if len(ops) == 2 and ops[0].type == x86.X86_OP_REG:
            dst = ins.reg_name(ops[0].reg)
            if m == 'mov' and ops[1].type == x86.X86_OP_REG:
                src = ins.reg_name(ops[1].reg)
                if src in alias: alias.add(dst)
                else: alias.discard(dst)
                if src in regconst: regconst[dst] = regconst[src]
                else: regconst.pop(dst, None)
                continue
            if m == 'mov' and ops[1].type == x86.X86_OP_IMM:
                regconst[dst] = ops[1].imm; alias.discard(dst); continue
            if m == 'xor' and ops[1].type == x86.X86_OP_REG and ops[0].reg == ops[1].reg:
                regconst[dst] = 0; alias.discard(dst); continue
            if m in ('movss', 'movd') and ops[1].type == x86.X86_OP_MEM and ops[1].mem.base == x86.X86_REG_RIP:
                xmmconst[dst] = read_float(ins.address + ins.size + ops[1].mem.disp); continue
            if m in ('xorps', 'pxor') and ops[1].type == x86.X86_OP_REG and ops[0].reg == ops[1].reg:
                xmmconst[dst] = 0.0; continue
            if m in ('movaps', 'movss') and ops[1].type == x86.X86_OP_REG:
                src = ins.reg_name(ops[1].reg)
                if src in xmmconst: xmmconst[dst] = xmmconst[src]
                else: xmmconst.pop(dst, None)
                continue
            # any other write to a register ends what we know about it
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
            elif m == 'movss' and ops[1].type == x86.X86_OP_REG and ins.reg_name(ops[1].reg) in xmmconst:
                stores.append((disp, 4, 'float', xmmconst[ins.reg_name(ops[1].reg)], ins.address))
            elif m == 'or' and ops[1].type == x86.X86_OP_IMM:
                stores.append((disp, size, 'or', ops[1].imm, ins.address))
            elif m == 'and' and ops[1].type == x86.X86_OP_IMM:
                stores.append((disp, size, 'and', ops[1].imm, ins.address))
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
                props[by_block[o['block_va']]] = o['properties']
            else:
                stack.extend(o.values())
        elif isinstance(o, list):
            stack.extend(o)
    bools = json.loads((PORTING / 'generated/reflection/bool_bits.json').read_text())['by_class']
    # A constructor also sets inherited properties (AFortWeapon sets AActor::bReplicates): give each class its chain.
    chains = {v['name']: v.get('chain') or [v['name']] for v in
              json.loads((PORTING / 'generated/reflection/class_parent_map.json').read_text()).values() if v.get('name')}
    inh_props = {c: [p for a in chains.get(c, [c]) for p in props.get(a, [])] for c in idx}
    inh_bools = {c: [b for a in chains.get(c, [c]) for b in bools.get(a, [])] for c in idx}
    return idx, inh_props, inh_bools


def resolve(cls, stores, props, bools):
    plain = {}
    for p in props.get(cls, []):
        if p.get('offset') is not None and p.get('array_inner_of') is None:
            plain.setdefault(p['offset'], p)
    bits = bools.get(cls, [])
    out = {}
    for disp, size, kind, val, addr in stores:
        if kind in ('or', 'and'):
            for b in bits:
                if b['offset'] <= disp < b['offset'] + 4 or disp <= b['offset'] < disp + size:
                    shift = (b['offset'] - disp) * 8 if disp <= b['offset'] else -(disp - b['offset']) * 8
                    mask = (b['mask'] << shift) if shift >= 0 else (b['mask'] >> -shift)
                    if kind == 'or' and val & mask:
                        out[b['name']] = {'value': True, 'at': hex(addr)}
                    elif kind == 'and' and not (val & mask) and mask:
                        out[b['name']] = {'value': False, 'at': hex(addr)}
            continue
        p = plain.get(disp)
        if p:
            k = p['kind']
            if kind == 'float' and k in ('Float',):
                v = round(val, 6)
            elif kind == 'int' and k in ('Int', 'Int8', 'Int16', 'Int64', 'UInt16', 'UInt32', 'UInt64', 'Byte', 'Enum', 'Bool'):
                bits_n = size * 8
                v = val if val < (1 << (bits_n - 1)) or k in ('Byte', 'UInt16', 'UInt32', 'UInt64') else val - (1 << bits_n)
                if k == 'Bool': v = bool(v)
            elif kind == 'int' and k == 'Float':
                v = round(struct.unpack('<f', struct.pack('<I', val & 0xffffffff))[0], 6)
            else:
                continue
            out[p['name']] = {'value': v, 'at': hex(addr)}
        else:
            # packed stores (e.g. an 8-byte immediate covering two ints, or a byte store into a bitfield byte)
            for b in bits:
                if disp <= b['offset'] < disp + size and kind == 'int':
                    byte = (val >> ((b['offset'] - disp) * 8)) & 0xff
                    out[b['name']] = {'value': bool(byte & b['mask']), 'at': hex(addr)}
    return {k: v for k, v in out.items() if v['value'] not in (0, False, 0.0)}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--class', dest='cls')
    ap.add_argument('--out')
    a = ap.parse_args()
    idx, props, bools = load_props()
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
    text = json.dumps(out, indent=1, sort_keys=True)
    if a.out:
        Path(a.out).write_text(text)
        print(json.dumps(stats))
    else:
        print(text[:4000])


if __name__ == '__main__':
    main()
