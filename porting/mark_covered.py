#!/usr/bin/env python3
"""Mark remaining functions already covered by ported `<fn>_Implementation` / `<fn>_Validate` manifest entries.
Ported RPC/native-event bodies are stored under those names, which do not match the reflected name, so they were
counted as unported. Updates generated/thunk-table/final2.json in place (fields covered_by, coverage) and writes
covered_summary.json. coverage: 'full' = `_Implementation` ported (the real body); 'partial' = only `_Validate`.
Run from repo root: python3 -I porting/mark_covered.py"""
import collections, json
R = 'porting/generated/thunk-table'
F = json.load(open(f'{R}/final2.json'))
port = {(e['class'], e['function']) for e in json.load(open('porting/behavior/MANIFEST.json'))
        if e.get('status') == 'ported'}
st = collections.Counter()
for e in F:
    hit = [s for s in ('_Implementation', '_Validate') if (e['cls'], e['fn'] + s) in port]
    e['covered_by'] = hit
    e['coverage'] = 'full' if '_Implementation' in hit else 'partial' if hit else 'none'
    st[e['coverage']] += 1
json.dump(F, open(f'{R}/final2.json', 'w'))
json.dump(dict(st), open(f'{R}/covered_summary.json', 'w'))
print(dict(st), 'truly unported (incl. partial):', st['none'] + st['partial'])
