#!/usr/bin/env python3
"""Follow pure wrapper chains from the chosen body addresses to the real body (read-only, orchestrator-owned).

A body is a PURE WRAPPER if its whole asm is `[ADD|SUB RCX,N ;] JMP <abs>` (plain tail jump, optionally adjusting
`this` for a secondary base). The real body is the jump target; the chain and cumulative this-delta are recorded.
Other tiny forwarders (preset args, member loads, `MOV RAX,[RCX]; JMP [RAX+N]` virtual forwards) are kept as real
small bodies and tagged `forward_kind`. Missing jump targets are captured via probe_callees.py (iterates).
Input generated/thunk-table/final.json; output generated/thunk-table/final2.json + wrappers_summary.json.
Run from repo root: python3 -I porting/resolve_wrappers.py
"""
import collections, glob, json, os, re, subprocess

R = 'porting/generated'
PY = 'porting/.venv/bin/python'
final = json.load(open(f'{R}/thunk-table/final.json'))


def index():
    idx = {}
    for d in glob.glob(f'{R}/native*/'):
        for f in os.listdir(d):
            m = re.fullmatch(r'(0x[0-9a-f]+)\.asm', f)
            if m:
                idx.setdefault(m.group(1), d + f)
    return idx


def insns(path):
    out = []
    for l in open(path):
        l = l.rstrip()
        m = re.match(r'^[0-9a-f]+\s+(.*)$', l)
        if m:
            out.append(m.group(1))
    return out


def classify(path):
    L = insns(path)
    if not L or len(L) > 4:
        return ('real', None, 0)
    delta = 0
    for ins in L[:-1]:
        m = re.fullmatch(r'(ADD|SUB) RCX,(0x[0-9a-f]+)', ins)
        if not m:
            return ('real', None, 0)
        delta += int(m.group(2), 16) * (1 if m.group(1) == 'ADD' else -1)
    m = re.fullmatch(r'JMP (0x[0-9a-f]+)', L[-1])
    if m:
        return ('wrapper', m.group(1), delta)
    return ('real', None, 0)


def addrs_of(e):
    return e.get('callees') or e.get('targets') or []


changed = True
rounds = 0
chain = {}
while changed and rounds < 6:
    rounds += 1
    changed = False
    idx = index()
    need = set()
    for e in final:
        for a in addrs_of(e):
            cur, seen = a, 0
            while cur in idx and seen < 8:
                kind, nxt, dl = classify(idx[cur])
                if kind != 'wrapper':
                    break
                cur, seen = nxt, seen + 1
            if cur not in idx:
                need.add(cur)
    if need:
        lst = f'{R}/thunk-table/wrapper_capture_{rounds}.json'
        json.dump(sorted(need), open(lst, 'w'))
        subprocess.run([PY, '-I', 'porting/ghidra/probe_callees.py', '--input', lst, '--out',
                        f'{R}/native-cap-wrap{rounds}', '--timeout', '120', '--mem', '24G'],
                       check=True, stderr=subprocess.DEVNULL)
        changed = True

idx = index()
st = collections.Counter()
FWD = {
    'virtual_forward': re.compile(r'MOV RAX,qword ptr \[RCX\] ; JMP qword ptr \[RAX \+ 0x[0-9a-f]+\]'),
}
for e in final:
    res = []
    for a in addrs_of(e):
        cur, hops, dl_total = a, [], 0
        while cur in idx and len(hops) < 8:
            kind, nxt, dl = classify(idx[cur])
            if kind != 'wrapper':
                break
            hops.append(cur)
            dl_total += dl
            cur = nxt
        fk = None
        if cur in idx:
            norm = ' ; '.join(insns(idx[cur])[:2]) if len(insns(idx[cur])) <= 3 else ''
            if re.fullmatch(r'MOV RAX,qword ptr \[RCX\] ; JMP qword ptr \[RAX \+ 0x[0-9a-f]+\]', norm):
                fk = 'virtual_forward'
        res.append(dict(entry=a, real=cur, chain=hops, this_delta=dl_total, forward_kind=fk))
        st['wrapper_followed' if hops else 'unchanged'] += 1
        if fk:
            st['virtual_forward'] += 1
    e['resolved'] = res
    e['real_bodies'] = sorted({r['real'] for r in res})
json.dump(final, open(f'{R}/thunk-table/final2.json', 'w'))
before = len({a for e in final for a in addrs_of(e)})
after = len({a for e in final for a in e['real_bodies']})
fn_changed = sum(1 for e in final if any(r['chain'] for r in e['resolved']))
summ = dict(rounds=rounds, entry_stats=dict(st), distinct_body_addrs_before=before,
            distinct_real_bodies_after=after, functions_with_wrapper_hops=fn_changed)
json.dump(summ, open(f'{R}/thunk-table/wrappers_summary.json', 'w'), indent=1)
print(summ)
