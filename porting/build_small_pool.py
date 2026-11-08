#!/usr/bin/env python3
"""Pool of unported functions whose real body is a small leaf and whose thunk is a clean forwarder (read-only).
Filters: final2 group B/C with ONE real body, body asm captured, <= MAXI instructions, no CALL/JMP-out, thunk has no extra
direct calls and no indirect calls on non-this receivers (thunk_receiver), not ported/blocked/interface/engine-provided.
Classifies the shape and writes generated/small-pool/pool.json: [{cls,fn,thunk,body,group,virtual,flags,shape,insns,
receiver_delta,this_delta_total,params,header_decl}]. Run from repo root: python3 -I porting/build_small_pool.py [MAXI]"""
import collections, glob, json, os, re, sys
sys.path.insert(0, 'porting')
import thunk_receiver as tr

MAXI = int(sys.argv[1]) if len(sys.argv) > 1 else 10
CALLS = len(sys.argv) > 2 and sys.argv[2] == 'calls'  # wrapper tier: keep bodies that CALL/JMP out (pool_calls.json)
R = 'porting/generated'
STUB = {'0x1406801c0': 'ret', '0x1406801d0': 'ret_true', '0x1406801e0': 'ret_zero', '0x140680300': 'ret_zero'}
idx = {}
for d in glob.glob(f'{R}/native*/'):
    for f in os.listdir(d):
        m = re.fullmatch(r'(0x[0-9a-f]+)\.asm', f)
        if m:
            idx.setdefault(m.group(1), d + f)
f2 = json.load(open(f'{R}/thunk-table/final3.json'))
nf = json.load(open(f'{R}/reflection/native_funcs.json'))
man = json.load(open('porting/behavior/MANIFEST.json'))
done = {(e['class'], e['function']) for e in man if e.get('status') == 'ported' and e.get('kind') != 'member_field'}
done |= {(e['class'], e['function'][:-len('_Implementation')]) for e in man
         if e.get('status') == 'ported' and e['function'].endswith('_Implementation')}
blocked = {x['entry'] for x in json.load(open('porting/behavior/stub_bulk_runtime/blocked.json'))}
inscope = json.load(open('porting/progress/native_functions_inscope.json'))
inscope = {(c, f) for c, fs in inscope.items() for f in fs}
hdr = {}
for p in glob.glob('porting/native/Source/*/Generated/*.h'):
    hdr[os.path.basename(p)[:-2]] = p


def insns(path):
    return [l.split(None, 1)[1].strip() for l in open(path) if len(l.split(None, 1)) > 1]


def shape(L):
    if len(L) == 1 and L[0] == 'RET':
        return 'ret'
    j = '; '.join(L)
    if re.fullmatch(r'XOR E?AX,E?AX; RET', j) or re.fullmatch(r'XOR EAX,EAX; RET', j):
        return 'ret_zero'
    if re.fullmatch(r'MOV AL,0x1; RET|MOV EAX,0x1; RET', j):
        return 'ret_true'
    if re.fullmatch(r'MOV E?AX,0x[0-9a-f]+; RET', j):
        return 'ret_const'
    if re.fullmatch(r'(MOVZX|MOV|MOVSXD|MOVSX) \w+,(byte|word|dword|qword) ptr \[RCX(?: \+ 0x[0-9a-f]+)?\]; RET', j):
        return 'getter'
    if re.fullmatch(r'LEA RAX,\[RCX \+ 0x[0-9a-f]+\]; RET', j):
        return 'addr_getter'
    if L and L[-1] == 'RET' and all(re.match(r'MOV (byte|word|dword|qword) ptr \[RCX(?: \+ 0x[0-9a-f]+)?\],', i) or i.startswith(('MOV ', 'MOVZX', 'MOVSS ', 'MOVSD ', 'MOVAPS', 'MOVUPS', 'XOR ', 'OR ', 'AND ', 'TEST', 'SETZ', 'SETNZ', 'RET', 'NOT ', 'LEA ', 'SHR ', 'SHL ', 'ADD ', 'SUB ', 'INC ', 'DEC ', 'CMP ', 'J', 'CMOV', 'SBB ', 'NEG ', 'MOVSXD', 'MOVSX ', 'CDQE', 'CMOVNZ', 'CMOVZ', 'MOVQ', 'MOVD', 'PXOR', 'CVT', 'SAR ')) for i in L):
        return 'leaf_other'
    return 'other'


pool = []
st = collections.Counter()
for e in f2:
    k = (e['cls'], e['fn'])
    if k in done or f"{e['cls']}::{e['fn']}" in blocked or k not in inscope or e['group'] in ('A', 'I', 'U'):
        st['skip_group_done_blocked'] += 1
        continue
    bodies = e.get('real_bodies') or []
    if len(bodies) != 1 or bodies[0] not in idx:
        st['skip_not_single_or_uncaptured'] += 1
        continue
    body = bodies[0]
    L = insns(idx[body])
    has_out = any(i.startswith(('CALL', 'JMP')) for i in L)
    if len(L) > MAXI or has_out != CALLS:
        st['skip_not_small_leaf'] += 1
        continue
    ri = tr.receiver_info(e['thunk'], body)
    if ri is None or ri['extra_calls'] or ri['foreign_virtuals']:
        st['skip_thunk_unclean_or_uncaptured'] += 1
        continue
    wd = sum(r['this_delta'] for r in e['resolved'] if r['real'] == body)
    d = nf[e['cls']][e['fn']]
    sh = STUB.get(body) and ('stub_' + STUB[body]) or shape(L)
    pool.append(dict(cls=e['cls'], fn=e['fn'], thunk=e['thunk'], body=body, group=e['group'], flags=d.get('flags'),
                     shape=sh, insns=L, receiver_delta=ri['delta'], this_delta_total=ri['delta'] + wd,
                     params=[(p['name'], p['kind']) for p in d.get('params') or []], header=hdr.get(e['cls'])))
    st['pool'] += 1
os.makedirs(f'{R}/small-pool', exist_ok=True)
json.dump(pool, open(f'{R}/small-pool/' + ('pool_calls.json' if CALLS else 'pool.json'), 'w'))
print(dict(st))
print(collections.Counter(p['shape'] for p in pool))
print('with nonzero total delta:', sum(1 for p in pool if p['this_delta_total']))
