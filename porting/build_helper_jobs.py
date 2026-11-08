#!/usr/bin/env python3
"""Build helper-port job batches from helpers_rank.json (read-only apart from generated/work-batches/PREFIXn.json).
Leaf-ready = captured, <= MAXI insns, every call target resolved. Python3 -I porting/build_helper_jobs.py PREFIX N SIZE [MAXI] [SKIPFILE]"""
import collections, glob, json, os, re, sys
prefix, n, size = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
MAXI = int(sys.argv[4]) if len(sys.argv) > 4 else 60
skip = set(json.load(open(sys.argv[5]))) if len(sys.argv) > 5 else set()
R = 'porting/generated'
ns = {'__name__': 'x'}
_argv = sys.argv; sys.argv = ['x']
src = open('porting/build_call_pool.py').read()
exec(compile(src[:src.index('pool, st = [], collections.Counter()')], 'bcp', 'exec'), ns)
idx, nf, named, gp, HELPERS, hcand, hdr, f2, insns = (ns[k] for k in ('idx', 'nf', 'named', 'gp', 'HELPERS', 'hcand', 'hdr', 'f2', 'insns'))
sys.argv = _argv
rank = json.load(open(f'{R}/small-pool/helpers_rank.json'))
ok = [r for r in rank if r['captured'] and r['insns'] <= MAXI and not r['targets_unres'] and r['addr'] not in skip and r['addr'] not in hcand]
# callers by helper address: reuse ALLU from the rank script via a rescan of final3 bodies
callers = collections.defaultdict(list)
for e in f2:
    b = e.get('real_bodies') or []
    if len(b) == 1 and b[0] in idx:
        for i in insns(idx[b[0]]):
            m = re.fullmatch(r'(?:CALL|JMP) (0x[0-9a-f]+)', i)
            if m:
                callers[m.group(1)].append((e['cls'], e['fn'], b[0]))
jobs = []
for r in ok:
    a = r['addr']
    hosts = [(c, k) for c, k in r['hosts'] if c in hdr]
    if not hosts:
        continue
    L = insns(idx[a])
    hint = {}
    for i in L:
        m = re.fullmatch(r'(?:CALL|JMP) (0x[0-9a-f]+)', i)
        if not m:
            continue
        t = m.group(1)
        if t in HELPERS:
            hint[t] = HELPERS[t]
        elif t in gp:
            hint[t] = [dict(static_class=c + '::StaticClass()') for c in gp[t]]
        elif t in hcand:
            hint[t] = hcand[t]
        elif t in named:
            hint[t] = [dict(fn=f'{c}::{f}', params=[(p['name'], p['kind']) for p in nf.get(c, {}).get(f, {}).get('params') or []]) for c, f in sorted(named[t])[:5]]
    cs = callers[a]
    _iat = json.load(open(f'{R}/small-pool/iat.json'))
    iatc = {m.group(1): _iat[m.group(1)] for i in L for m in [re.search(r'(?:CALL|JMP) qword ptr \[(0x14[0-9a-f]{7})\]', i)] if m and m.group(1) in _iat}
    jobs.append(dict(addr=a, host=hosts[0][0], host_header=hdr[hosts[0][0]], n_callers=r['any'], insns=L, callee_hints=hint, import_calls=iatc,
                     callers=[dict(cls=c, fn=f, body=b) for c, f, b in cs[:6]], asm=idx[a]))
os.makedirs(f'{R}/work-batches', exist_ok=True)
sel = jobs[:n]
for i in range(0, len(sel), size):
    json.dump(sel[i:i + size], open(f'{R}/work-batches/{prefix}{i // size + 1}.json', 'w'))
print('ready', len(jobs), 'selected', len(sel), 'batches', (len(sel) + size - 1) // size)
print(collections.Counter(j['host'] for j in sel).most_common(5))
