"""Absolute data operands of a body with their values (read-only helper). Run from the repo root.

consts(L): for every `[0x14xxxxxxx]` / `LEA R,[0x14xxxxxxx]` absolute operand outside the code range, read 8 bytes from the runtime
image (RVA = addr - 0x140000000). Section classes come from the PE table: `rdata` operands are true constants (use the value as a
literal); `data` operands are writable globals - the value is only the capture-time snapshot (use it only when it is plainly an
engine constant such as 0.0f vectors / FVector::ZeroVector)."""
import re, struct
PE = '/mnt/vibrato/Fortnite/FortniteGame/Binaries/Win64/FortniteClient-Win64-Shipping.exe'
IMG = '/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'
BASE = 0x140000000
_d = open(PE, 'rb').read(0x1000)
_pe = struct.unpack_from('<I', _d, 0x3c)[0]
_n = struct.unpack_from('<H', _d, _pe + 6)[0]
_o = struct.unpack_from('<H', _d, _pe + 20)[0]
SECS = []
for i in range(_n):
    nm, vs, va, rs, ro = struct.unpack_from('<8sIIII', _d, _pe + 24 + _o + i * 40)
    SECS.append((nm.rstrip(b'\0').decode(), va, vs))
_img = open(IMG, 'rb')


def sec(addr):
    r = addr - BASE
    for nm, va, vs in SECS:
        if va <= r < va + vs:
            return nm
    return None


def consts(L):
    out = {}
    for i in L:
        for m in re.finditer(r'\[(0x14[0-9a-f]{7})\]', i):
            a = int(m.group(1), 16)
            s = sec(a)
            if s in (None, '.text'):
                continue
            _img.seek(a - BASE)
            b = _img.read(8)
            if len(b) < 8:
                continue
            u64 = struct.unpack('<Q', b)[0]
            out[m.group(1)] = dict(section=s, kind='constant' if s == '.rdata' else 'writable_global_snapshot',
                                   u32=hex(u64 & 0xffffffff), f32=struct.unpack('<f', b[:4])[0], u64=hex(u64), f64=struct.unpack('<d', b)[0])
    return out
