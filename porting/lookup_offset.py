#!/usr/bin/env python3
"""Which reflected member (own or inherited from a generated ancestor) covers OBJECT offset OFF of CLASS?
Usage (repo root): python3 -I porting/lookup_offset.py CLASS 0xOFF [more offsets]
Prints each reflected property of CLASS and its generated ancestors with offset in [OFF-16, OFF], so the covering
member (offset <= OFF < offset+size) and any neighbour are visible, plus the ancestors' chain and each class's lowest
own offset. Offsets here are object offsets (body-relative offset + the receiver delta the thunk/adjustor applies)."""
import json, sys
R = 'porting/generated/reflection/'
rf = json.load(open(R + 'fortnite_game_reflection.json'))
pm = json.load(open(R + 'class_parent_map.json'))
nm = json.load(open(R + 'class_name_map.json'))
props = {}
for c in rf['classes_in_groups_with_fort_names']:
    n = c.get('name') or nm.get(c['block_va'])
    props[n] = sorted((p['offset'], p['name'], p['kind'], p.get('array_dim')) for p in c.get('properties') or [] if p.get('offset') is not None)
par = {v['name']: v.get('parent_name') for v in pm.values() if isinstance(v, dict)}
cls = sys.argv[1]
chain, c = [], cls
while c:
    chain.append(c)
    c = par.get(c)
print('chain:', ' <- '.join(f"{x}(min {hex(props[x][0][0]) if props.get(x) else '-'})" for x in chain))
for a in sys.argv[2:]:
    off = int(a, 16)
    print(f'-- {a}')
    for c in chain:
        for o, n, k, d in props.get(c, []):
            if off - 16 <= o <= off:
                print(f'   {c}: +{hex(o)} {n} {k}' + (f'[{d}]' if d and d > 1 else ''))
