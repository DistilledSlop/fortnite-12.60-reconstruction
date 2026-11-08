#!/usr/bin/env python3
"""Ported vs unported native functions that cooked content (from the frontend + battle royale roots, hard+soft refs) uses, by area.
UI = class generated in FortniteUI/CommonUI; BR = Athena-named or other FortniteGame classes; run: python3 -I porting/ported_by_area.py pkgs2.jsonl"""
import collections, glob, json, os, sys
pk = {}
for l in open(sys.argv[1]):
    r = json.loads(l); pk[r['pkg']] = r
ins = json.load(open('porting/progress/native_functions_inscope.json'))
unp = {tuple(x.split('::')) for v in json.load(open('porting/generated/remaining/classification.json')).values() for x in v}
mod = {os.path.basename(p)[:-2]: p.split('/')[3] for p in glob.glob('porting/native/Source/*/Generated/*.h')}
ROOTS = ['/Game/Maps/Frontend', '/Game/Athena/Athena_GameMode', '/Game/Athena/Athena_PlayerController', '/Game/Athena/PlayerPawn_Athena',
         '/Game/Athena/Athena_GameState'] + [p for p in pk if p.startswith('/Game/Athena/Apollo/Maps/Apollo_Terrain')]
def closure(roots, soft=True):
    seen = {r for r in roots if r in pk}; st = list(seen)
    while st:
        r = pk[st.pop()]
        for d in r['deps'] + (r.get('soft', []) if soft else []):
            if d in pk and d not in seen: seen.add(d); st.append(d)
    return seen
def natives(pkgs):
    out = set()
    for p in pkgs:
        for m, c, f in pk[p]['nat']:
            for pre in 'AUFI':
                if (pre + c) in ins and f in ins[pre + c]: out.add((pre + c, f)); break
    return out
allin = {(c, f) for c, fs in ins.items() for f in fs}
res = {}
for name, roots in (('battle royale (Athena roots)', [r for r in ROOTS if 'Athena' in r or 'Apollo' in r]), ('frontend/UI root', ['/Game/Maps/Frontend'])):
    n = natives(closure(roots))
    res[name] = n
only_br = res['battle royale (Athena roots)'] - res['frontend/UI root']
only_ui = res['frontend/UI root'] - res['battle royale (Athena roots)']
both = res['battle royale (Athena roots)'] & res['frontend/UI root']
def line(label, s):
    p = sum(1 for k in s if k not in unp)
    print(f'{label:46s} used {len(s):6d}  ported {p:5d}  unported {len(s)-p:5d}  ({100*p/max(1,len(s)):.1f}% ported)')
for label, s in (('BR content only', only_br), ('Frontend/UI content only', only_ui), ('used by both', both), ('either (BR or UI)', only_br | only_ui | both)):
    line(label, s)
    ui = {k for k in s if mod.get(k[0]) in ('FortniteUI', 'CommonUI')}
    line('   of which UI-module classes', ui)
    line('   of which Athena-named classes', {k for k in s if 'Athena' in k[0]})
print('in-scope total', len(allin), 'ported', len(allin - unp))
