#!/usr/bin/env python3
"""Direct call/tail-jump graph of the runtime image. Usage: callgraph.py RUNTIME.exe OUT.json [OBJDUMP_OUTPUT.asm [EXTRA_TARGETS.json: hex address list kept as edge targets]]
Streams `objdump -d`, attributes every instruction to a function via the .pdata table and records
call/jmp edges whose direct target is another function entry. Indirect and virtual calls are not seen."""
import bisect, json, re, struct, subprocess, sys
exe, out = sys.argv[1:3]
asm = sys.argv[3] if len(sys.argv) > 3 else None
b = open(exe, 'rb').read()
pe = struct.unpack_from('<I', b, 0x3c)[0]
ns = struct.unpack_from('<H', b, pe + 6)[0]
so = struct.unpack_from('<H', b, pe + 20)[0]
base = struct.unpack_from('<Q', b, pe + 24 + 24)[0]
t = pe + 24 + so
sec = {}
for i in range(ns):
    n = b[t + i * 40:t + i * 40 + 8].rstrip(b'\0').decode(); vs, va, rs, ro = struct.unpack_from('<IIII', b, t + i * 40 + 8)
    sec.setdefault(n, []).append((va, vs, ro, rs))
va, vs, ro, rs = sec['.pdata'][0]
starts, ends = [], []
for o in range(ro, ro + rs, 12):
    s, e, _ = struct.unpack_from('<III', b, o)
    if s:
        starts.append(base + s); ends.append(base + e)
extra = {int(a, 16) for a in json.load(open(sys.argv[4]))} if len(sys.argv) > 4 else set()
starts = sorted(set(starts) | extra)  # UFunction thunks and bodies split larger ranges
# The dump's .pdata end addresses are not reliable, so a function extends to the next function start.
ends = starts[1:] + [starts[-1] + 0x10000]
print(len(starts), 'functions', file=sys.stderr)
startset = set(starts)
if asm:
    class _P: stdout = open(asm, errors='replace', buffering=1 << 22)
    p = _P()
else:
    p = subprocess.Popen(['objdump', '-d', '-M', 'intel', '--no-show-raw-insn', exe], stdout=subprocess.PIPE, text=True, errors='replace', bufsize=1 << 20)
rx = re.compile(r'^\s*([0-9a-f]+):\s+(call|jmp)\s+0x([0-9a-f]+)\b')
edges = set(); cur = None; cur_end = 0; n = 0
for line in p.stdout:
    m = rx.match(line)
    if not m:
        continue
    a = int(m.group(1), 16); tgt = int(m.group(3), 16)
    if not (cur is not None and cur <= a < cur_end):
        i = bisect.bisect_right(starts, a) - 1
        if i < 0 or a >= ends[i]:
            cur = None; continue
        cur, cur_end = starts[i], ends[i]
    if tgt in startset and tgt != cur:
        edges.add((cur, tgt))
    n += 1
    if n % 2000000 == 0:
        print(n, len(edges), file=sys.stderr)
json.dump(sorted(edges), open(out, 'w'))
print('edges', len(edges), file=sys.stderr)
