#!/usr/bin/env python3
"""Audit ported manifest entries that have no recorded native_body_address (read-only).
For each, compare the body addresses named in the .inc comments with final2 real_bodies, report the receiver delta and
thunk-logic hint. Output generated/receiver-adjust/unrecorded_audit.json. Run from repo root: python3 -I porting/audit_unrecorded_bodies.py"""
import collections, json, os, re, sys
sys.path.insert(0, 'porting')
import thunk_receiver as tr
R = 'porting/generated'
f2 = {}
for e in json.load(open(f'{R}/thunk-table/final2.json')):
    f2[(e['cls'], e['fn'])] = e
man = json.load(open('porting/behavior/MANIFEST.json'))
out, st = [], collections.Counter()
for e in man:
    if e.get('status') != 'ported' or e.get('kind') not in ('exec', 'getter', 'impl', 'native_event_impl') or e.get('native_body_address'):
        continue
    fn = e['function']
    base = fn[:-len('_Implementation')] if fn.endswith('_Implementation') else fn
    row = f2.get((e['class'], base))
    rec = dict(cls=e['class'], fn=fn, inc=e.get('inc_path'))
    if not row:
        rec['verdict'] = 'no_table_row'
    else:
        bodies = row.get('real_bodies') or []
        txt = ''
        p = e.get('inc_path')
        if p and os.path.exists(p):
            txt = open(p, errors='ignore').read() + ' ' + (e.get('notes') or '')
        named = set(re.findall(r'0x1[0-9a-f]{8}', txt))
        rec.update(group=row['group'], bodies=bodies)
        if bodies and set(bodies) & named:
            rec['verdict'] = 'body_named_matches'
        elif not named:
            rec['verdict'] = 'no_address_named'
        else:
            rec['verdict'] = 'named_differs'
            rec['named'] = sorted(named)[:4]
        if len(bodies) == 1:
            ri = tr.receiver_info(row['thunk'], bodies[0])
            if ri:
                wd = sum(r['this_delta'] for r in row['resolved'] if r['real'] == bodies[0])
                rec['delta'] = ri['delta'] + wd
                rec['logic_hint'] = bool(ri.get('logic_hint'))
    st[rec['verdict'] + ('+delta' if rec.get('delta') else '') + ('+logic' if rec.get('logic_hint') else '')] += 1
    out.append(rec)
os.makedirs(f'{R}/receiver-adjust', exist_ok=True)
json.dump(out, open(f'{R}/receiver-adjust/unrecorded_audit.json', 'w'), indent=1)
print(len(out)); [print(' ', k, v) for k, v in st.most_common()]
