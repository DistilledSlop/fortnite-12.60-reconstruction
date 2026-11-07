#!/usr/bin/env python3
"""Final per-function body-location table for all remaining native functions.
Inputs: generated/thunk-table/{table,virtual_targets}.json (build_thunk_table.py, resolve_virtual_targets.py).
Output: generated/thunk-table/final.json (+ final_summary.json). Run from repo root.
Groups: A inline (thunk is the body), B direct (callee list), C virtual (resolved targets),
U unresolved (listed with reason). Virtual calls on OTHER objects do not make a thunk 'virtual'."""
import collections, glob, json, os, re
R = 'porting/generated'
have = set()
for d in glob.glob(f'{R}/native*/'):
    for f in os.listdir(d):
        m = re.fullmatch(r'(0x[0-9a-f]+)\.c', f)
        if m:
            have.add(m.group(1))
rows = json.load(open(f'{R}/thunk-table/table.json'))
vt = {(o['cls'], o['fn']): o for o in json.load(open(f'{R}/thunk-table/virtual_targets.json'))}
out, st = [], collections.Counter()
for r in rows:
    e = dict(cls=r['cls'], fn=r['fn'], thunk=r['thunk'])
    k = r['kind']
    if k == 'virtual':
        o = vt[(r['cls'], r['fn'])]
        res = [s for s in o['sites'] if s['status'] == 'resolved']
        if res:
            e.update(group='C', targets=sorted({s['target'] for s in res}), confidence=min(
                (s['confidence'] for s in res), key=['high', 'medium', 'low'].index),
                also_direct=r['body_candidates'])
        elif r['body_candidates']:
            k = 'direct'
        else:
            bad = {s['status'] for s in o['sites']} - {'other_object'}
            if bad:
                why = ','.join(sorted(bad))
                if r['cls'].endswith('Interface'):
                    why = 'interface dispatch: no single body; implemented by the implementing classes'
                elif 'bad_read' in bad:
                    why = 'pure-virtual slot on a base-only vtable: abstract here, bodies live in subclasses'
                elif 'secondary_vptr_unknown' in bad:
                    why = 'virtual call on an embedded subobject (vtable of the member type)'
                e.update(group='U', reason=why)
            else:
                k = 'inline'
    if 'group' not in e:
        if k == 'inline':
            e.update(group='A', note='thunk decompile is the body (may call other objects virtually)')
        elif k in ('direct', 'multi'):
            e.update(group='B', callees=r['body_candidates'])
        elif k == 'virtual':
            pass
    e['all_captured'] = all(a in have for a in (e.get('targets') or e.get('callees') or []))
    e['group_key'] = e.get('group')
    out.append(e)
    st[(e.get('group'), e['all_captured'])] += 1
json.dump(out, open(f'{R}/thunk-table/final.json', 'w'))
summ = {f'{g}/captured={c}': n for (g, c), n in sorted(st.items(), key=str)}
summ['total'] = len(out)
summ['unresolved'] = [(e['cls'], e['fn'], e['reason']) for e in out if e.get('group') == 'U']
json.dump(summ, open(f'{R}/thunk-table/final_summary.json', 'w'), indent=1)
print({k: v for k, v in summ.items() if k != 'unresolved'}, len(summ['unresolved']))
