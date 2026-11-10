#!/usr/bin/env python3
"""Find 8-byte pointers to UFunction bodies in .rdata/.data/_RDATA (vtables, function-pointer tables, delegates).
Usage: data_refs.py RUNTIME.exe THUNK_TABLE final3.json OUT.json"""
import array, collections, json, struct, sys
exe, tt, out = sys.argv[1:4]
b = open(exe, 'rb').read()
thunks, bodies = set(), set()
for r in json.load(open(tt)):
    thunks.add(int(r['thunk'], 16))
    bodies.update(int(y, 16) for y in (r.get('real_bodies') or []) if isinstance(y, str))
want = bodies - thunks
pe = struct.unpack_from('<I', b, 0x3c)[0]; ns = struct.unpack_from('<H', b, pe + 6)[0]; so = struct.unpack_from('<H', b, pe + 20)[0]; t = pe + 24 + so
found = collections.defaultdict(int)
for i in range(ns):
    n = b[t + i * 40:t + i * 40 + 8].rstrip(b'\0').decode(); vs, va, rs, ro = struct.unpack_from('<IIII', b, t + i * 40 + 8)
    if n in ('.rdata', '.data', '_RDATA'):
        a = array.array('Q'); a.frombytes(b[ro:ro + rs // 8 * 8])
        for v in a:
            if v in want:
                found[v] += 1
json.dump({hex(k): v for k, v in found.items()}, open(out, 'w'))
print(len(want), 'bodies,', len(found), 'referenced from data', file=sys.stderr)
