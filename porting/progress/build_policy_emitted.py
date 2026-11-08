#!/usr/bin/env python3
"""Ledger of in-scope functions whose real body is emitted by a generator policy (no manifest entry): porting/progress/policy_emitted.json
{"Class::Fn": {"file": generated .cpp, "sha256": hash of the exact definition text}}. progress.py credits an entry only while that text is unchanged.
Run from repo root: python3 -I porting/progress/build_policy_emitted.py"""
import glob, hashlib, json, re, sys
sys.path.insert(0, 'porting')
ins = json.load(open('porting/progress/native_functions_inscope.json'))
man = {(e['class'], e['function']) for e in json.load(open('porting/behavior/MANIFEST.json')) if e.get('status') == 'ported'}
out = {}
for c, fs in ins.items():
    paths = glob.glob(f'porting/native/Source/*/Generated/{c}.cpp')
    if not paths:
        continue
    text = open(paths[0]).read()
    for f in fs:
        if (c, f) in man or (c, f + '_Implementation') in man:
            continue
        for m in re.finditer(r'\b%s::(?:%s|%s_Implementation)\s*\([^)]*\)[^{;]*\n\{(.*?)\n\}' % (re.escape(c), re.escape(f), re.escape(f)), text, re.S):
            if 'Body not recovered' not in m.group(1):
                out[f'{c}::{f}'] = {'file': paths[0], 'sha256': hashlib.sha256(m.group(0).encode()).hexdigest()}
                break
json.dump(out, open('porting/progress/policy_emitted.json', 'w'), indent=1, sort_keys=True)
print(len(out))
