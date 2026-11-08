#!/usr/bin/env python3
"""Rank unresolved callees (no reflected name, not a known engine helper) by how many pool bodies they unlock (read-only).
Reuses build_call_pool.py's eligibility rules by exec'ing it with an env flag that records unresolved targets.
Output generated/small-pool/helpers_rank.json: [{addr, bodies(only blocker), bodies_any, captured, insns, targets_unres, hosts}].
Run from repo root: python3 -I porting/build_helper_pool.py [MAXI]"""
import collections, json, os, re, sys
os.environ['HELPER_SCAN'] = '1'
src = open('porting/build_call_pool.py').read()
src = src.replace("""        else:
            ok = False
            break
    if not ok:
        st['skip_unresolved_callee'] += 1
        continue""", """        else:
            ok = False
            UNRES.setdefault(t, set()).add(e['cls'] + '::' + e['fn'])
            ALLU.setdefault((e['cls'], e['fn']), set()).update(
                x for x in tgt if x not in HELPERS and x not in gp and x not in named and x not in hcand and not (span(idx[b[0]])[0] <= int(x, 16) <= span(idx[b[0]])[1]))
            continue_outer = True
            break
    if not ok:
        st['skip_unresolved_callee'] += 1
        continue""")
src = src.replace("pool, st = [], collections.Counter()", "UNRES, ALLU = {}, {}\npool, st = [], collections.Counter()", 1)
# stop before the pool is written
cut = src.index('# dependency filter')
ns = {'__name__': 'x'}
sys.argv = ['x'] + sys.argv[1:2]
exec(compile(src[:cut], 'build_call_pool', 'exec'), ns)
idx, ALLU, UNRES, insns, span, named, gp, HELPERS, hcand = (ns[k] for k in ('idx', 'ALLU', 'UNRES', 'insns', 'span', 'named', 'gp', 'HELPERS', 'hcand'))
only = collections.Counter()
anyc = collections.Counter()
hosts = collections.defaultdict(collections.Counter)
for (c, f), us in ALLU.items():
    for u in us:
        anyc[u] += 1
        hosts[u][c] += 1
    if len(us) == 1:
        only[next(iter(us))] += 1
rows = []
for a in anyc:
    cap = a in idx
    L = insns(idx[a]) if cap else None
    tu = None
    if L:
        ts = [m.group(1) for i in L for m in [re.fullmatch(r'(?:CALL|JMP) (0x[0-9a-f]+)', i)] if m]
        lo, hi = span(idx[a])
        tu = [t for t in ts if t not in HELPERS and t not in gp and t not in named and t not in hcand and not (lo <= int(t, 16) <= hi)]
    rows.append(dict(addr=a, only=only[a], any=anyc[a], captured=cap, insns=len(L) if L else None, targets_unres=tu,
                     hosts=hosts[a].most_common(4)))
rows.sort(key=lambda r: (-r['only'], -r['any']))
os.makedirs('porting/generated/small-pool', exist_ok=True)
json.dump(rows, open('porting/generated/small-pool/helpers_rank.json', 'w'))
print('callees', len(rows), 'bodies blocked', len(ALLU), 'captured', sum(r['captured'] for r in rows))
for n in (25, 50, 100, 200, 400):
    s = {r['addr'] for r in rows[:n]}
    print(n, 'unlock', sum(1 for us in ALLU.values() if us <= s))
ok = [r for r in rows if r['captured'] and r['insns'] <= 60 and not r['targets_unres']]
print('leaf-ready (captured, <=60 insns, no unresolved callees):', len(ok), 'of top200:', sum(1 for r in rows[:200] if r in ok))
