#!/usr/bin/env python3
"""One-shot: move the hand-written UFortKismetLibrary functions (FortniteMapClasses.cpp) into manifest entries + .inc bodies and
delete the hand-written class, so the generator owns it. Writes porting/behavior/UFortKismetLibrary/<Fn>.inc and
generated/kismet_entries.json. Does not edit the manifest. Run from repo root."""
import json, os, re
CPP = 'porting/native/Source/FortniteGame/Private/FortniteMapClasses.cpp'
HDR = 'porting/native/Source/FortniteGame/Public/FortniteMapClasses.h'
src = open(CPP).read()
f3 = {(e['cls'], e['fn']): e for e in json.load(open('porting/generated/thunk-table/final3.json'))}
nf = json.load(open('porting/generated/reflection/native_funcs.json'))['UFortKismetLibrary']
out, spans = [], []
for m in re.finditer(r'(?m)^[A-Za-z_][\w:<>\*& ]*? UFortKismetLibrary::(\w+)\s*\(', src):
    name = m.group(1)
    i = src.index('{', src.index(')', m.end()))  # first brace after the parameter list
    depth, j = 0, i
    while True:
        depth += src[j] == '{'
        depth -= src[j] == '}'
        j += 1
        if depth == 0:
            break
    body = src[i + 1:j - 1].strip('\n')
    spans.append((m.start(), j))
    e = f3.get(('UFortKismetLibrary', name)) or {}
    os.makedirs('porting/behavior/UFortKismetLibrary', exist_ok=True)
    inc = f'porting/behavior/UFortKismetLibrary/{name}.inc'
    lines = [l[4:] if l.startswith('    ') else l for l in body.split('\n')]
    txt = '\n'.join(lines).strip('\n')
    if '// CONFIDENCE' not in txt:
        txt = '// CONFIDENCE: MEDIUM\n// Migrated from the hand-written UFortKismetLibrary in FortniteMapClasses.cpp.\n' + txt
    open(inc, 'w').write(txt + '\n')
    ent = {'class': 'UFortKismetLibrary', 'function': name, 'kind': 'exec', 'signature': '@SIG@', 'signature_status': 'proposed',
           'inc_path': inc, 'confidence': 'medium', 'address': e.get('thunk'), 'status': 'ported',
           'reflection_flags': nf[name]['flags'] if name in nf else None, 'static': True, 'access': 'public',
           'native_body_address': (e.get('real_bodies') or [None])[0]}
    out.append(ent)
json.dump(out, open('porting/generated/kismet_entries.json', 'w'), indent=1)
print(len(out), [e['function'] for e in out][:5], sum(1 for e in out if not e['address']))
