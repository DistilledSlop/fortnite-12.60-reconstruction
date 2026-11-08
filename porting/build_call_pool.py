#!/usr/bin/env python3
"""Pool of unported functions whose single captured body has <= MAXI instructions and whose call targets are all resolvable (read-only).

A call target is resolvable when it is (a) a GetPrivateStaticClass getter (-> `Class::StaticClass()`), (b) the real body of a reflected native function
(final2 / ported native_body_address), or (c) an engine/shared helper in HELPERS. Writes generated/small-pool/pool_resolved.json with the job fields of
build_small_pool.py plus `callee_candidates` {addr: [{fn|static_class,...}]}. Skips policy classes, blocked, interface and already attempted wrapper jobs.
Run from repo root: python3 -I porting/build_call_pool.py [MAXI]"""
import collections, glob, json, os, re, sys
sys.path.insert(0, 'porting')
import thunk_receiver as tr
from generated_defs import defined_in_generated
import virtual_slots

MAXI = int(sys.argv[1]) if len(sys.argv) > 1 else 60
R = 'porting/generated'
HELPERS = {'0x142bee710': 'FMemory::Free', '0x1406a9dc0': 'ProcessMulticastDelegate',
           '0x142c54b70': 'FMsg::Logf (verbosity-gated UE_LOG)',
           '0x142ee5dd0': 'TWeakObjectPtr::Get', '0x142ee7880': 'TWeakObjectPtr::IsValid', '0x142ee3b30': 'TWeakObjectPtr assign'}
STUBS = {'0x1406801c0', '0x1406801d0', '0x1406801e0', '0x140680300'}
idx = {}
for d in glob.glob(f'{R}/native*/'):
    for f in os.listdir(d):
        m = re.fullmatch(r'(0x[0-9a-f]+)\.asm', f)
        if m:
            idx.setdefault(m.group(1), d + f)
f2 = json.load(open(f'{R}/thunk-table/final3.json'))
nf = json.load(open(f'{R}/reflection/native_funcs.json'))
ci = json.load(open(f'{R}/reflection/native_class_index.json'))
man = json.load(open('porting/behavior/MANIFEST.json'))
gp = {}
for c, v in ci.items():
    for a in v.get('gpsc_targets') or []:
        gp.setdefault(a, []).append(c)
named = collections.defaultdict(set)
for e in f2:
    for b in e.get('real_bodies') or []:
        named[b].add((e['cls'], e['fn']))
for m in man:
    a = m.get('native_body_address')
    if a and m.get('kind') != 'member_field':
        fn = m['function'][:-len('_Implementation')] if m['function'].endswith('_Implementation') else m['function']
        named[a].add((m['class'], fn))
hcand = collections.defaultdict(list)  # ported helper entries by native address: callable by their declared signature
for m in man:
    if m.get('status') == 'ported' and m.get('kind') in ('helper', 'static_helper') and m.get('address'):
        hcand[m['address']].append(dict(fn=m['class'] + '::' + m['function'], static=bool(m.get('static') or m.get('kind') == 'static_helper'),
                                        signature=m['signature'], note='ported helper for native %s: call it as declared' % m['address']))
done = {(e['class'], e['function']) for e in man if e.get('status') == 'ported' and e.get('kind') != 'member_field'}
done |= {(e['class'], e['function'][:-len('_Implementation')]) for e in man if e.get('status') == 'ported' and e['function'].endswith('_Implementation')}
blocked = {x['entry'] for x in json.load(open('porting/behavior/stub_bulk_runtime/blocked.json'))}
for _r in glob.glob('porting/behavior/stub_bulk_runtime/build_removed_*.json'):
    blocked |= {x['entry'].replace('_Implementation::', '::') if False else (x['entry'][:-len('_Implementation')] if x['entry'].endswith('_Implementation') else x['entry']) for x in json.load(open(_r))}
inscope = json.load(open('porting/progress/native_functions_inscope.json'))
inscope = {(c, f) for c, fs in inscope.items() for f in fs}
if os.environ.get('TARGETS'):  # restrict to a target list [{cls,fn}] (content-used functions)
    _t = {(x['cls'], x['fn']) for x in json.load(open(os.environ['TARGETS']))}
    inscope &= _t
