#!/usr/bin/env python3
"""Pick the real body for thunks that call several bodies (read-only; writes thunk-table/final3.json).
Rule: in an exec thunk the real body is the direct call whose RCX still holds the object pointer (`this`, possibly
adjusted), while parameter-decode helpers receive the frame or a local address. A tiny register tracker over the thunk asm
decides this. RPC thunks additionally call `_Validate`, recognised by `TEST AL,AL` right after the call; it is excluded.
The rule is validated on every thunk that has exactly one body (does the tracker name that body?).
Usage: python3 -I porting/disambiguate_bodies.py"""
import collections, glob, json, os, re
R = 'porting/generated'
idx = {}
for d in glob.glob(f'{R}/native*/'):
    for f in os.listdir(d):
        m = re.fullmatch(r'(0x[0-9a-f]+)\.asm', f)
        if m:
            idx.setdefault(m.group(1), d + f)
VOL = ('RAX', 'RCX', 'RDX', 'R8', 'R9', 'R10', 'R11')
f2 = json.load(open(f'{R}/thunk-table/final2.json'))


def calls(thunk):
    """[(target, rcx_state, validate_like)] for the direct CALL/JMP sites in the thunk, in order."""
    L = [l.split(None, 1)[1].strip() for l in open(idx[thunk]) if len(l.split(None, 1)) > 1]
    reg = {'RCX': 'this'}
    out = []
    for i, ins in enumerate(L):
        m = re.fullmatch(r'MOV (R\w+),(R\w+)', ins)
        if m:
            reg[m.group(1)] = reg.get(m.group(2))
            continue
        m = re.fullmatch(r'LEA (R\w+),\[(R\w+)(?: \+ 0x[0-9a-f]+)?\]', ins)
        if m:
            reg[m.group(1)] = reg.get(m.group(2))
            continue
        m = re.fullmatch(r'(?:ADD|SUB) (R\w+),0x[0-9a-f]+', ins)
        if m:
            continue
        m = re.fullmatch(r'(CALL|JMP) (0x[0-9a-f]+)', ins)
        if m:
            nxt = L[i + 1] if i + 1 < len(L) else ''
            out.append((m.group(2), reg.get('RCX'), bool(re.match(r'TEST AL,AL|TEST AL,0x1|CMP AL', nxt))))
            if m.group(1) == 'CALL':
                for r in VOL:
                    reg[r] = None
            continue
        if re.match(r'(CALL|JMP) ', ins):
            if ins.startswith('CALL'):
                for r in VOL:
                    reg[r] = None
            continue
        m = re.match(r'(?:MOV|MOVZX|MOVSXD|MOVSX|XOR|AND|OR|POP|IMUL|SHL|SHR|NEG|NOT|INC|DEC|CMOV\w+|SET\w+|MOVAPS|MOVUPS|MOVQ|MOVD) (R\w+)', ins)
        if m:
            reg[m.group(1)] = None
    return out


def choose(e):
    thunk_calls = calls(e['thunk'])
    real_of = {r['entry']: r['real'] for r in e.get('resolved', [])}
    cand = []
    for t, state, validate in thunk_calls:
        r = real_of.get(t)
        if r is not None and state == 'this' and not validate and r not in cand:
            cand.append(r)
    return cand


VT = {(r['cls'], r['fn']): r for r in json.load(open(f'{R}/thunk-table/virtual_targets.json'))}


def vcalls(thunk):
    L = [l.split(None, 1)[1].strip() for l in open(idx[thunk]) if len(l.split(None, 1)) > 1]
    out = []
    for i, ins in enumerate(L):
        m = re.fullmatch(r'(?:CALL|JMP) qword ptr \[(R\w+) \+ (0x[0-9a-f]+)\]', ins)
        if m:
            nxt = L[i + 1] if i + 1 < len(L) else ''
            out.append((int(m.group(2), 16), bool(re.match(r'TEST AL,AL|CMP AL', nxt))))
    return out


def choose_virtual(e):
    """Group C: the vtable slot whose indirect call is not followed by a validate-style TEST AL,AL."""
    r = VT.get((e['cls'], e['fn']))
    if not r or e['thunk'] not in idx:
        return []
    vc = vcalls(e['thunk'])
    real = {x['entry']: x['real'] for x in e['resolved']}
    out = []
    for s_ in r['sites']:
        if s_['kind'] == 'primary' and s_['status'] == 'resolved' and any(off == int(s_['slot']) and not val for off, val in vc):
            t = real.get(s_['target'], s_['target'])
            if t not in out:
                out.append(t)
    return out


tp = fp = nomatch = 0
vtp = vfp = vnone = 0
for e in f2:
    b = e.get('real_bodies') or []
    if len(b) == 1 and e['group'] == 'C' and e['thunk'] in idx:
        c = choose_virtual(e)
        if c == b:
            vtp += 1
        elif not c:
            vnone += 1
        else:
            vfp += 1
print(f'validation of the virtual-slot rule on single-body group C thunks: correct {vtp}, names nothing {vnone}, wrong {vfp}')
for e in f2:
    b = e.get('real_bodies') or []
    if len(b) == 1 and e['thunk'] in idx:
        c = choose(e)
        if c == b:
            tp += 1
        elif not c:
            nomatch += 1
        else:
            fp += 1
print(f'validation on single-body thunks: tracker names the body {tp}, names nothing {nomatch}, names something else {fp}')
res = still = 0
for e in f2:
    b = e.get('real_bodies') or []
    if len(b) > 1 and e['thunk'] in idx:
        c = [x for x in choose(e) if x in b]
        if len(c) != 1 and e['group'] == 'C':
            c = [x for x in choose_virtual(e) if x in b]
        if len(c) == 1:
            e['all_candidate_bodies'] = b
            e['real_bodies'] = c
            e['disambiguated'] = True
            res += 1
        else:
            still += 1
print('multi-body functions resolved:', res, ' still ambiguous:', still)
json.dump(f2, open(f'{R}/thunk-table/final3.json', 'w'))
