#!/usr/bin/env python3
"""Remove a manifest entry (and its batch entries.json copy) and register it. Usage: drop_entry.py CLASS FUNCTION BLOCKER REASON [ENTRIES.json]
Register: behavior/stub_bulk_runtime/build_removed_compile_errors.json. Then re-promote with an empty batch."""
import json, sys
c, f, blocker, reason = sys.argv[1:5]
paths = ['porting/behavior/MANIFEST.json'] + sys.argv[5:]
rm = None
for p in paths:
    m = json.load(open(p))
    rm = rm or next((e for e in m if (e['class'], e['function']) == (c, f)), None)
    json.dump([e for e in m if (e['class'], e['function']) != (c, f)], open(p, 'w'), indent=2)
r = 'porting/behavior/stub_bulk_runtime/build_removed_compile_errors.json'
reg = json.load(open(r))
reg.append({'entry': f'{c}::{f}', 'kind': rm and rm['kind'], 'blocker': blocker, 'reason': reason, 'inc_path': rm and rm.get('inc_path')})
json.dump(reg, open(r, 'w'), indent=1)
