#!/usr/bin/env python3
"""Resolve vtable-call targets in remaining exec thunks, from Ghidra thunk decompiles + flat image.

Read-only. Needs porting/generated/thunk-table/table.json (build_thunk_table.py).
Writes generated/thunk-table/virtual_targets.json, virtual_summary.json, virtual_capture_queue.json.
Site forms handled (var must be param_1 = this):
  primary    (**(code **)(*param_1 + SLOT))
  secondary  (**(code **)(*(longlong *)(param_1 + OFF) + SLOT))   -> needs the secondary vptr (not resolved here)
Targets are read as u64 at class vtable + SLOT from the flat image. Class vtable = native_class_index
vtable_addr, else vtable_candidate. Confidence: high if vtable_status unique and slot < vtable_slots,
medium if candidate/inherited and slot in range, low if slot beyond table size (likely a base/derived mismatch).
"""
import collections, json, mmap, re

IMG = '/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'
BASE = 0x140000000
rows = [r for r in json.load(open('porting/generated/thunk-table/table.json')) if r['kind'] == 'virtual']
ci = json.load(open('porting/generated/reflection/native_class_index.json'))
CV = json.load(open('porting/generated/class-vtables/vtables.json'))  # from resolve_class_vtables.py (ctor-derived)
TEXT_END = 0x145800000  # code lies below the data/vtable area; Ghidra capture is the final check
_fh = open(IMG, 'rb')
mm = mmap.mmap(_fh.fileno(), 0, access=mmap.ACCESS_READ)
SIZE = len(mm)
A = re.compile(r'\(\*\*\(code \*\*\)\(\*(\w+) \+ (0x[0-9a-f]+|\d+)\)\)')
B = re.compile(r'\(\*\*\(code \*\*\)\(\*\(longlong \*\)\((\w+) \+ (0x[0-9a-f]+|\d+)\) \+ (0x[0-9a-f]+|\d+)\)\)')


def num(x):
    return int(x, 16) if x.startswith('0x') else int(x)


def rd(va):
    o = va - BASE
    if 0 <= o <= SIZE - 8:
        return int.from_bytes(mm[o:o + 8], 'little')
    return None


out, reasons, tcount = [], collections.Counter(), collections.Counter()
for r in rows:
    t = open(r['src'], errors='ignore').read().split('*/', 1)[-1]
    sites = []
    for m in A.finditer(t):
        sites.append(('primary', m.group(1), None, num(m.group(2))))
    for m in B.finditer(t):
        sites.append(('secondary', m.group(1), num(m.group(2)), num(m.group(3))))
    info = ci.get(r['cls']) or {}
    cv = CV.get(r['cls']) or {}
    if cv.get('status') == 'ok':
        vt, vst, nsl = cv['primary'], 'ctor', None
    else:
        vt = info.get('vtable_addr') or info.get('vtable_candidate')
        vst = info.get('vtable_status')
        nsl = info.get('vtable_slots') or info.get('vtable_candidate_slots')
    res = []
    for kind, var, off, slot in sites:
        s = dict(kind=kind, var=var, secondary_off=off, slot=slot)
        if var != 'param_1':
            s['status'] = 'other_object'
        elif kind == 'secondary':
            sv = (cv.get('secondary') or {}).get(hex(off))
            if not sv:
                s['status'] = 'secondary_vptr_unknown'
            else:
                tgt = rd(int(sv, 16) + slot)
                ok = tgt is not None and 0x140001000 <= tgt < TEXT_END
                s['status'] = 'resolved' if ok else 'bad_read'
                if ok:
                    s['target'] = hex(tgt)
                    s['confidence'] = 'medium' if vst == 'ctor' else 'low'
        elif not vt:
            s['status'] = 'no_class_vtable'
        else:
            tgt = rd(int(vt, 16) + slot)
            ok = tgt is not None and 0x140001000 <= tgt < TEXT_END
            s['status'] = 'resolved' if ok else 'bad_read'
            if ok:
                s['target'] = hex(tgt)
                s['confidence'] = ('high' if vst == 'ctor' else 'medium' if vst == 'unique' and nsl and slot // 8 < nsl
                                   else 'low')
        reasons[s['status']] += 1
        res.append(s)
    out.append(dict(cls=r['cls'], fn=r['fn'], thunk=r['thunk'], vtable=vt, vtable_status=vst, sites=res))
tgts = collections.Counter()
for o in out:
    for s in o['sites']:
        if s['status'] == 'resolved':
            tgts[s['target']] += 1
tcount = len(tgts)
fn_resolved = sum(1 for o in out if any(s['status'] == 'resolved' for s in o['sites']))
fn_nosite = sum(1 for o in out if not o['sites'])
d = 'porting/generated/thunk-table'
json.dump(out, open(f'{d}/virtual_targets.json', 'w'))
cap_missing = []
import glob, os
have = set()
for dd in glob.glob('porting/generated/native*/'):
    for f in os.listdir(dd):
        m = re.fullmatch(r'(0x[0-9a-f]+)\.c', f)
        if m:
            have.add(m.group(1))
queue = [a for a, _ in tgts.most_common() if a not in have]
json.dump(queue, open(f'{d}/virtual_capture_queue.json', 'w'))
summ = dict(virtual_thunks=len(out), site_status=dict(reasons), functions_with_a_resolved_target=fn_resolved,
            functions_with_no_recognised_site=fn_nosite, distinct_resolved_targets=tcount,
            targets_needing_capture=len(queue), top_targets=tgts.most_common(8))
json.dump(summ, open(f'{d}/virtual_summary.json', 'w'), indent=1)
print(json.dumps(summ))
