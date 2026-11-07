#!/usr/bin/env python3
"""Propose manifest entries for the parameterless pure-stub family (orchestrator-owned; no manifest edit here).
Input generated/thunk-table/{stub_family,final2}.json; output porting/behavior/stub_bulk_runtime/entries.json and
skipped.json. Conventions follow the earlier bulk runs (bulk_virtual_scalar / bulk_native_event / bulk_empty_exec):
  exec         plain reflected function; shared return.inc/return_true.inc/return_false.inc (virtual) or empty.inc
  native_event_impl  `<fn>_Implementation`, pinned reflection_flags, shared return*.inc
  impl / validate    net RPC `<fn>_Implementation` (impl_empty.inc) and `<fn>_Validate` (validate_return_true.inc)
Functions whose return type fits the stub (void<->RET, bool<->return true/false). The generator
rejects any signature that does not match reflection (fail closed)."""
import collections, json, os, sys
TAG = sys.argv[1] if len(sys.argv) > 1 else 'pending'
import glob, re
R = 'porting/generated'
nf = json.load(open(f'{R}/reflection/native_funcs.json'))
man = json.load(open('porting/behavior/MANIFEST.json'))
have = {(e['class'], e['function']) for e in man}
BLOCKED = {b['entry'] for b in json.load(open('porting/behavior/stub_bulk_runtime/blocked.json'))}
G = {(e['cls'], e['fn']): e for e in json.load(open(f'{R}/thunk-table/final2.json'))}
_asm = {}
for _d in glob.glob(f'{R}/native*/'):
    for _f in os.listdir(_d):
        _m = re.fullmatch(r'(0x[0-9a-f]+)\.asm', _f)
        if _m:
            _asm.setdefault(_m.group(1), _d + _f)
_STUBS = {'RET': 'empty', 'XOR EAX,EAX ; RET': 'ret0', 'XOR AL,AL ; RET': 'retfalse', 'MOV AL,0x1 ; RET': 'rettrue',
          'MOV EAX,0x1 ; RET': 'ret1'}


def stub_of(addr):
    if addr not in _asm:
        return None
    L = [re.sub(r' 0x0$', '', l.split(None, 1)[1]) for l in open(_asm[addr]).read().splitlines() if l.strip()]
    return _STUBS.get(' ; '.join(L))


S = []
for (c, fn), e in G.items():
    if e['group'] not in ('B', 'C') or e.get('coverage') == 'full':
        continue
    st = [stub_of(a) for a in e['real_bodies']]
    if not st or not all(st):
        continue
    d = nf[c][fn]
    fl = d.get('flags')
    fl = fl if isinstance(fl, int) else int(fl, 16) if fl else 0
    S.append(dict(cls=c, fn=fn, bodies=e['real_bodies'], stubs=st, group=e['group'], coverage=e['coverage'],
                  thunk=e['thunk'], kind='net-rpc' if fl & 0x40 else 'native-event' if fl & 0x800 else 'plain'))
BODY = {}
for _r in S:
    for _a, _s in zip(_r['bodies'], _r['stubs']):
        BODY[(_r['thunk'], _s)] = _a
INC = {'return': 'porting/behavior/_shared/return.inc', 'true': 'porting/behavior/_shared/return_true.inc',
       'false': 'porting/behavior/_shared/return_false.inc', 'null': 'porting/behavior/_shared/return_nullptr.inc',
       'zero': 'porting/behavior/_shared/return_0.inc', 'empty': 'porting/behavior/_shared/empty.inc'}
STUBADDR = {'empty': '0x1406801c0', 'true': '0x1406801d0', 'false': '0x1406801e0'}
out, skip = [], collections.Counter()
skipped = []


def flags_of(d):
    f = d.get('flags')
    return f if isinstance(f, int) else int(f, 16) if f else 0


def access(f):
    return 'public' if f & 0x20000 else 'protected' if f & 0x80000 else 'private' if f & 0x40000 else 'public'


