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


seen_soft = {r for r in ROOTS if r in pk}
st = list(seen_soft)
while st:
    r = pk[st.pop()]
    for d in r['deps'] + r.get('soft', []):
        if d in pk and d not in seen_soft:
            seen_soft.add(d)
            st.append(d)


def key(c, f):
    for pre in 'AUFI':
        if (pre + c, f) in unp:
            return (pre + c, f)


allref = {key(c, f) for p in pk.values() for m, c, f in p['nat']} - {None}
closure = {key(c, f) for p in seen for m, c, f in pk[p]['nat']} - {None}
closure_soft = {key(c, f) for p in seen_soft for m, c, f in pk[p]['nat']} - {None}


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


out = dict(reachable_from_roots_with_soft_refs=len(set(unp) & closure_soft), soft_closure_packages=len(seen_soft), unported=len(unp), referenced_by_any_content=len(set(unp) & allref), reachable_from_roots=len(set(unp) & closure),
           roots=len(ROOTS), closure_packages=len(seen))
rest = collections.Counter(kind(k) for k in unp if k not in allref)
out['never_referenced_by_kind'] = dict(rest)
out['referenced_by_group'] = dict(collections.Counter(unp[k].split(':')[0] for k in allref if k in unp))
json.dump(out, open(sys.argv[2], 'w'), indent=1)
print(json.dumps(out, indent=1))


# call-graph pass: direct callees of needed functions (tier B and C) that are themselves unported
import glob, os, re
R = 'porting/generated'
idx = {}
for d in glob.glob(f'{R}/native*/'):
    for f in os.listdir(d):
        m = re.fullmatch(r'(0x[0-9a-f]+)\.asm', f)
        if m:
            idx.setdefault(m.group(1), d + f)
f2 = json.load(open(f'{R}/thunk-table/final2.json'))
named = collections.defaultdict(set)
body_of = {}
for e in f2:
    k = (e['cls'], e['fn'])
    for b in e.get('real_bodies') or []:
        named[b].add(k)
    body_of[k] = e.get('real_bodies') or []
need = {k for k in unp if k in allref or kind(k).startswith(('network', 'OnRep', 'BlueprintImplementable'))}
cand = set(need)
indirect_unknown = 0
frontier = list(need)
while frontier:
    k = frontier.pop()
    for b in body_of.get(k, []):
        if b not in idx:
            continue
        for l in open(idx[b]):
            parts = l.split(None, 1)
            if len(parts) < 2:
                continue
            ins = parts[1].strip()
            m = re.fullmatch(r'(?:CALL|JMP) (0x[0-9a-f]+)', ins)
            if m:
                for c in named.get(m.group(1), ()):
                    if c in unp and c not in need:
                        need.add(c)
                        frontier.append(c)
            elif re.match(r'(CALL|JMP) (qword ptr|R)', ins):
                indirect_unknown += 1
out['call_graph'] = dict(candidates_before=len(cand), needed_after_direct_callees=len(need),
                         added_by_call_graph=len(need) - len(cand), not_needed_by_this_analysis=len(unp) - len(need),
                         indirect_calls_in_needed_bodies_unresolved=indirect_unknown)
out['needed_by_group'] = dict(collections.Counter(unp[k].split(':')[0] for k in need))
json.dump(out, open(sys.argv[2], 'w'), indent=1)
print(json.dumps({k: v for k, v in out.items() if k in ('call_graph', 'reachable_from_roots_with_soft_refs', 'soft_closure_packages')}, indent=1))
