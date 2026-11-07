#!/usr/bin/env python3
"""Write the tracked inputs the dashboard needs for the native-body leaf.

  porting/progress/native_functions_inscope.json  {Class: [fn, ...]}  reflected functions with an exec address,
      minus those the stock UE 4.25 source provides (the denominator). Lets the dashboard count exactly, from Git.
  porting/progress/native_workflow.json  small summary of the body-location table and tiers (from the local
      generated/thunk-table files; omitted keys if those are absent).
Run from the repo root after refreshing the thunk table: python3 -I porting/progress/native_status.py
"""
import collections, json, os

nf = json.load(open('porting/generated/reflection/native_funcs.json'))
ep = json.load(open('porting/progress/engine_provided_natives.json')).get('verified', {})
inscope = {}
for c in sorted(nf):
    skip = set(ep.get(c, ()))
    fns = sorted(f for f, m in nf[c].items() if m.get('exec_addr') and f not in skip)
    if fns:
        inscope[c] = fns
json.dump(inscope, open('porting/progress/native_functions_inscope.json', 'w'), separators=(',', ':'))
print('in scope', sum(len(v) for v in inscope.values()))

wf = {}
p = 'porting/generated/thunk-table/final2.json'
if os.path.exists(p):
    F = json.load(open(p))
    T = {(r['cls'], r['fn']): r['kind'] for r in json.load(open('porting/generated/thunk-table/table.json'))}
    g = collections.Counter(e['group'] for e in F)
    cov = collections.Counter(e.get('coverage', 'none') for e in F)
    b1 = sum(1 for e in F if e['group'] == 'B' and len(e['callees']) == 1)
    bm = sum(1 for e in F if e['group'] == 'B' and len(e['callees']) > 1)
    wf = dict(
        table_size=len(F),
        covered_by_implementation=cov['full'], validate_only=cov['partial'],
        body_location={'A_inline': g['A'], 'B_direct_single_callee': b1, 'B_direct_multi_candidate': bm,
                       'C_virtual_target': g['C'], 'U_no_single_body': g['U'],
                       'I_interface_dispatch': g['I']},
        distinct_real_bodies=len({a for e in F for a in e['real_bodies']}),
        note='Body locations come from Ghidra thunk decompiles, ctor-derived vtables and wrapper resolution; '
             'see porting/NATIVE_PORTING.md. Group B single-callee is about 93% precise and group C 90% on a '
             'ported-function ground-truth sample; multi-candidate rows need per-function review.')
json.dump(wf, open('porting/progress/native_workflow.json', 'w'), indent=1)
print(wf)