for r in S:
    c, fn = r['cls'], r['fn']
    d = nf[c][fn]
    fl = flags_of(d)
    params = d['params']
    why = None
    if any(f"{c}::{fn}{s}" in BLOCKED for s in ('', '_Implementation', '_Validate')):
        skip['blocked (see blocked.json)'] += 1
        continue
    has_params = bool([p for p in params if p['name'] != 'ReturnValue'])
    ret = [p for p in params if p['name'] == 'ReturnValue']
    retbool = bool(ret) and ret[0]['kind'] == 'Bool'
    retptr = bool(ret) and ret[0]['kind'] in ('Object', 'Class') and set(r['stubs']) == {'ret0'}
    retint = bool(ret) and ret[0]['kind'] == 'Int' and set(r['stubs']) == {'ret0'}
    if ret and not (retbool or retptr or retint):
        why = why or 'non-bool return'
    if r['coverage'] == 'partial' and r['kind'] != 'net-rpc':
        why = why or 'partial coverage'
    kinds = set(r['stubs'])
    if not why:
        if r['kind'] == 'net-rpc':
            if not (kinds <= {'empty', 'rettrue', 'retfalse'} and 'empty' in kinds) or ret:
                why = 'net rpc stub shape not empty(+true validate)'
        elif ret and retbool and kinds - {'rettrue', 'retfalse', 'ret0'}:
            why = 'returns value but stub shape does not fit'
        elif not ret and kinds != {'empty'}:
            why = 'void but stub is not empty'
    if why:
        skip[why] += 1
        skipped.append(dict(cls=c, fn=fn, why=why, stubs=r['stubs'], kind=r['kind']))
        continue
    grp = G[(c, fn)]['group']
    cm = dict(address=r['thunk'], status='ported', confidence='high')
    PH = '@PARAMS@' if has_params else ''
    base = dict(class_=c)
    if r['kind'] == 'net-rpc':
        for suffix, kind, key in (('_Implementation', 'impl', 'empty'), ('_Validate', 'validate', 'rettrue')):
            if suffix == '_Validate' and not (kinds & {'rettrue', 'retfalse'}):
                continue
            vfalse = 'retfalse' in kinds
            nm = fn + suffix
            if (c, nm) in have:
                continue
            e = dict(**{'class': c}, function=nm, kind=kind,
                     signature=('void ' if suffix == '_Implementation' else 'bool ') + nm + '(' + PH + ')',
                     signature_status='proposed',
                     inc_path='porting/behavior/_shared/impl_empty.inc' if kind == 'impl'
                     else 'porting/behavior/_shared/validate_return_false.inc' if vfalse
                     else 'porting/behavior/_shared/validate_return_true.inc',
                     **cm, native_body_address=BODY[(r['thunk'], 'empty')] if kind == 'impl'
                     else BODY[(r['thunk'], 'retfalse' if vfalse else 'rettrue')],
                     notes=f'Stub-bodied RPC; thunk {r["thunk"]} reaches stub bodies {r["bodies"]} (exact pure stubs); '
                           f'see behavior/stub_bulk_runtime. Source recovery only.')
            out.append(e)
        continue
    stub = 'empty' if not ret else 'null' if retptr else 'zero' if retint else 'true' if 'rettrue' in kinds else 'false'
    rt = ('bool ' if retbool else 'T* ' if retptr else 'int32 ') if ret else 'void '
    const = ' const' if fl & 0x40000000 else ''
    if r['kind'] == 'native-event':
        nm = fn + '_Implementation'
        if (c, nm) in have:
            skip['already ported'] += 1
            continue
        inc = INC['return'] if not ret else INC[stub]
        e = dict(**{'class': c}, function=nm, kind='native_event_impl', signature=f'{rt}{nm}({PH}){const}',
                 signature_status='proposed', inc_path=inc, **cm, reflection_flags=hex(fl), static=False,
                 access=access(fl), native_body_address=BODY[(r['thunk'], {'empty': 'empty', 'true': 'rettrue', 'false': 'retfalse', 'null': 'ret0', 'zero': 'ret0'}[stub])],
                 notes=f'Stub-bodied native event; thunk {r["thunk"]} reaches exact pure stub {r["bodies"]}; '
                       f'see behavior/stub_bulk_runtime. Source recovery only.')
        out.append(e)
    else:
        if (c, fn) in have:
            skip['already ported'] += 1
            continue
        if grp == 'C':
            inc = INC['return'] if not ret else INC[stub]
        else:
            inc = INC['empty'] if not ret else INC[stub]
        e = dict(**{'class': c}, function=fn, kind='exec', signature=f'{rt}{fn}({PH}){const}',
                 signature_status='proposed', inc_path=inc, **cm, reflection_flags=hex(fl),
                 static=bool(fl & 0x2000), virtual=(grp == 'C'), access=access(fl),
                 native_body_address=BODY[(r['thunk'], {'empty': 'empty', 'true': 'rettrue', 'false': 'retfalse', 'null': 'ret0', 'zero': 'ret0'}[stub])],
                 notes=f'Stub-bodied exec; thunk {r["thunk"]} reaches exact pure stub {r["bodies"]}; '
                       f'see behavior/stub_bulk_runtime. Source recovery only.')
        out.append(e)
os.makedirs('porting/behavior/stub_bulk_runtime', exist_ok=True)
json.dump(out, open(f'porting/behavior/stub_bulk_runtime/entries_{TAG}.json', 'w'), indent=1)
json.dump(skipped, open(f'porting/behavior/stub_bulk_runtime/skipped_{TAG}.json', 'w'), indent=1)
print(len(out), 'entries;', dict(skip))
print(collections.Counter(e['kind'] for e in out))
