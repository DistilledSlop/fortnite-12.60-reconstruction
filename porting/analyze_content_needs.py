#!/usr/bin/env python3
"""Join content native references with the unported native function set (read-only).
Usage: python3 -I porting/analyze_content_needs.py PKGS.jsonl OUT.json
PKGS.jsonl comes from scan_content_native_refs.py. Reports, for the in-scope unported functions: how many cooked content
references, how many are reachable from the root packages by hard imports, and the kind of those never referenced
(network RPC, OnRep, native event, other). Needs generated/remaining/classification.json (classify_remaining.py)."""
import collections, json, sys
pk = {}
for l in open(sys.argv[1]):
    r = json.loads(l)
    pk[r['pkg']] = r
nf = json.load(open('porting/generated/reflection/native_funcs.json'))
unp = {}
for g, v in json.load(open('porting/generated/remaining/classification.json')).items():
    for x in v:
        unp[tuple(x.split('::'))] = g
ROOTS = ['/Game/Maps/Frontend', '/Game/Athena/Athena_GameMode', '/Game/Athena/Athena_PlayerController',
         '/Game/Athena/PlayerPawn_Athena', '/Game/Athena/Athena_GameState'] + [p for p in pk if p.startswith('/Game/Athena/Apollo/Maps/Apollo_Terrain')]
seen = {r for r in ROOTS if r in pk}
st = list(seen)
while st:
    for d in pk[st.pop()]['deps']:
        if d in pk and d not in seen:
            seen.add(d)
            st.append(d)


def key(c, f):
    for pre in 'AUFI':
        if (pre + c, f) in unp:
            return (pre + c, f)


allref = {key(c, f) for p in pk.values() for m, c, f in p['nat']} - {None}
closure = {key(c, f) for p in seen for m, c, f in pk[p]['nat']} - {None}


def kind(k):
    d = nf.get(k[0], {}).get(k[1], {})
    fl = d.get('flags')
    fl = fl if isinstance(fl, int) else int(fl, 16) if fl else 0
    if fl & 0x40:
        return 'network RPC (called by the network layer)'
    if k[1].startswith('OnRep_'):
        return 'OnRep (called by replication)'
    if fl & 0x08000000:
        return 'BlueprintImplementableEvent/NativeEvent (called from C++ or BP)'
    if fl & 0x04000000:
        return 'BlueprintCallable, no cooked reference (C++ callers only)'
    return 'native without Blueprint exposure (C++ callers only)'


out = dict(unported=len(unp), referenced_by_any_content=len(set(unp) & allref), reachable_from_roots=len(set(unp) & closure),
           roots=len(ROOTS), closure_packages=len(seen))
rest = collections.Counter(kind(k) for k in unp if k not in allref)
out['never_referenced_by_kind'] = dict(rest)
out['referenced_by_group'] = dict(collections.Counter(unp[k].split(':')[0] for k in allref if k in unp))
json.dump(out, open(sys.argv[2], 'w'), indent=1)
print(json.dumps(out, indent=1))
