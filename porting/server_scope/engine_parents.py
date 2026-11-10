#!/usr/bin/env python3
"""Build engine class -> parent map from UE 4.25 headers (read-only). Usage: engine_parents.py UE_ROOT OUT.json"""
import json, os, re, sys
pat = re.compile(r'^\s*class\s+(?:[A-Z0-9_]+_API\s+)?([AUF][A-Za-z0-9_]+)\s*(?:final\s*)?:\s*public\s+([AUF][A-Za-z0-9_]+)', re.M)
out = {}
root = sys.argv[1]
for base in ('Engine/Source/Runtime', 'Engine/Plugins'):
    for d, _, fs in os.walk(os.path.join(root, base)):
        if '/ThirdParty' in d or '/Tests' in d:
            continue
        for f in fs:
            if f.endswith('.h'):
                try:
                    t = open(os.path.join(d, f), encoding='utf-8', errors='ignore').read()
                except OSError:
                    continue
                for m in pat.finditer(t):
                    out.setdefault(m.group(1), m.group(2))
json.dump(out, open(sys.argv[2], 'w'), indent=0, sort_keys=True)
print(len(out), 'engine classes', file=sys.stderr)
