#!/usr/bin/env python3
"""Per-thunk receiver adjustment and inlined-logic hints, from the captured thunk asm (read-only helper module).

receiver_info(thunk) -> dict or None (no asm captured):
  delta       int   bytes added to `this` before the body/vtable call (ADD RCX,N or LEA RCX,[R+N] where R aliases this;
                    taken from the LAST such write before the final call). Body offsets are relative to this+delta+wrapper.
  extra_calls list  call targets that are neither the shared exec helpers nor the final call ('V' = virtual/indirect call).
  helper_virtuals int  count of indirect calls (struct/array parameter decoding makes some benign: judge by use).
  logic_hint  bool  True when extra non-helper calls exist: the thunk may inline logic beyond forwarding to the body.
Used by build_work_packets.py and audit_receiver_adjust.py. Cache: generated/thunk-table/receiver.json (rebuilt with --rebuild).
Run: python3 -I porting/thunk_receiver.py --rebuild   (repo root)"""
import glob, json, os, re, sys

R = 'porting/generated'
CACHE = f'{R}/thunk-table/receiver.json'
HELPER_MIN = 150


def _idx():
    idx = {}
    for d in glob.glob(f'{R}/native*/'):
        for f in os.listdir(d):
            m = re.fullmatch(r'(0x[0-9a-f]+)\.asm', f)
            if m:
                idx.setdefault(m.group(1), d + f)
    return idx


def _parse(path, thunk):
    """-> (receiver_delta, calls[list of target or 'V' in order])"""
    this = {'RCX'}
    delta = 0
    calls = []
    last_delta = 0
    rcx_this = True  # does RCX currently hold this (+delta)?
    for l in open(path):
        m = re.match(r'^[0-9a-f]+\s+(.*)$', l.rstrip())
        if not m:
            continue
        i = m.group(1)
        a = re.fullmatch(r'MOV (R\w+),RCX', i)
        if a:
            if rcx_this:
                this.add(a.group(1))
            continue
        a = re.fullmatch(r'(ADD|SUB) RCX,(0x[0-9a-f]+)', i)
        if a and 'RCX' in this:
            d = int(a.group(2), 16) * (1 if a.group(1) == 'ADD' else -1)
            delta += d
            last_delta = delta
            continue
        a = re.fullmatch(r'LEA RCX,\[(R\w+) \+ (-?0x[0-9a-f]+)\]', i)
        if a and a.group(1) in this:
            last_delta = int(a.group(2), 16)
            delta = last_delta
            rcx_this = True
            continue
        a = re.fullmatch(r'MOV RCX,(R\w+)', i)
        if a:
            rcx_this = a.group(1) in this
            delta = 0 if rcx_this else delta
            continue
        if re.match(r'(MOV|LEA|XOR) (RCX|ECX),', i):
            rcx_this = False
            continue
        a = re.match(r'(CALL|JMP) (0x[0-9a-f]+)$', i)
        if a and a.group(1) == 'JMP' and abs(int(a.group(2), 16) - int(thunk, 16)) < 0x800:
            continue  # local branch inside the thunk, not a tail call
        if a:
            calls.append((a.group(1), a.group(2), delta))
            rcx_this = False
            continue
        if re.match(r'CALL (qword ptr .*|R\w+)$', i):
            calls.append(('CALL', 'V' if rcx_this else 'F', delta))
            rcx_this = False
        elif a:
            rcx_this = False
    return calls


def build():
    idx = _idx()
    table = json.load(open(f'{R}/thunk-table/table.json'))
    freq = {}
    parsed = {}
    for r in table:
        t = r['thunk']
        if t in parsed or t not in idx:
            continue
        parsed[t] = _parse(idx[t], t)
        for x in {c[1] for c in parsed[t] if c[1] not in ('V', 'F')}:
            freq[x] = freq.get(x, 0) + 1
    helpers = {x for x, n in freq.items() if n >= HELPER_MIN}
    out = {}
    for t, calls in parsed.items():
        non_helper = [c for c in calls if c[1] not in helpers]
        final = non_helper[-1] if non_helper else None
        out[t] = dict(delta=final[2] if final else 0,
                      calls=[c[1] for c in non_helper],
                      helper_virtuals=sum(1 for c in calls if c[1] == 'V'),
                      foreign_virtuals=sum(1 for c in calls if c[1] == 'F'))
    os.makedirs(os.path.dirname(CACHE), exist_ok=True)
    json.dump(dict(helpers=sorted(helpers), thunks=out), open(CACHE, 'w'))
    return out


_c = None


def receiver_info(thunk, body=None):
    global _c
    if _c is None:
        _c = json.load(open(CACHE))['thunks'] if os.path.exists(CACHE) else build()
    e = _c.get(thunk)
    if e is None:
        return None
    extra = [c for c in e['calls'] if c != body and c not in ('V', 'F')]
    return dict(delta=e['delta'], extra_calls=extra, helper_virtuals=e['calls'].count('V'),
                foreign_virtuals=e.get('foreign_virtuals', 0), logic_hint=bool(extra) or bool(e.get('foreign_virtuals')))


if __name__ == '__main__':
    if '--rebuild' in sys.argv or not os.path.exists(CACHE):
        o = build()
        print(len(o), 'thunks analysed;', sum(1 for v in o.values() if v['delta']), 'adjust this')
