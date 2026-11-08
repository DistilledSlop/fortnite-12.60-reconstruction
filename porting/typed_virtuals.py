"""Receiver-typed virtual call resolution (read-only helper). Run from the repo root.

analyse(L, cls, delta): walk a body linearly tracking registers as this / vptr-of-C / object-of-C. `MOV R,[RCX+off]` loads an
object whose class is the pointee class of the reflected Object property covering object offset delta+off (own or inherited);
`MOV R2,[R]` loads that object's vptr; `CALL/JMP [R2+slot]` is a virtual call on it. Returns (calls, bad): calls = list of
(receiver_class, slot, member_name_or_None) in order, bad = count of indirect calls not attributable to a typed receiver."""
import json, re
R = 'porting/generated/reflection/'
_rf = json.load(open(R + 'fortnite_game_reflection.json'))
_nm = json.load(open(R + 'class_name_map.json'))
_ci = json.load(open('porting/generated/reflection/native_class_index.json'))
_pm = json.load(open(R + 'class_parent_map.json'))
_gp = {}
for c, v in _ci.items():
    for a in v.get('gpsc_targets') or []:
        _gp.setdefault(a, c)
_props = {}
for c in _rf['classes_in_groups_with_fort_names']:
    n = c.get('name') or _nm.get(c['block_va'])
    _props[n] = {p['offset']: p for p in c.get('properties') or [] if p.get('offset') is not None and p['kind'] == 'Object'}
import os
_iat = json.load(open('porting/generated/small-pool/iat.json')) if os.path.exists('porting/generated/small-pool/iat.json') else {}
_chain = {v['name']: v.get('chain') or [v['name']] for v in _pm.values() if isinstance(v, dict) and v.get('name')}


def member_class(cls, off):
    for c in _chain.get(cls, [cls]):
        p = _props.get(c, {}).get(off)
        if p:
            k = _gp.get((p.get('type_refs') or {}).get('class'))
            return (k, p['name']) if k else None
    return None


def analyse(L, cls, delta):
    reg = {'RCX': ('this', cls)}
    out, bad = [], 0
    vol = ('RAX', 'RCX', 'RDX', 'R8', 'R9', 'R10', 'R11')
    for i in L:
        m = re.fullmatch(r'MOV (R\w+),(R\w+)', i)
        if m:
            reg[m.group(1)] = reg.get(m.group(2))
            continue
        m = re.fullmatch(r'MOV (R\w+),qword ptr \[(R\w+)(?: \+ (0x[0-9a-f]+))?\]', i)
        if m:
            src = reg.get(m.group(2)); off = int(m.group(3), 16) if m.group(3) else 0
            if src and src[0] in ('this', 'obj') and off == 0:
                reg[m.group(1)] = ('vptr', src[1], src[2] if len(src) > 2 else None)
            elif src and src[0] == 'this':
                mc = member_class(src[1], delta + off)
                reg[m.group(1)] = ('obj', mc[0], mc[1]) if mc else None
            else:
                reg[m.group(1)] = None
            continue
        m = re.fullmatch(r'(?:CALL|JMP) qword ptr \[(0x14[0-9a-f]{7})\]', i)
        if m and m.group(1) in _iat:  # import call (kernel32/CRT): resolved by name, not a virtual call
            if i.startswith('CALL'):
                for r in vol:
                    reg[r] = None
            continue
        m = re.fullmatch(r'(?:CALL|JMP) qword ptr \[(R\w+)(?: \+ (0x[0-9a-f]+))?\]', i)
        if m:
            v = reg.get(m.group(1))
            if v and v[0] == 'vptr':
                out.append((v[1], int(m.group(2), 16) if m.group(2) else 0, v[2] if len(v) > 2 else None))
            else:
                bad += 1
            if i.startswith('CALL'):
                for r in vol:
                    reg[r] = None
            continue
        if re.match(r'(CALL|JMP) ', i):
            if i.startswith('CALL'):
                for r in vol:
                    reg[r] = None
            if re.match(r'(CALL|JMP) R', i):
                bad += 1
            continue
        m = re.match(r'(?:MOV|LEA|ADD|SUB|XOR|AND|OR|MOVZX|MOVSXD|MOVSX|SHL|SHR|IMUL|INC|DEC|POP|NEG|NOT|CMOV\w+|SET\w+) (R\w+)', i)
        if m:
            reg[m.group(1)] = None
    return out, bad
