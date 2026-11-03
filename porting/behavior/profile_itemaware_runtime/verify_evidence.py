#!/usr/bin/env python3
"""Read-only evidence gate; no build or runtime execution."""
import hashlib
from pathlib import Path

image = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
base = 0x140000000
windows = {
    0x142024010: '40534883ec20488bd9e84293e90033c948c743600200000080a380000000fc488d05ba830404488903488d057888040448894328488bc348894b3048894b3848894b4048894b4848894b5048894b5848894b6848894b70c74378ffffffff894b7c4883c4205bc3',
    0x141111830: 'e93bffffff',
    0x1464134a0: '30181141010000002800000000000000',
    0x14606c8b8: '50610242010000003054694001000000e001684001000000',
    0x140695430: '488d41d8c3',
    0x1406801e0: '32c0c3',
    0x142041fc0: '48895c2408574883ec20488bd9e83e20feff33ff48c783b000000002000000488d054a8603044889bba0000000488903488d8be0000000488d05428b03044889bba800000048894328',
    0x14607ab40: 'fc790442010000003054694001000000400e064201000000',
}
h = hashlib.sha256()
with image.open('rb') as f:
    while block := f.read(1024 * 1024):
        h.update(block)
    assert h.hexdigest() == '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
    for addr, expected in windows.items():
        data = bytes.fromhex(expected)
        f.seek(addr - base)
        assert f.read(len(data)) == data, hex(addr)
print(f'Image SHA-256 and {len(windows)} exact windows verified')