attempted = set()
for p in glob.glob(f'{R}/work-batches/wrap[0-9A]*.json') + glob.glob(f'{R}/work-batches/bcast*.json') + glob.glob(f'{R}/work-batches/rs[0-9]*.json'):
    if not os.path.isdir('porting/behavior/%s_runtime' % os.path.basename(p)[:-5]):
        continue  # generated but never run by a worker: those functions are still open
    for j in json.load(open(p)):
        attempted.add((j['cls'], j['fn']))
# functions a worker already blocked with a recorded reason are not re-proposed until the reason changes
for _d in glob.glob('porting/behavior/*_runtime/dispositions.json'):
    try:
        for _x in json.load(open(_d)):
            if _x.get('status') == 'blocked' and _x.get('class') and _x.get('function'):
                attempted.add((_x['class'], _x['function']))
    except Exception:
        pass
NO_GENERATED_HEADER = set()  # UFortKismetLibrary is generator-owned now (was hand-written)
hdr = {os.path.basename(p)[:-2]: p for p in glob.glob('porting/native/Source/*/Generated/*.h')}


def span(p):
    a = [int(l.split(None, 1)[0], 16) for l in open(p) if re.match(r'^[0-9a-f]+\s', l)]
    return min(a), max(a)


def this_virtuals(L):
    """Slots of indirect calls made through the primary vptr of `this`; the second value counts every other indirect call.
    Tracks RCX = this on entry, register copies, `MOV Rv,[Rthis]` (vptr) and `CALL/JMP [Rv + slot]`."""
    reg = {'RCX': 'this'}
    out, bad = [], 0
    vol = ('RAX', 'RCX', 'RDX', 'R8', 'R9', 'R10', 'R11')
    for i in L:
        m = re.fullmatch(r'MOV (R\w+),(R\w+)', i)
        if m:
            reg[m.group(1)] = reg.get(m.group(2))
            continue
        m = re.fullmatch(r'MOV (R\w+),qword ptr \[(R\w+)\]', i)
        if m:
            reg[m.group(1)] = 'vptr' if reg.get(m.group(2)) == 'this' else None
            continue
        m = re.fullmatch(r'(?:CALL|JMP) qword ptr \[(R\w+)(?: \+ (0x[0-9a-f]+))?\]', i)
        if m:
            if reg.get(m.group(1)) == 'vptr':
                out.append(int(m.group(2), 16) if m.group(2) else 0)
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


def insns(p):
    return [l.split(None, 1)[1].strip() for l in open(p) if len(l.split(None, 1)) > 1]


