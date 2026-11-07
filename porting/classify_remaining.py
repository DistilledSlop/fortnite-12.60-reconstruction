#!/usr/bin/env python3
"""Classify every in-scope unported native function into exactly one blocker group (read-only).
Writes generated/remaining/classification.json ({group: [Class::Fn,...]}) and prints counts. Run from repo root:
python3 -I porting/classify_remaining.py [MAXI]   (MAXI = body-size limit of the call tier, default 60)"""
import collections, glob, json, os, re, sys
sys.path.insert(0, 'porting')
import thunk_receiver as tr
R = 'porting/generated'
MAXI = int(sys.argv[1]) if len(sys.argv) > 1 else 60
STUBS = {'0x1406801c0', '0x1406801d0', '0x1406801e0', '0x140680300'}
HELPERS = {'0x142bee710', '0x1406a9dc0', '0x142c54b70', '0x142ee5dd0', '0x142ee7880', '0x142ee3b30'}
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
gp = {a for v in ci.values() for a in (v.get('gpsc_targets') or [])}
named = set()
for e in f2:
    named.update(e.get('real_bodies') or [])
for m in man:
    if m.get('native_body_address') and m.get('kind') != 'member_field':
        named.add(m['native_body_address'])
done = {(e['class'], e['function']) for e in man if e.get('status') == 'ported' and e.get('kind') != 'member_field'}
done |= {(e['class'], e['function'][:-len('_Implementation')]) for e in man
         if e.get('status') == 'ported' and e['function'].endswith('_Implementation')}
inscope = {(c, f) for c, fs in json.load(open('porting/progress/native_functions_inscope.json')).items() for f in fs}
SB = 'porting/behavior/stub_bulk_runtime/'
reg = {}  # (cls, fn) -> recorded blocker from earlier worker/triage passes
for x in json.load(open(SB + 'blocked.json')):
    c, _, f = x['entry'].partition('::')
    reg[(c, f.removesuffix('_Implementation'))] = 'recorded_blocker:' + x['blocker']
for n in ('wrapper_tier_deferred', 'resolved_call_tier_deferred'):
    for x in json.load(open(SB + n + '.json'))['entries']:
        reg[(x['class'], x['function'])] = 'worker_deferred:' + x['case']
for x in json.load(open(SB + 'deferred_tiers.json'))['entries']:
    c, _, f = x['entry'].partition('::')
    reg[(c, f.removesuffix('_Implementation'))] = 'worker_deferred:' + (x.get('tier') or 'other')
for p in sorted(glob.glob('porting/behavior/*_runtime/dispositions.json')):
    try:
        rows = json.load(open(p))
    except Exception:
        continue
    for x in rows if isinstance(rows, list) else []:
        c, f = x.get('class') or x.get('cls'), x.get('function')
        if not (c and f) or x.get('status') in ('proposed', 'ported'):
            continue
        why = x.get('case') or (x.get('reason') or 'recorded').split()[0].strip(':,.')
        reg.setdefault((c, f.removesuffix('_Implementation')), 'worker_deferred:' + str(why)[:40])
attempted = set()
for p in glob.glob(f'{R}/work-batches/*.json'):
    if re.search(r'(manifest_backup|rejected|sigprobe)', p) or not os.path.isdir('porting/behavior/%s_runtime' % os.path.basename(p)[:-5]):
        continue  # only batches that a worker actually ran
    try:
        for j in json.load(open(p)):
            attempted.add((j['cls'], j['fn']))
    except Exception:
        pass


POLICY = {"UCommonBorder", "UCommonInputSettings", "UCommonUISettings", "UCommonInputControllerData", "UFortLoginInteractions",
          "UFortProgressModal", "UFortRootViewportLayout", "UFortSignInWidget", "UCommonUIActionRouter", "UCommonButtonInternal",
          "UCommonActivatableWidget", "UCommonActivatablePanel", "UCommonInputManager", "UFortRestartClientModal", "UCommonInputSubsystem",
          "UCommonActionWidget", "UCommonButton", "UCommonTextBlock", "UFortGlobals", "UFortGlobalUIContext", "UFortUINotification",
          "UCommonLazyImage", "UCommonActivatableWidgetContainerBase", "UFortKismetLibrary"}  # same set as build_call_pool.py


def span(p):
    a = [int(l.split(None, 1)[0], 16) for l in open(p) if re.match(r'^[0-9a-f]+\s', l)]
    return min(a), max(a)


def insns(p):
    return [l.split(None, 1)[1].strip() for l in open(p) if len(l.split(None, 1)) > 1]


out = collections.defaultdict(list)
for e in f2:
    k = (e['cls'], e['fn'])
    if k in done or k not in inscope:
        continue
    name = f"{e['cls']}::{e['fn']}"
    g = None
    if e['group'] == 'I':
        g = '01 interface dispatch (no body at interface level)'
    elif e['group'] == 'U':
        g = '02 no single body (pure virtual/embedded subobject)'
    elif e['cls'] in POLICY:
        g = '12 class is hand-written policy code (generator does not own it)'
    elif k in reg:
        g = '10 recorded blocker/deferral: ' + reg[k]
    else:
        b = e.get('real_bodies') or []
        if not b:
            g = '03 no body address resolved'
        elif len(b) > 1:
            g = '04 several candidate bodies (needs per-function choice)'
        elif b[0] not in idx:
            g = '05 body not captured'
        else:
            L = insns(idx[b[0]])
            tg = [m.group(1) for i in L for m in [re.fullmatch(r'(?:CALL|JMP) (0x[0-9a-f]+)', i)] if m]
            ind = [i for i in L if re.match(r'(CALL|JMP) (qword ptr|R)', i)]
            lo, hi = span(idx[b[0]])
            ext = [t for t in tg if t not in HELPERS and t not in STUBS and not (lo <= int(t, 16) <= hi)]
            unres = [t for t in ext if t not in gp and t not in named]
            if k in attempted:
                g = '11 attempted by a worker, no recorded reason'
            elif len(L) > MAXI:
                g = '20 body too large (%s)' % ('61-120' if len(L) <= 120 else '121-300' if len(L) <= 300 else '301-1000' if len(L) <= 1000 else '>1000')
            elif ind:
                g = '21 indirect calls (virtual/function pointer)'
            elif unres:
                g = '22 calls internal helpers not in reflection (%d unresolved)' % min(len(set(unres)), 3) if len(set(unres)) < 3 else '22 calls internal helpers not in reflection (3+ unresolved)'
            elif (lambda ri: ri is None or ri['extra_calls'] or ri['foreign_virtuals'])(tr.receiver_info(e['thunk'], b[0])):
                g = '23 exec thunk does extra work (extra calls / non-this virtuals)'
            else:
                g = '30 READY: all callees resolvable'
    out[g].append(name)
os.makedirs(f'{R}/remaining', exist_ok=True)
json.dump(out, open(f'{R}/remaining/classification.json', 'w'), indent=0)
tot = sum(len(v) for v in out.values())
print('unported in-scope (table rows):', tot, ' ported:', len(done))
for g in sorted(out):
    print('%5d  %s' % (len(out[g]), g))
