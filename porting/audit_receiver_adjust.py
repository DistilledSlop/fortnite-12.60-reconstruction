#!/usr/bin/env python3
"""Find exec thunks that adjust `this` (RCX) before calling the body, so body field offsets are relative to this+delta.
Read-only. For every manifest exec entry (ported, non-stub body) plus the escalation job files, parse the captured
thunk asm (porting/generated/native*/<thunk>.asm): ADD/SUB RCX,N and LEA RCX,[RCX+N] before the first CALL/JMP
accumulate into delta; MOV RCX,[RCX+N] is reported as a pointer load. Output: generated/receiver-adjust/report.json.
Run from repo root: python3 -I porting/audit_receiver_adjust.py"""
import glob, json, os, re
R = 'porting/generated'
STUB = {'0x1406801c0', '0x1406801d0', '0x1406801e0', '0x140680300'}
idx = {}
for d in glob.glob(f'{R}/native*/'):
    for f in os.listdir(d):
        m = re.fullmatch(r'(0x[0-9a-f]+)\.asm', f)
        if m:
            idx.setdefault(m.group(1), d + f)


def analyse(thunk):
    p = idx.get(thunk)
    if not p:
        return None
    delta, ptr = 0, []
    for l in open(p):
        m = re.match(r'^[0-9a-f]+\s+(.*)$', l.rstrip())
        if not m:
            continue
        i = m.group(1)
        if i.startswith(('CALL', 'JMP')):
            break
        a = re.fullmatch(r'(ADD|SUB) RCX,(0x[0-9a-f]+)', i)
        if a:
            delta += int(a.group(2), 16) * (1 if a.group(1) == 'ADD' else -1)
            continue
        a = re.fullmatch(r'LEA RCX,\[RCX \+ (-?0x[0-9a-f]+)\]', i)
        if a:
            delta += int(a.group(1), 16)
            continue
        a = re.fullmatch(r'MOV RCX,qword ptr \[RCX(?: \+ (0x[0-9a-f]+))?\]', i)
        if a:
            ptr.append(a.group(1) or '0x0')
    return dict(delta=delta, ptr_loads=ptr)


man = json.load(open('porting/behavior/MANIFEST.json'))
out = []
for e in man:
    if e.get('kind') != 'exec' or e.get('status') != 'ported' or not e.get('native_body_address'):
        continue
    if e['native_body_address'] in STUB or not e.get('address'):
        continue
    a = analyse(e['address'])
    if a is None:
        out.append(dict(cls=e['class'], fn=e['function'], thunk=e['address'], status='thunk_asm_missing'))
    elif a['delta'] or a['ptr_loads']:
        out.append(dict(cls=e['class'], fn=e['function'], thunk=e['address'], inc=e.get('inc_path'), **a, status='adjusted'))
os.makedirs(f'{R}/receiver-adjust', exist_ok=True)
json.dump(out, open(f'{R}/receiver-adjust/report.json', 'w'), indent=1)
print(len(out), {s: sum(1 for o in out if o['status'] == s) for s in ('adjusted', 'thunk_asm_missing')})
for o in out:
    if o['status'] == 'adjusted':
        print(o['cls'], o['fn'], hex(o['delta']), o['ptr_loads'], (o['inc'] or '')[-60:])
