#!/usr/bin/env python3
"""Read-only capture and reflection gate for the three proposed mutator bodies."""
import hashlib, json, struct
from pathlib import Path
R = Path(__file__).resolve().parents[3]
P = Path(__file__).resolve().parent
image = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
with image.open('rb') as f:
    assert hashlib.file_digest(f, 'sha256').hexdigest() == '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
    def raw(a, n):
        f.seek(a - 0x140000000)
        return f.read(n)
    for a, h in {
        0x1427df000:'488b014885c0740b488bd1488bc8e98d9928ffc3',
        0x141a689a0:'40534883ec20488bd9e8d2986600488d05a3bd3d04c7836802000000000000488903488d0547ca3d0448898318020000488bc366c7835002000000004883c4205bc3',
        0x14280aeb0:'488b014885c0740b488bd1488bc8e95d3a2effc3',
        0x141aee920:'40534883ec20488bd9e872a0f7ff488d05db283b04c6835102000001488903488d056a81360448898318020000488bc3c68370020000004883c4205bc3',
        0x1406801c0:'c20000', 0x1406801e0:'32c0c3',
        0x141aa75e0:'0fb681700200008802b002c3',
        0x1420e5530:'f6814802000001750cf6812002000001750332c0c3b001c3',
    }.items():
        assert raw(a, len(bytes.fromhex(h))) == bytes.fromhex(h), hex(a)
    q = lambda a: struct.unpack('<Q', raw(a, 8))[0]
    assert q(0x145e44758+0x6b8) == 0x1406801c0
    assert q(0x145e44758+0x9b8) == 0x1406801e0
    assert q(0x145ea1210+0x9b8) == 0x141aa75e0
bits = json.loads((R/'porting/generated/reflection/bool_bits.json').read_text())
# The file wraps classes under a top-level key.
def find_dict(o, key):
    if isinstance(o, dict):
        if key in o: return o[key]
        for v in o.values():
            r = find_dict(v, key)
            if r is not None: return r
    return None
for cls, name, offset in [('AFortGameplayMutator','bMutatorActive',0x220),('AFortGameplayMutator','bMutatorActiveInternal',0x248),('AFortAthenaMutator_InfiniteAmmo','bInfiniteAmmo',0x270)]:
    entry = next(x for x in find_dict(bits, cls) if x['name'] == name)
    assert entry['offset'] == offset and entry['mask'] == 1
entries = json.loads((P/'manifest-entries.json').read_text())
assert [(x['class'], x['function'], x['address']) for x in entries] == [
    ('AFortAthenaMutator','NativeSetMutatorContext','0x1406801c0'),
    ('AFortAthenaMutator','NativeQueryInfiniteAmmo','0x1406801e0'),
    ('AFortAthenaMutator_InfiniteAmmo','NativeQueryInfiniteAmmo','0x141aa75e0')]
assert all((R/x['inc_path']).is_file() for x in entries)
print('Pinned mutator callback evidence passed')

# Reviewed source bodies pinned for promotion.
assert hashlib.sha256((P/'BaseQueryInfiniteAmmo.inc').read_bytes()).hexdigest() == '25e1b1a3d1b6ee8eec9ec53e2b131c85cd00bbaaca1d50ed662304593a89d391'
assert hashlib.sha256((P/'BaseSetMutatorContext.inc').read_bytes()).hexdigest() == 'a544bd331722914a131beee84a09a6b0fa94c1e2953fe0c354481fb2eea73233'
assert hashlib.sha256((P/'InfiniteAmmoQuery.inc').read_bytes()).hexdigest() == '2e3c99fad9709035e6f9d604c41a5e649c66d5ef287e889c0b35f7449133da0b'
