#!/usr/bin/env python3
"""Pool of unported functions whose single captured body has <= MAXI instructions and whose call targets are all resolvable (read-only).

A call target is resolvable when it is (a) a GetPrivateStaticClass getter (-> `Class::StaticClass()`), (b) the real body of a reflected native function
(final2 / ported native_body_address), or (c) an engine/shared helper in HELPERS. Writes generated/small-pool/pool_resolved.json with the job fields of
build_small_pool.py plus `callee_candidates` {addr: [{fn|static_class,...}]}. Skips policy classes, blocked, interface and already attempted wrapper jobs.
Run from repo root: python3 -I porting/build_call_pool.py [MAXI]"""
import collections, glob, json, os, re, sys
sys.path.insert(0, 'porting')
import thunk_receiver as tr

MAXI = int(sys.argv[1]) if len(sys.argv) > 1 else 60
R = 'porting/generated'
HELPERS = {'0x142bee710': 'FMemory::Free', '0x1406a9dc0': 'ProcessMulticastDelegate'}
STUBS = {'0x1406801c0', '0x1406801d0', '0x1406801e0', '0x140680300'}
idx = {}
for d in glob.glob(f'{R}/native*/'):
    for f in os.listdir(d):
        m = re.fullmatch(r'(0x[0-9a-f]+)\.asm', f)
        if m:
            idx.setdefault(m.group(1), d + f)
f2 = json.load(open(f'{R}/thunk-table/final2.json'))
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
done = {(e['class'], e['function']) for e in man if e.get('status') == 'ported' and e.get('kind') != 'member_field'}
done |= {(e['class'], e['function'][:-len('_Implementation')]) for e in man if e.get('status') == 'ported' and e['function'].endswith('_Implementation')}
blocked = {x['entry'] for x in json.load(open('porting/behavior/stub_bulk_runtime/blocked.json'))}
inscope = json.load(open('porting/progress/native_functions_inscope.json'))
inscope = {(c, f) for c, fs in inscope.items() for f in fs}
attempted = set()
for p in glob.glob(f'{R}/work-batches/wrap[0-9A]*.json') + glob.glob(f'{R}/work-batches/bcast*.json'):
    for j in json.load(open(p)):
        attempted.add((j['cls'], j['fn']))
POLICY = {"UCommonBorder", "UCommonInputSettings", "UCommonUISettings", "UCommonInputControllerData", "UFortLoginInteractions",
          "UFortProgressModal", "UFortRootViewportLayout", "UFortSignInWidget", "UCommonUIActionRouter", "UCommonButtonInternal",
          "UCommonActivatableWidget", "UCommonActivatablePanel", "UCommonInputManager", "UFortRestartClientModal", "UCommonInputSubsystem",
          "UCommonActionWidget", "UCommonButton", "UCommonTextBlock", "UFortGlobals", "UFortGlobalUIContext", "UFortUINotification",
          "UCommonLazyImage", "UCommonActivatableWidgetContainerBase", "UFortKismetLibrary"}
hdr = {os.path.basename(p)[:-2]: p for p in glob.glob('porting/native/Source/*/Generated/*.h')}


def insns(p):
    return [l.split(None, 1)[1].strip() for l in open(p) if len(l.split(None, 1)) > 1]


pool, st = [], collections.Counter()
for e in f2:
    k = (e['cls'], e['fn'])
    if k in done or f"{e['cls']}::{e['fn']}" in blocked or k not in inscope or e['group'] in ('A', 'I', 'U') or e['cls'] in POLICY or k in attempted:
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
    if ind:
        st['skip_indirect'] += 1
        continue
    cal = {}
    ok = True
    for t in tgt:
        if t in gp:
            cal[t] = [dict(static_class=c + '::StaticClass()') for c in gp[t]]
        elif t in named:
            cal[t] = []
            for c, f in sorted(named[t])[:6]:
                d = nf.get(c, {}).get(f, {})
                fl = d.get('flags')
                fl = fl if isinstance(fl, int) else int(fl, 16) if fl else 0
                cal[t].append(dict(fn=f'{c}::{f}', static=bool(fl & 0x2000), const=bool(fl & 0x40000000), params=[(p['name'], p['kind']) for p in d.get('params') or []]))
        elif t in idx and 0 < t.count('x'):  # local branch target inside the body is not an external call
            lo = int(b[0], 16)
            if lo <= int(t, 16) < lo + 0x400:
                continue
            ok = False
            break
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
os.makedirs(f'{R}/small-pool', exist_ok=True)
json.dump(pool, open(f'{R}/small-pool/pool_resolved.json', 'w'))
print(dict(st))
print(collections.Counter(len(p['insns']) // 10 * 10 for p in pool))
