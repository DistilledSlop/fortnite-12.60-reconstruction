#!/usr/bin/env python3
"""Recover each class's real vtable(s) from its constructor (not the unreliable class-index candidates).

Chain per class (native_class_index gpsc_targets[0]):
  GetPrivateStaticClass  -> stack arg 10 (stored at [R11-0x40]) = InternalConstructor<C>
  InternalConstructor<C> -> its direct callee = C::C(FObjectInitializer const&)   (or it is the ctor)
  ctor                   -> LEA reg,[vtbl]; MOV [reg2],reg  (displacement 0) = primary vptr (last store wins);
                            stores with displacement = secondary vptrs (offset -> vtable)
Captures missing addresses with porting/ghidra/probe_callees.py (serial). Read-only on the repo apart from
generated/ output. Usage (repo root):
  python3 -I porting/resolve_class_vtables.py [--classes-from virtual|all]
Output: porting/generated/class-vtables/vtables.json, summary.json
"""
import argparse, collections, glob, json, os, re, subprocess, sys

ROOT = 'porting/generated'
OUT = f'{ROOT}/class-vtables'
PY = 'porting/.venv/bin/python'
os.makedirs(OUT, exist_ok=True)


def cap_index():
    idx = {}
    for d in glob.glob(f'{ROOT}/native*/'):
        for f in os.listdir(d):
            m = re.fullmatch(r'(0x[0-9a-f]+)\.asm', f)
            if m:
                idx.setdefault(m.group(1), d + f)
    return idx


def capture(addrs, tag):
    if not addrs:
        return
    lst = f'{OUT}/{tag}.json'
    json.dump(sorted(addrs), open(lst, 'w'))
    subprocess.run([PY, '-I', 'porting/ghidra/probe_callees.py', '--input', lst, '--out', f'{ROOT}/native-cap-{tag}',
                    '--timeout', '120', '--mem', '24G'], check=True, stderr=subprocess.DEVNULL)


INS = re.compile(r'^([0-9a-f]+)\s+(.*)$')
LEA = re.compile(r'LEA (\w+),\[(0x[0-9a-f]+)\]')
STORE = re.compile(r'MOV qword ptr \[(\w+)(?: \+ (0x[0-9a-f]+))?\],(\w+)$')
R11STORE = re.compile(r'MOV qword ptr \[R11 \+ -0x40\],(\w+)$')


def lines(path):
    return [INS.match(l.rstrip()).group(2) for l in open(path) if INS.match(l.rstrip())]


def internal_ctor(path):
    """Arg 10 of GetPrivateStaticClassBody (InternalConstructor<C>). Call-time RSP = entry RSP - N (N from
    SUB RSP,N); arg k>=5 lives at call RSP + 0x20 + 8*(k-5) = [RSP+0x48] for k=10, or [R11 - (N-0x48)]
    when the function copied the entry RSP into R11."""
    L = lines(path)
    n = None
    for ins in L:
        m = re.match(r'SUB RSP,(0x[0-9a-f]+)', ins)
        if m:
            n = int(m.group(1), 16)
            break
    if n is None:
        return None
    r11 = 'MOV R11,RSP' in L[:2]
    pats = [re.compile(r'MOV qword ptr \[RSP \+ 0x48\],(\w+)$')]
    if r11:
        pats.append(re.compile(r'MOV qword ptr \[R11 \+ -(0x[0-9a-f]+)\],(\w+)$'))
    regs = {}
    for ins in L:
        m = LEA.match(ins)
        if m:
            regs[m.group(1)] = m.group(2)
            continue
        m = pats[0].match(ins)
        if m and m.group(1) in regs:
            return regs[m.group(1)]
        if r11:
            m = pats[1].match(ins)
            if m and int(m.group(1), 16) == n - 0x48 and m.group(2) in regs:
                return regs[m.group(2)]
        m = re.match(r'(?:MOV|XOR|ADD|SUB|MOVZX) (\w+),', ins)
        if m and not LEA.match(ins):
            regs.pop(m.group(1), None)
    return None


