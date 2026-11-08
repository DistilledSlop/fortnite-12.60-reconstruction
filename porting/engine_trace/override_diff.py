import sys, json, collections
sys.path.insert(0, 'porting')
import virtual_slots as vs
man = json.load(open('porting/behavior/MANIFEST.json'))
done = {(e['class'], e['function'].replace('_Implementation', '')) for e in man if e.get('status') == 'ported' and e.get('kind') != 'member_field'}
body_done = set()
for e in vs._f2:
    if (e['cls'], e['fn']) in done:
        body_done.update(e.get('real_bodies') or [])
EV = json.load(open('/tmp/claude-1000/-opt-fortnite/46cd2cbf-c73d-4053-ba19-142367ff8af0/scratchpad/engine_vt.json'))
def eng(c):
    for a in vs.PARENTS.get(c, [c]):
        if a in EV and not re.match(r'^[AU](Fort|Playspace|Hoagie|Mesh|Mcp|Fortnite)', a): return a
import re
CORE = sys.argv[1:] or ['AFortGameModeAthena', 'AFortGameStateAthena', 'AFortPlayerControllerAthena', 'AFortPlayerPawnAthena', 'AFortPlayerStateAthena']
out = {}
for c in CORE:
    chain = vs.PARENTS.get(c, [c])
    for p in chain[1:]:
        if vs.VT.get(p, {}).get('status') == 'ok': break
    else: p = None
    rows = []
    for s in range(0, 700*8, 8):
        t = vs.virt_target(c, s)
        if not t: break
        pt = vs.virt_target(p, s) if p else None
        if pt == t: continue
        real = vs._real.get(t, t)
        stub = real in vs.STUB
        nm = vs.virt_name(c, s)
        ea = eng(c); ev = EV.get(ea, []); idx = s // 8
        en = ev[idx] if idx < len(ev) else None
        rows.append(dict(slot=s, target=real, name=f'{nm[0]}::{nm[1]}' if nm else None, eng=en, slot_idx=idx, stub=stub, ported=real in body_done))
    out[c] = dict(parent=p, rows=rows)
    r = [x for x in rows if not x['stub']]
    print(c, 'parent', p, 'overrides', len(rows), 'non-stub', len(r), 'named', sum(1 for x in r if x['name'] or x['eng']), 'ported', sum(1 for x in r if x['ported']))
json.dump(out, open(sys.argv[0].rsplit('/', 1)[0] + '/override_diff.json', 'w'))
