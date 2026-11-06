#!/usr/bin/env python3
"""Read-only capture gate for provider registration, tables and recovered bodies."""
import hashlib
from pathlib import Path

IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
BASE = 0x140000000
WINDOWS = [
    (0x1463cf238, 16, '43732812878f15a3e3af3494690ac5be1bb218daa846f56678f403f983ed7783'),
    (0x14192db54, 14, 'cd3f967aa08c452725d0371791cb86501544b91b6921d132c0377be3d033ff0b'),
    (0x145dfc210, 8, '4d6ad018ddeefe22d65f00a66bed558c946c2edec2cc239212d3d3d11401235f'),
    (0x141948340, 138, '92e2fbcc4163ff09eeb609288cbb6b9c83521fb79a8d8c2af571101654db0e09'),
    (0x1463ea120, 16, '60d6a59f1e2f3a94094ce5726138809313a9a948fd300a3d2d2d4a2b13be2a85'),
    (0x14197f7fb, 14, '72f7756688b3d6f24ebd3e7b221df108724a13b6e2f1def298c36aca5b5ef8a0'),
    (0x145e0b720, 8, '317b27b46b452161968fd9d4cd7c97d7d98d4b5f8ac81f6ae2ee1ae24edcfd12'),
    (0x141995780, 138, 'ba54f63c5b1a59f2fbdd277d7358c34b068d85a0e060f99e183b028e3dd6428a'),
    (0x1464d3e38, 16, '4597f7ed30ecace0678e51e91ddc9eb2705f97233b71424813dd329e63b60f3e'),
    (0x141a2e10d, 14, '72a33950be66b6c42b291acad89e64d382417a182e67decfd20568944424a710'),
    (0x145e368d0, 8, '6147bdc3712365f0a9f1e295df886f7f3b34aa25075536b0a3fd413e8cba845d'),
    (0x141a45030, 170, 'c06bb71dcbeffc9424a5882e61d376c5a151c30effcdf3966dcbcd8f866fb845'),
]
with IMAGE.open('rb') as image:
    for address, length, expected in WINDOWS:
        image.seek(address - BASE)
        actual = hashlib.sha256(image.read(length)).hexdigest()
        if actual != expected:
            raise SystemExit(f'capture mismatch at {address:#x}: {actual}')
print(f'{len(WINDOWS)} capture windows verified')
