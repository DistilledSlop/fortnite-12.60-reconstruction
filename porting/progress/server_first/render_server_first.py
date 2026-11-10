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

# ---- Server-function scope for the native-bodies tile -------------------------------------------------------------------
# The dashboard scores gameplay.native_behavior over all 14,391 reflected functions. This view scores it only over the server
# functions being recovered: the evidence-based required floor (proven/strong/likely) already ported, plus the unported floor
# (porting/server_scope/local_model/queue.json) minus the functions cut as probably not battle royale (claude/cut_not_br.json).
# A function counts as done when it is ported (floor) or has a proposed source body (claude/<Class>__<Function>.inc); proposed
# bodies are not built or run, which the tile's evidence text says.
REPO = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
SS = os.path.join(REPO, 'porting', 'server_scope')
LM = os.path.join(SS, 'local_model')
_rows = [json.loads(l) for l in open(os.path.join(SS, 'functions.jsonl'))]
_floor_ported = {f"{r['c']}::{r['f']}" for r in _rows if r['v'] == 'required' and r['conf'] in ('proven', 'strong', 'likely') and r['p']}
_queue = {f"{c}::{f}" for c, f in json.load(open(os.path.join(LM, 'queue.json')))}
_cut = set(json.load(open(os.path.join(LM, 'claude', 'cut_not_br.json'))))
_target = _queue - _cut
def _proposed(fn):
    c, f = fn.split('::')
    return os.path.exists(os.path.join(LM, 'claude', f'{c}__{f}.inc'))
_prop = {fn for fn in _target if _proposed(fn)}
_cells = ([('Ported before the recovery (required floor)', fn, True) for fn in sorted(_floor_ported)]
          + [('Proposed source body (not built or run)', fn, True) for fn in sorted(_prop)]
          + [('To do', fn, False) for fn in sorted(_target - _prop)])
_done, _total = len(_floor_ported) + len(_prop), len(_floor_ported) + len(_target)
def _override(n):
    if n['kind'] == 'leaf':
        if n['id'] == 'gameplay.native_behavior':
            n.update(name='Server native functions ported or proposed', done=_done, total=_total, pct=round(100.0 * _done / _total, 3),
                     evidence=(f"{len(_floor_ported)} required-floor functions already ported plus {len(_prop)} proposed source bodies, of {_total} server "
                               f"functions in scope ({len(_target)} being recovered after {len(_cut)} cut as probably not battle royale). Proposed bodies are "
                               "written source with evidence, not built or run; verified-ported alone is " f"{len(_floor_ported)} of {_total}."))
        return
    for k in n['children']: _override(k)
    n['pct'] = round(sum(k['weight'] * k['pct'] for k in n['children']) / sum(k['weight'] for k in n['children']), 3)
for _g in groups: _override(_g)
leaves=[l for g in groups for l in P.walk_leaves(g)]
W=sum(l['weight'] for l in leaves); overall=sum(l['weight']*l['pct'] for l in leaves)/W
grids=P.build_unit_grids(model,files,{l['id']:l for l in res['leaves']})
grids['gameplay.native_behavior']=_cells
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
