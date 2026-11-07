#!/usr/bin/env python3
"""Attribute clang errors from a build log to manifest entries and move the failing ports to a register (orchestrator tool).
Usage: python3 -I porting/build_fix_round.py ERRS.txt   (ERRS.txt = sorted unique 'Source/...:line:col: error: msg' lines)
Failing helper entries (kind helper, used by other bodies) are never removed; they are printed for manual repair."""
import collections, json, re, sys
man = json.load(open('porting/behavior/MANIFEST.json'))
by = {(e['class'], e['function']): e for e in man if e.get('status') == 'ported' and e.get('kind') != 'member_field'}
fails = collections.OrderedDict()
for l in open(sys.argv[1]):
    m = re.match(r'(\S+?):(\d+):\d+: error: (.*)', l.strip())
    if not m or not m.group(1).startswith('Source/'):
        continue
    f, n, msg = m.groups()
    src = open('porting/native/' + f).read().split('\n')
    i, fn = int(n) - 1, None
    while i >= 0:
        mm = re.match(r'^[\w:<>\*&\s,]*?\b(\w+)::(\w+)\s*\(', src[i])
        if mm and not src[i].startswith((' ', '\t')):
            fn = (mm.group(1), mm.group(2))
            break
        i -= 1
    fails.setdefault(fn, []).append(msg)
rem, keep, other = [], [], []
for e in man:
    k = (e['class'], e['function'])
    if k in fails and e.get('kind') not in ('member_field', 'helper') and e.get('status') == 'ported':
        rem.append(e)
    else:
        keep.append(e)
for k, v in fails.items():
    if k not in {(e['class'], e['function']) for e in rem}:
        other.append((k, v[0][:120]))
reg_path = 'porting/behavior/stub_bulk_runtime/build_removed_compile_errors.json'
reg = json.load(open(reg_path))
for e in rem:
    reg.append(dict(entry=f"{e['class']}::{e['function']}", kind=e['kind'], blocker='clang_compile_error',
                    reason=fails[(e['class'], e['function'])][0][:200],
                    unblock='Fix the body (declare missing recovered members, use public accessors, avoid thunk transcription) and re-add the port',
                    inc_path=e.get('inc_path')))
json.dump(keep, open('porting/behavior/MANIFEST.json', 'w'), indent=2)
json.dump(reg, open(reg_path, 'w'), indent=1)
print('removed', len(rem), 'not removable:', other)