pool, st = [], collections.Counter()
for e in f2:
    k = (e['cls'], e['fn'])
    if k in done or f"{e['cls']}::{e['fn']}" in blocked or k not in inscope or e['group'] in ('A', 'I', 'U') or e['cls'] in NO_GENERATED_HEADER or defined_in_generated(e['cls'], e['fn']) or k in attempted:
        st['skip_done_blocked_policy_attempted'] += 1
        continue
    b = e.get('real_bodies') or []
    if len(b) != 1 or b[0] not in idx:
        st['skip_not_single_or_uncaptured'] += 1
        continue
    L = insns(idx[b[0]])
    if len(L) > MAXI:
        st['skip_too_big'] += 1
        continue
    tg = [m.group(1) for i in L for m in [re.fullmatch(r'(?:CALL|JMP) (0x[0-9a-f]+)', i)] if m]
    ind = [i for i in L if re.match(r'(CALL|JMP) (qword ptr|R)', i)]
    tgt = [t for t in tg if t not in HELPERS and t not in STUBS]
    vslots, vbad = this_virtuals(L) if ind else ([], 0)
    if ind and (vbad or len(vslots) != len(ind)):
        st['skip_indirect_not_this_vptr'] += 1
        continue
    cal = {}
    vnames = {}
    for sl in vslots:
        vn = virtual_slots.virt_name(e['cls'], sl)
        if not vn:
            _t = virtual_slots.virt_target(e['cls'], sl)
            _chain = virtual_slots.PARENTS.get(e['cls'], [e['cls']])
            _h = [h for h in hcand.get(_t, []) if h['fn'].partition('::')[0] in _chain and not h['static']] if _t else []
            if not _h:
                vnames = None
                break
            vnames[sl] = ('HELPER', _h[0])
            continue
        vnames[sl] = vn
    if vnames is None:
        st['skip_virtual_slot_unnamed'] += 1
        continue
    for sl, (vc, vf) in vnames.items():
        if vc == 'HELPER':
            cal['virtual+0x%x' % sl] = [dict(vf, virtual_slot=hex(sl), note='ported helper for this vtable slot (non-virtual call on this): call %s on this exactly as declared' % vf['fn'])]
            continue
        d = nf.get(vc, {}).get(vf, {})
        fl = d.get('flags')
        fl = fl if isinstance(fl, int) else int(fl, 16) if fl else 0
        cal['virtual+0x%x' % sl] = [dict(virtual_slot=hex(sl), fn=f'{vc}::{vf}', static=bool(fl & 0x2000), const=bool(fl & 0x40000000),
                                          params=[(q['name'], q['kind']) for q in d.get('params') or []],
                                          note='virtual call on this through the class vtable (slot named by address): call %s()' % vf)]
    ok = True
    for t in tgt:
        if t in gp:
            cal[t] = [dict(static_class=c + '::StaticClass()') for c in gp[t]]
        elif t in hcand and t not in named:
            cal[t] = hcand[t]
        elif t in named:
            cal[t] = []
            for c, f in sorted(named[t])[:6]:
                d = nf.get(c, {}).get(f, {})
                fl = d.get('flags')
                fl = fl if isinstance(fl, int) else int(fl, 16) if fl else 0
                cal[t].append(dict(fn=f'{c}::{f}', static=bool(fl & 0x2000), const=bool(fl & 0x40000000), params=[(p['name'], p['kind']) for p in d.get('params') or []]))
        elif span(idx[b[0]])[0] <= int(t, 16) <= span(idx[b[0]])[1]:  # branch target inside the body is not an external call
            continue
        else:
            ok = False
            break
    if not ok:
        st['skip_unresolved_callee'] += 1
        continue
    ri = tr.receiver_info(e['thunk'], b[0])
    if ri is None or ri['extra_calls'] or ri['foreign_virtuals']:
        st['skip_thunk_unclean_or_uncaptured'] += 1
        continue
    wd = sum(r['this_delta'] for r in e['resolved'] if r['real'] == b[0])
    d = nf[e['cls']][e['fn']]
    pool.append(dict(cls=e['cls'], fn=e['fn'], thunk=e['thunk'], body=b[0], group=e['group'], flags=d.get('flags'), shape='call' if tgt else 'leaf',
                     insns=L, receiver_delta=ri['delta'], this_delta_total=ri['delta'] + wd,
                     params=[(p['name'], p['kind']) for p in d.get('params') or []], header=hdr.get(e['cls']), callee_candidates=cal))
    st['pool'] += 1


# dependency filter: a callee must be declared in its class header, already ported, or itself in the pool (declared by the same promotion).
_hdr_txt = {}


def declared(c, f):
    if (c, f) in done:
        return True
    if c not in _hdr_txt:
        pth = hdr.get(c)
        _hdr_txt[c] = open(pth).read() if pth else ''
    return re.search(r'\b%s\s*\(' % re.escape(f), _hdr_txt[c]) is not None


changed = True
while changed:
    changed = False
    inpool = {(j['cls'], j['fn']) for j in pool}
    keep = []
    for j in pool:
        bad = False
        for cands in j['callee_candidates'].values():
            for cd in cands:
                if 'fn' in cd:
                    c, _, f = cd['fn'].partition('::')
                    if not (declared(c, f) or (c, f) in inpool):
                        bad = True
        if bad:
            st['skip_callee_not_declared_or_pooled'] += 1
            st['pool'] -= 1
            changed = True
        else:
            keep.append(j)
    pool = keep
# keep dependent jobs adjacent so a batch contains a callee together with its callers
comp = {}
for j in pool:
    comp[(j['cls'], j['fn'])] = (j['cls'], j['fn'])


def find(x):
    while comp[x] != x:
        comp[x] = comp[comp[x]]
        x = comp[x]
    return x


for j in pool:
    for cands in j['callee_candidates'].values():
        for cd in cands:
            if 'fn' in cd:
                c, _, f = cd['fn'].partition('::')
                if (c, f) in comp:
                    comp[find((j['cls'], j['fn']))] = find((c, f))
pool.sort(key=lambda j: find((j['cls'], j['fn'])))
os.makedirs(f'{R}/small-pool', exist_ok=True)
json.dump(pool, open(f'{R}/small-pool/pool_resolved.json', 'w'))
print(dict(st))
print(collections.Counter(len(p['insns']) // 10 * 10 for p in pool))