def ctor_vptrs(path):
    """-> (primary, {off: vtable}). Only stores through registers aliasing `this` (RCX on entry, copied to
    non-volatile regs) count; stores through other registers are member subobjects."""
    VOL = {'RAX', 'RCX', 'RDX', 'R8', 'R9', 'R10', 'R11'}
    regs, prim, sec, this = {}, None, {}, {'RCX'}
    for ins in lines(path):
        if ins.startswith('CALL'):
            this -= VOL
            for r in VOL:
                regs.pop(r, None)
            continue
        m = LEA.match(ins)
        if m:
            regs[m.group(1)] = m.group(2)
            this.discard(m.group(1))
            continue
        m = STORE.match(ins)
        if m:
            if m.group(1) in this and m.group(3) in regs:
                off = int(m.group(2), 16) if m.group(2) else 0
                if off == 0:
                    prim = regs[m.group(3)]
                else:
                    sec[off] = regs[m.group(3)]
            continue
        m = re.match(r'MOV (\w+),(\w+)$', ins)
        if m:
            d, s = m.group(1), m.group(2)
            regs.pop(d, None)
            if s in this:
                this.add(d)
            else:
                this.discard(d)
            continue
        m = re.match(r'(?:MOV|MOVZX|MOVSX|XOR|ADD|SUB|POP|IMUL|AND|OR|SHL|SHR|SAR|NEG|NOT|INC|DEC|CMOV\w+) (\w+),', ins)
        if m:
            regs.pop(m.group(1), None)
            this.discard(m.group(1))
    return prim, sec


def callees(addr):
    for d in glob.glob(f'{ROOT}/native*/{addr}.json'):
        try:
            return [c['address'] for c in json.load(open(d)).get('called', [])]
        except Exception:
            pass
    return []


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--classes-from', default='virtual')
    a = ap.parse_args()
    ci = json.load(open(f'{ROOT}/reflection/native_class_index.json'))
    if a.classes_from == 'virtual':
        want = {r['cls'] for r in json.load(open(f'{ROOT}/thunk-table/table.json')) if r['kind'] == 'virtual'}
    else:
        want = {c for c, v in ci.items() if isinstance(v, dict) and v.get('gpsc_targets')}
    cls = {c: ci[c]['gpsc_targets'][0] for c in sorted(want) if ci.get(c, {}).get('gpsc_targets')}
    res = {c: dict(gpsc=g) for c, g in cls.items()}
    idx = cap_index()
    capture({g for g in cls.values() if g not in idx}, 'gpsc')
    idx = cap_index()
    for c, g in cls.items():
        if g in idx:
            res[c]['internal_ctor'] = internal_ctor(idx[g])
    ic = {r['internal_ctor'] for r in res.values() if r.get('internal_ctor')}
    capture({x for x in ic if x not in idx}, 'ictor')
    idx = cap_index()
    for c, r in res.items():
        ic_ = r.get('internal_ctor')
        if not ic_ or ic_ not in idx:
            continue
        co = callees(ic_)
        jt = [m.group(1) for ins in lines(idx[ic_]) for m in [re.match(r'J\w+ (0x[0-9a-f]+)$', ins)] if m]
        co = list(dict.fromkeys(co + [j for j in jt if int(j, 16) < int(ic_, 16) or int(j, 16) > int(ic_, 16) + 0x200]))
        # the class ctor is the (single non-allocator) callee; try each until a vptr store is found later
        r['ictor_callees'] = co
    need = set()
    for r in res.values():
        need.update(r.get('ictor_callees', []))
    capture({x for x in need if x not in idx}, 'ctor')
    idx = cap_index()
    st = collections.Counter()
    for c, r in res.items():
        r['status'] = 'no_gpsc_capture'
        if 'internal_ctor' not in r or not r['internal_ctor']:
            r['status'] = 'no_internal_ctor'
            st[r['status']] += 1
            continue
        best = None
        for ca in [r['internal_ctor']] + r.get('ictor_callees', []):
            if ca in idx:
                prim, sec = ctor_vptrs(idx[ca])
                if prim:
                    best = (ca, prim, sec)
        if best:
            r.update(ctor=best[0], primary=best[1], secondary={hex(k): v for k, v in best[2].items()}, status='ok')
        else:
            r['status'] = 'no_vptr_store_found'
        st[r['status']] += 1
    json.dump(res, open(f'{OUT}/vtables.json', 'w'))
    summ = dict(classes=len(res), status=dict(st))
    for known, want_v in (('AFortAthenaVehicle', '0x145debe20'), ('ABuildingTrap', '0x145f46db0')):
        summ[known] = dict(found=res.get(known, {}).get('primary'), expected=want_v,
                           secondary=res.get(known, {}).get('secondary'))
    json.dump(summ, open(f'{OUT}/summary.json', 'w'), indent=1)
    print(json.dumps(summ))


main()
