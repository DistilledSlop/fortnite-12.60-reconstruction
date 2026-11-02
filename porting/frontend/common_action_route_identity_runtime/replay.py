#!/usr/bin/env python3
"""Replay bounded original-image identity witnesses; never loads the whole image."""
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
IMAGE = '/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'


def main():
    policy = json.loads((HERE / 'original-policy.json').read_text())
    fd = os.open(IMAGE, os.O_RDONLY)
    try:
        for window in policy['windows']:
            address, size = int(window['address'], 16), window['size']
            data = os.pread(fd, size, address - 0x140000000)
            assert len(data) == size, window['label']
            assert hashlib.sha256(data).hexdigest() == window['sha256'], window['label']
        for slot in policy['vtable_slots']:
            address = int(slot['vtable'], 16) + slot['offset']
            value = int.from_bytes(os.pread(fd, 8, address - 0x140000000), 'little')
            assert value == int(slot['target'], 16), slot
        name = os.pread(fd, 42, 0x145a08ae2 - 0x140000000).decode('utf-16le')
        assert name.rstrip('\0') == 'CommonUIActionRouter'
    finally:
        os.close(fd)
    print(f"PASS: {len(policy['windows'])} bounded identity windows, "
          f"{len(policy['vtable_slots'])} original slots, router class literal")


if __name__ == '__main__':
    main()
