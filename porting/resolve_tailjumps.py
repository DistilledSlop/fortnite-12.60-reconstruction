#!/usr/bin/env python3
"""Thunks that end in a direct tail JMP to the real function (frame advance, then `JMP target`).

The earlier decompile-based table (build_thunk_table.py) read the callees/virtual calls INSIDE that target's decompile
and chose a body from them, which is wrong for these thunks: the body is the jump target itself. This script finds
them from the captured thunk asm, captures missing targets with probe_callees.py, follows pure adjustor wrappers
(resolve_wrappers rules: `[ADD|SUB RCX,N;] JMP abs`) and writes generated/thunk-table/tailjmp.json:
  {"CLS::FN": {thunk, target, real, chain, this_delta, thunk_receiver_delta, prev_group, prev_real_bodies, captured}}
Used by build_work_packets.py (body candidates) and the porting guide. Run from repo root: python3 -I porting/resolve_tailjumps.py
"""
import glob, json, os, re, subprocess

R = 'porting/generated'
PY = 'porting/.venv/bin/python'


def index():
    idx = {}
    for d in glob.glob(f'{R}/native*/'):
        for f in os.listdir(d):
            m = re.fullmatch(r'(0x[0-9a-f]+)\.asm', f)
            if m:
                idx.setdefault(m.group(1), d + f)
    return idx


def insns(path):
    return [l.split(None, 1)[1].strip() for l in open(path) if len(l.split(None, 1)) > 1]


def wrapper(path):
    L = insns(path)
    if not L or len(L) > 4:
        return None
    delta = 0
    for ins in L[:-1]:
        m = re.fullmatch(r'(ADD|SUB) RCX,(0x[0-9a-f]+)', ins)
        if not m:
            return None
        delta += int(m.group(2), 16) * (1 if m.group(1) == 'ADD' else -1)
    m = re.fullmatch(r'JMP (0x[0-9a-f]+)', L[-1])
    return (m.group(1), delta) if m else None


f2 = json.load(open(f'{R}/thunk-table/final2.json'))
idx = index()
rows = {}
for e in f2:
    p = idx.get(e['thunk'])
    if not p or e['group'] == 'A':
        continue
    L = insns(p)
    m = re.fullmatch(r'JMP (0x[0-9a-f]+)', L[-1]) if L else None
    if m and len(L) <= 12 and abs(int(m.group(1), 16) - int(e['thunk'], 16)) > 0x400:
        rows[f"{e['cls']}::{e['fn']}"] = dict(thunk=e['thunk'], target=m.group(1), prev_group=e['group'],
                                              prev_real_bodies=e.get('real_bodies') or [])
print(len(rows), 'tail-jump thunks')
for rnd in range(6):
    need = set()
    for r in rows.values():
        cur, seen = r['target'], 0
        while cur in idx and seen < 8:
            w = wrapper(idx[cur])
            if not w:
                break
            cur, seen = w[0], seen + 1
        if cur not in idx:
            need.add(cur)
    if not need:
        break
    lst = f'{R}/thunk-table/tailjmp_capture_{rnd}.json'
    json.dump(sorted(need), open(lst, 'w'))
    print('capturing', len(need))
    subprocess.run([PY, '-I', 'porting/ghidra/probe_callees.py', '--input', lst, '--out', f'{R}/native-cap-tailjmp{rnd}',
                    '--timeout', '120', '--mem', '24G'], check=True, stderr=subprocess.DEVNULL)
    idx = index()
for k, r in rows.items():
    cur, chain, dl = r['target'], [], 0
    while cur in idx and len(chain) < 8:
        w = wrapper(idx[cur])
        if not w:
            break
        chain.append(cur)
        dl += w[1]
        cur = w[0]
    r.update(real=cur, chain=chain, this_delta=dl, captured=cur in idx)
json.dump(rows, open(f'{R}/thunk-table/tailjmp.json', 'w'), indent=1)
same = sum(1 for r in rows.values() if r['real'] in r['prev_real_bodies'])
print(dict(total=len(rows), real_equals_previous_choice=same, captured=sum(1 for r in rows.values() if r['captured'])))
