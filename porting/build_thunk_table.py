#!/usr/bin/env python3
"""Classify every remaining native function's exec thunk from its Ghidra decompile.

Read-only. Output (git-ignored): porting/generated/thunk-table/table.json and summary.json.
Kinds: inline (no non-helper call; thunk is the body), direct (one non-helper callee),
multi (2+ non-helper callees), virtual (call through a vtable slot), plus per-entry
callee capture status. Helpers = callees appearing in >= HELPER_MIN distinct thunks.
Run from repo root: python3 -I porting/build_thunk_table.py
"""
import collections, glob, json, os, re

HELPER_MIN = 200
root = 'porting/generated'
cap = {}
for d in glob.glob(f'{root}/native*/'):
    for f in os.listdir(d):
        m = re.fullmatch(r'(0x[0-9a-f]+)\.c', f)
        if m:
            cap.setdefault(m.group(1), d + f)
nf = json.load(open(f'{root}/reflection/native_funcs.json'))
man = json.load(open('porting/behavior/MANIFEST.json'))
ep = json.load(open('porting/progress/engine_provided_natives.json'))['verified']
epset = {(c, f) for c, v in ep.items() for f in v}
done = {(e['class'], e['function']) for e in man
        if e.get('status') == 'ported' and e.get('kind') != 'member_field'}
rem = [(c, f, d) for c, fs in nf.items() for f, d in fs.items()
       if d.get('exec_addr') and (c, f) not in done and (c, f) not in epset]


def thunk_path(c, f, a):
    p = f'{root}/decomp/{c}/{f}.c'
    return p if os.path.exists(p) else cap.get(a)


VIRT = re.compile(r'\(\*\*\(code \*\*\)\(\*\(longlong \*\)?\(?\w+\s*\+\s*(0x[0-9a-f]+|\d+)\)?\)?|\(\*\*\(code \*\*\)\(\*\w+ \+ (0x[0-9a-f]+|\d+)\)\)')
rows = []
freq = collections.Counter()
for c, f, d in rem:
    a = d['exec_addr']
    p = thunk_path(c, f, a)
    t = open(p, errors='ignore').read().split('*/', 1)[-1] if p else ''
    calls = re.findall(r'\b(FUN_[0-9a-f]+)\(', t)
    virt = [m.group(1) or m.group(2) for m in VIRT.finditer(t)]
    rows.append(dict(cls=c, fn=f, thunk=a, src=p, calls=calls, virt_slots=virt, lines=t.count('\n'),
                     nparams=len(d.get('params') or [])))
    for x in set(calls):
        freq[x] += 1
helpers = {h for h, n in freq.items() if n >= HELPER_MIN}
cnt = collections.Counter()
callee_use = collections.Counter()
for r in rows:
    me = 'FUN_' + r['thunk'][2:]
    nh = [x for x in dict.fromkeys(r['calls']) if x not in helpers and x != me]
    r['body_candidates'] = ['0x' + x[4:] for x in nh]
    if r['virt_slots']:
        r['kind'] = 'virtual'
    elif not nh:
        r['kind'] = 'inline'
    elif len(nh) == 1:
        r['kind'] = 'direct'
    else:
        r['kind'] = 'multi'
    r['body_captured'] = [a in cap for a in r['body_candidates']]
    cnt[r['kind']] += 1
    for a in r['body_candidates']:
        callee_use[a] += 1
need = [a for a, n in callee_use.items() if a not in cap]
out = 'porting/generated/thunk-table'
os.makedirs(out, exist_ok=True)
json.dump(rows, open(f'{out}/table.json', 'w'))
kc = collections.Counter()
for r in rows:
    k = r['kind']
    if k in ('direct', 'multi', 'virtual'):
        kc[(k, 'all candidates captured' if all(r['body_captured']) and r['body_candidates'] else 'needs capture')] += 1
summary = dict(remaining=len(rows), kinds=dict(cnt), helper_min=HELPER_MIN, helper_count=len(helpers),
               capture_status={f'{k[0]}/{k[1]}': v for k, v in kc.items()},
               uncaptured_candidate_callees=len(need),
               top_uncaptured=sorted(((a, callee_use[a]) for a in need), key=lambda x: -x[1])[:40])
json.dump(summary, open(f'{out}/summary.json', 'w'), indent=1)
print(json.dumps(summary)[:1500])
