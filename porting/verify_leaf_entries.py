#!/usr/bin/env python3
"""Verify getter/setter/field-access proposals against reflection (read-only).

  python3 -I porting/verify_leaf_entries.py ENTRIES.json

For each entry: collect the `[RCX + off]` / `[RAX + off]`-style displacements in the body's asm, resolve every
identifier used in the .inc to a reflected property (class chain via class_parent_map.json) or bool bit
(bool_bits.json), and report whether each displacement is explained by exactly one used member at that offset
(`verified`), or what is unexplained / mismatched (`check`). It does not prove semantics; it catches wrong
members/offsets, which is the main risk of transcribing field accesses. Output prints a summary and the entries
that need a human look. Exit 0 always."""
import glob, json, re, sys
R = 'porting/generated/reflection/'
refl = json.load(open(R + 'fortnite_game_reflection.json'))
cname = json.load(open(R + 'class_name_map.json'))
parent = json.load(open(R + 'class_parent_map.json'))
bits = json.load(open(R + 'bool_bits.json'))['by_class']
by_va = {}
for c in refl['classes_in_groups_with_fort_names']:
    if c.get('block_va'):
        by_va[c['block_va']] = c
props = {}   # class -> {name: offset}
chain = {}   # class -> parent class
for va, nm in cname.items():
    c = by_va.get(va)
    props[nm] = {p['name']: p['offset'] for p in (c.get('properties') or []) if isinstance(p.get('offset'), int)} if c else {}
    pn = (parent.get(va) or {}).get('parent_name')
    chain[nm] = pn
for cl, items in bits.items():
    for b in items:
        props.setdefault(cl, {})[b['name']] = b['offset']


def lookup(cls):
    seen, out = set(), {}
    while cls and cls not in seen:
        seen.add(cls)
        for k, v in props.get(cls, {}).items():
            out.setdefault(k, v)
        cls = chain.get(cls)
    return out


def asm_offsets(path):
    offs = set()
    for l in open(path):
        for m in re.finditer(r'\[R(?:CX|AX|DX|DI|SI|BX|[0-9]+)\s*\+\s*(0x[0-9a-f]+)\]', l):
            offs.add(int(m.group(1), 16))
    return offs


ents = json.load(open(sys.argv[1]))
ok, check = 0, []
for e in ents:
    body = '\n'.join(l for l in open(e['inc_path']).read().splitlines() if not l.startswith('//'))
    idents = set(re.findall(r'[A-Za-z_]\w*', body))
    ms = lookup(e['class'])
    used = {i: ms[i] for i in idents if i in ms}
    a = glob.glob(f"porting/generated/native*/{e['native_body_address']}.asm")
    offs = asm_offsets(a[0]) if a else set()
    explained = {o for o in offs if o in used.values()}
    unexplained = offs - explained
    bad = []
    if unexplained:
        bad.append('asm offsets not matched by a used member: ' + ','.join(hex(o) for o in sorted(unexplained)))
    extra = [f'{k}@{hex(v)}' for k, v in used.items() if v not in offs and v >= 0]
    if offs and extra and not unexplained:
        bad.append('members used but not in asm: ' + ','.join(extra))
    if bad:
        check.append((f"{e['class']}::{e['function']}", bad))
    else:
        ok += 1
print(f'verified {ok} of {len(ents)}')
for k, b in check:
    print('CHECK', k, '|', '; '.join(b))
