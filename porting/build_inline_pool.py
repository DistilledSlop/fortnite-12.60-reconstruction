#!/usr/bin/env python3
"""Pool of group A (inline thunk) functions whose thunk is a simple body (read-only).
An exec thunk that contains its function's body starts with the six-instruction P_FINISH sequence
(`MOV RAX,[RDX+0x20]; XOR RnD,RnD; TEST RAX,RAX; SETNZ RnB; ADD Rn,RAX; MOV [RDX+0x20],Rn` (n = 8..11)), reads no parameters, and stores the
return value through R8. The pool keeps functions with at most one reflected parameter (the return value), strips that prefix and keeps
the rest as the body: RCX is `this`, `[R8]` is the return slot. Writes generated/small-pool/pool_inline.json.
Usage: python3 -I porting/build_inline_pool.py [MAXI]"""
import collections, glob, json, os, re, sys
sys.path.insert(0, 'porting')
from generated_defs import defined_in_generated
R = 'porting/generated'
MAXI = int(sys.argv[1]) if len(sys.argv) > 1 else 40
idx = {}
for d in glob.glob(f'{R}/native*/'):
    for f in os.listdir(d):
        m = re.fullmatch(r'(0x[0-9a-f]+)\.asm', f)
        if m:
            idx.setdefault(m.group(1), d + f)
f2 = json.load(open(f'{R}/thunk-table/final3.json'))
nf = json.load(open(f'{R}/reflection/native_funcs.json'))
man = json.load(open('porting/behavior/MANIFEST.json'))
done = {(e['class'], e['function'].removesuffix('_Implementation')) for e in man if e.get('status') == 'ported' and e.get('kind') != 'member_field'}
inscope = {(c, f) for c, fs in json.load(open('porting/progress/native_functions_inscope.json')).items() for f in fs}
hdr = {os.path.basename(p)[:-2]: p for p in glob.glob('porting/native/Source/*/Generated/*.h')}
attempted = {x for p in glob.glob(f'{R}/work-batches/*.json') if os.path.isdir('porting/behavior/%s_runtime' % os.path.basename(p)[:-5])
             for x in [(j['cls'], j['fn']) for j in json.load(open(p)) if isinstance(j, dict) and 'cls' in j]}
PRE = [r'MOV RAX,qword ptr \[RDX \+ 0x20\]', r'XOR R(8|9|10|11)D,R\1D', r'TEST RAX,RAX', r'SETNZ R(8|9|10|11)B', r'ADD R(8|9|10|11),RAX', r'MOV qword ptr \[RDX \+ 0x20\],R(8|9|10|11)']
st = collections.Counter()
pool = []
for e in f2:
    k = (e['cls'], e['fn'])
    if e['group'] != 'A' or k in done or k not in inscope or k in attempted or e['cls'] == 'UFortKismetLibrary' or defined_in_generated(*k):
        continue
    if e['thunk'] not in idx:
        st['no_asm'] += 1
        continue
    L = [l.split(None, 1)[1].strip() for l in open(idx[e['thunk']]) if len(l.split(None, 1)) > 1]
    if len(L) < 7 or any(not re.fullmatch(p, L[i]) for i, p in enumerate(PRE)):
        st['no_pfinish_prefix'] += 1
        continue
    body = L[6:]
    if len(body) > MAXI:
        st['too_big'] += 1
        continue
    d = nf[e['cls']][e['fn']]
    ps = d.get('params') or []
    if any(p['name'] != 'ReturnValue' for p in ps):
        st['has_parameters'] += 1
        continue
    if any(re.match(r'(CALL|JMP)', i) and not i.startswith('JMP 0x') for i in body) or any(re.fullmatch(r'(CALL|JMP) (0x[0-9a-f]+)', i) and i.startswith('CALL') for i in body):
        st['has_calls'] += 1
        continue
    pool.append(dict(cls=e['cls'], fn=e['fn'], thunk=e['thunk'], body=e['thunk'], group='A', flags=d.get('flags'), shape='inline_thunk',
                     insns=body, receiver_delta=0, this_delta_total=0, params=[(p['name'], p['kind']) for p in ps],
                     header=hdr.get(e['cls']), callee_candidates={}, note='inline thunk: P_FINISH prefix removed; RCX=this, [R8]=return slot'))
    st['pool'] += 1
json.dump(pool, open(f'{R}/small-pool/pool_inline.json', 'w'))
print(dict(st))
