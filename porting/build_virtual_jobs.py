#!/usr/bin/env python3
"""Jobs for unreflected virtual slots used by pool bodies (read-only). A slot whose target body is not a reflected function is ported as a
MEMBER helper on the topmost class that has that target (safe only when no descendant overrides the slot differently).
Writes generated/work-batches/PREFIXn.json and generated/small-pool/virtual_uncaptured.json.
python3 -I porting/build_virtual_jobs.py PREFIX N SIZE [MAXI]   (env TARGETS restricts the bodies considered)"""
import collections, json, os, re, sys
sys.path.insert(0, 'porting')
prefix, n, size = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
MAXI = int(sys.argv[4]) if len(sys.argv) > 4 else 300
ns = {'__name__': 'x'}; sys.argv = ['x', '300']
src = open('porting/build_call_pool.py').read()
src = src.replace("        st['skip_virtual_slot_unnamed'] += 1\n        continue",
                  "        st['skip_virtual_slot_unnamed'] += 1\n        UNNAMED.append((e['cls'], e['fn'], [x for x in vslots if not virtual_slots.virt_name(e['cls'], x)]))\n        continue", 1)
src = src.replace("pool, st = [], collections.Counter()", "UNNAMED = []\npool, st = [], collections.Counter()", 1)
exec(compile(src[:src.index('# dependency filter')], 'b', 'exec'), ns)
vs, idx, insns, span, hdr, named, gp, HELPERS, hcand = (ns[k] for k in ('virtual_slots', 'idx', 'insns', 'span', 'hdr', 'named', 'gp', 'HELPERS', 'hcand'))
PAR = vs.PARENTS
need = collections.defaultdict(lambda: dict(callers=[], slots=set()))
for c, f, ss in ns['UNNAMED']:
    for s in ss:
        T = vs.virt_target(c, s)
        if not T or any(vs.virt_target(k, s) not in (None, T) for k, ch in PAR.items() if c in ch and k != c):
            continue  # unknown target or a descendant overrides it differently
        chain = list(reversed(PAR.get(c, [c])))  # root first
        host = next((k for k in chain if vs.virt_target(k, s) == T and k in hdr), None)
        if not host or any(h['fn'].startswith(host + '::') for h in hcand.get(T, [])):
            continue
        d = need[(host, T)]
        d['callers'].append(f'{c}::{f}'); d['slots'].add(hex(s))
unc = sorted({T for (h, T) in need if T not in idx})
json.dump(unc, open('porting/generated/small-pool/virtual_uncaptured.json', 'w'))
jobs = []
for (host, T), d in sorted(need.items(), key=lambda kv: -len(kv[1]['callers'])):
    if T not in idx:
        continue
    L = insns(idx[T])
    if len(L) > MAXI:
        continue
    jobs.append(dict(addr=T, host=host, host_header=hdr[host], member=True, slots=sorted(d['slots']), n_callers=len(d['callers']),
                     callers=[dict(cls=x.split('::')[0], fn=x.split('::')[1]) for x in d['callers'][:6]], insns=L, asm=idx[T], callee_hints={}))
    for i in L:
        m = re.fullmatch(r'(?:CALL|JMP) (0x[0-9a-f]+)', i)
        if m:
            t = m.group(1)
            jobs[-1]['callee_hints'][t] = (HELPERS[t] if t in HELPERS else [dict(static_class=c + '::StaticClass()') for c in ns['gp'][t]] if t in gp else hcand[t] if t in hcand else
                                           [dict(fn=f'{c}::{f}', params=[(p['name'], p['kind']) for p in ns['nf'].get(c, {}).get(f, {}).get('params') or []]) for c, f in sorted(named[t])[:5]] if t in named else 'UNRESOLVED')
jobs = [j for j in jobs if 'UNRESOLVED' not in json.dumps(j['callee_hints']) or True]
sel = jobs[:n]
os.makedirs('porting/generated/work-batches', exist_ok=True)
for i in range(0, len(sel), size):
    json.dump(sel[i:i + size], open(f'porting/generated/work-batches/{prefix}{i // size + 1}.json', 'w'))
print('pairs', len(need), 'uncaptured targets', len(unc), 'jobs', len(jobs), 'selected', len(sel))
