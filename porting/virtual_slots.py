"""Name a vtable slot by address (read-only helper). Run from the repo root.

virt_target(cls, slot): read the slot of cls's primary vtable from the runtime image and return the body address.
virt_name(cls, slot): the reflected function (declaring class, name) whose real body is that address, looked up along the
class chain. Slots whose target is a shared return stub, or no reflected function, stay unnamed (None)."""
import collections, json, mmap, struct
R = 'porting/generated'
IMG = '/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'
BASE = 0x140000000
STUB = {'0x1406801c0', '0x1406801d0', '0x1406801e0', '0x140680300'}
_f = open(IMG, 'rb')
_mm = mmap.mmap(_f.fileno(), 0, access=mmap.ACCESS_READ)
VT = json.load(open(f'{R}/class-vtables/vtables.json'))
PARENTS = {v['name']: v.get('chain') or [v['name']] for v in json.load(open(f'{R}/reflection/class_parent_map.json')).values()}
_f2 = json.load(open(f'{R}/thunk-table/final3.json'))
_real = {}
_named = collections.defaultdict(set)
for e in _f2:
    for r in e.get('resolved', []):
        _real[r['entry']] = r['real']
    for b in e.get('real_bodies') or []:
        _named[b].add((e['cls'], e['fn']))


def virt_target(cls, slot):
    v = VT.get(cls)
    if not v or v.get('status') != 'ok' or not v.get('primary'):
        return None
    off = int(v['primary'], 16) + slot - BASE
    if off < 0 or off + 8 > len(_mm):
        return None
    a = struct.unpack_from('<Q', _mm, off)[0]
    return hex(a) if BASE <= a < BASE + 0x6000000 else None


def virt_name(cls, slot):
    chain = PARENTS.get(cls, [cls])
    for c in chain:
        t = virt_target(c, slot)
        if not t or t in STUB:
            continue
        real = _real.get(t, t)
        if real in STUB:
            continue
        names = [n for n in _named.get(real, ()) if n[0] in chain]
        if len(names) == 1:
            return names[0]
        if len(names) > 1:
            names.sort(key=lambda n: chain.index(n[0]))
            return names[0]
    return None
