#!/usr/bin/env python3
"""Validate bounded original activation witnesses; no whole-image read/hash."""
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
IMAGE = '/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'


def main():
    policy = json.loads((HERE / 'original-policy.json').read_text())
    base = int(policy['image_base'], 16)
    fd = os.open(IMAGE, os.O_RDONLY)
    total = 0
    try:
        for item in policy['windows']:
            size = item['size']
            assert 0 < size <= 0x400, item
            data = os.pread(fd, size, int(item['address'], 16) - base)
            assert len(data) == size, item['label']
            assert hashlib.sha256(data).hexdigest() == item['sha256'], item['label']
            total += size
        for slot in policy['vtable_slots']:
            address = int(slot['vtable'], 16) + slot['offset']
            value = int.from_bytes(os.pread(fd, 8, address - base), 'little')
            assert value == int(slot['target'], 16), slot
    finally:
        os.close(fd)
    print(f"PASS: {len(policy['windows'])} bounded activation windows "
          f"({total} bytes), {len(policy['vtable_slots'])} virtual+30 slots")


if __name__ == '__main__':
    main()
