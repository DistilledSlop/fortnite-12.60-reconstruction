import re, subprocess, struct, bisect, json, sys
B = 'FortniteLinuxServer'
secs = []
for l in subprocess.check_output(['readelf', '-SW', B]).decode().splitlines():
    m = re.match(r'\s*\[\s*\d+\]\s+(\S+)\s+(\S+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)', l)
    if m and m.group(2) == 'PROGBITS': secs.append((int(m.group(3), 16), int(m.group(4), 16), int(m.group(5), 16)))
f = open(B, 'rb')
def rd(va, n=8):
    for a, o, s in secs:
        if a <= va < a + s:
            f.seek(o + va - a); return f.read(n)
S = '/tmp/claude-1000/-opt-fortnite/46cd2cbf-c73d-4053-ba19-142367ff8af0/scratchpad/'
syms = []
for l in open(S + 'syms_text.txt'):
    a, t, n = l.rstrip('\n').split(' ', 2); syms.append((int(a, 16), n))
syms.sort(); keys = [s[0] for s in syms]
def sym(a):
    i = bisect.bisect_right(keys, a) - 1
    return syms[i][1] if i >= 0 and keys[i] == a else None
vt = {}
for l in open(S + 'vt_syms.txt'):
    a, sz, t, n = l.split(' ', 3); vt[n.strip()[len('vtable for '):]] = (int(a, 16), int(sz, 16))
out = {}
for c, (a, sz) in vt.items():
    d = rd(a + 16, sz - 16)
    if not d or len(d) < 8: continue
    out[c] = [sym(x) for x in struct.unpack('<%dQ' % (len(d) // 8), d[:len(d) // 8 * 8])]
json.dump(out, open(S + 'engine_vt.json', 'w'))
print(len(out), out.get('AGameMode', [])[:6])
