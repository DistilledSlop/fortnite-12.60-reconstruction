#!/usr/bin/env python3
"""Validate only bounded original-image windows; never read/hash the whole image."""
import hashlib
import json
import os
from pathlib import Path


def main():
    policy = json.loads(Path(__file__).with_name('original-windows.json').read_text())
    base = int(policy['base'], 16)
    fd = os.open(policy['image'], os.O_RDONLY)
    try:
        for window in policy['windows']:
            size = int(window['size'], 16)
            assert 0 < size <= 0x2000
            data = os.pread(fd, size, int(window['start'], 16) - base)
            assert len(data) == size, window['name']
            assert hashlib.sha256(data).hexdigest() == window['sha256'], window['name']
    finally:
        os.close(fd)
    print(f"PASS: {len(policy['windows'])} bounded route hierarchy/source-join windows")


if __name__ == '__main__':
    main()
