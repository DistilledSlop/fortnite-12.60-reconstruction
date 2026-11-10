#!/usr/bin/env python3
"""Server-first treemap: the port dashboard's own treemap with the components the original client already provides removed.

Usage (from the repo root): python3 -I porting/progress/server_first/render_server_first.py OUT.svg
Reads components_kept.json (which leaves stay; written by the earlier scope analysis) and reuses progress.py's measurement and
renderer, so tile weights, percents and unit squares match the dashboard. Nothing is reweighted.
"""
import sys, json, copy, os
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..'))
import progress as P
sc=json.load(open(os.path.join(HERE, 'components_kept.json')))
keep=set()
for g in sc['groups']:
    for l in g['leaves']:
        if not l['new']: keep.add(l['id'])
model=json.load(open(P.COMPONENTS))
head,subject,date=P.resolve_head()
res,_,files=P.measure_commit(model,head,subject,date,head,seeded=False,fallback_totals=P.head_totals_entries(head))
def filt(n):
    if n['kind']=='leaf': return n if n['id'] in keep else None
    kids=[k for k in (filt(c) for c in n['children']) if k]
    if not kids: return None
    w=sum(k['weight'] for k in kids)
    return dict(n,weight=w,pct=round(sum(k['weight']*k['pct'] for k in kids)/w,3),children=kids)
groups=[g for g in (filt(g) for g in res['groups']) if g]
leaves=[l for g in groups for l in P.walk_leaves(g)]
W=sum(l['weight'] for l in leaves); overall=sum(l['weight']*l['pct'] for l in leaves)/W
grids=P.build_unit_grids(model,files,{l['id']:l for l in res['leaves']})
width,height=P.STANDALONE_W,P.STANDALONE_H
header=(f'<text x="12" y="28" font-size="20" font-weight="700" class="tm-head">Server-first scope (mock-up): overall {overall:.1f}% (effort-weighted)</text>'
 f'<text x="12" y="48" font-size="12" class="tm-head" font-weight="400">Same treemap as the port dashboard at commit {head[:7]}, with client-only components removed | {len(leaves)} of {len(res["leaves"])} components</text>')
body=P.render_treemap(groups,width,height-60-28,grids=grids,prefix="s")
svg=(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" width="{width}" height="{height}" role="img" aria-label="Server-first treemap" font-family="system-ui,-apple-system,Segoe UI,sans-serif"><style>{P.TREEMAP_CSS}</style><rect class="tm-bg" x="0" y="0" width="{width}" height="{height}"/>{header}<g transform="translate(0,60)">{body}</g><text x="12" y="{height-10}" font-size="11" class="tm-head" font-weight="400">Kept: backend, protocol, server gameplay, server world, client harness. Removed: rendering, UI, audio, visuals, localization, cosmetic animation.</text></svg>')
open(sys.argv[1],'w').write(svg)
print(round(overall,2),W,len(leaves),len(res['leaves']),[ (g['name'],round(g['weight']),g['pct']) for g in groups])
tw=sum(l['weight'] for l in res['leaves']); td=sum(l['weight']*l['pct'] for l in res['leaves'])/tw
ex=[l for l in res['leaves'] if l['id'] not in keep]; ew=sum(l['weight'] for l in ex)
print('TOTAL',tw,round(td,2),'EXCL',ew,round(sum(l['weight']*l['pct'] for l in ex)/ew,1),head[:7])
