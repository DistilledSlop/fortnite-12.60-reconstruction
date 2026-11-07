#!/usr/bin/env python3
"""Replace the @PARAMS@ placeholder signatures of pending stub proposals with the signature the generator derives from
reflection (taken from its own rejection text, so the source of truth stays the generator and a wrong one stays
fail-closed). Usage (repo root): python3 -I porting/fill_signatures_from_generator.py REPORT_DIR
Patches porting/behavior/stub_bulk_runtime/entries_batch2.json and porting/behavior/MANIFEST.json in place."""
import json, re, sys
rep = json.load(open(sys.argv[1].rstrip('/') + '/behavior_splice.json'))
want = {}
for r in rep['rejections']:
    m = re.search(r"reflection-derived '(.*)'$", r['reason'])
    if m and '@PARAMS@' in r['reason']:
        want[r['entry']] = m.group(1)
def patch(path):
    data = json.load(open(path)); n = 0
    for e in data:
        k = f"{e['class']}::{e['function']}"
        if k in want and '@PARAMS@' in e.get('signature', ''):
            e['signature'] = want[k]; n += 1
    open(path, 'w').write(json.dumps(data, indent=1 if 'entries_batch2' in path else 2) + '\n')
    return n
print('signatures learned', len(want), 'patched pending', patch('porting/behavior/stub_bulk_runtime/entries_batch2.json'),
      'patched manifest', patch('porting/behavior/MANIFEST.json'))
