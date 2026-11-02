"""Bounded original-source gate for the unregistered route operation candidate."""
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())


def validate_original_image(path):
    fd = os.open(path, os.O_RDONLY)
    try:
        for item in CONTRACT['windows']:
            data = os.pread(fd, item['length'], int(item['va'], 16) - 0x140000000)
            if len(data) != item['length'] or hashlib.sha256(data).hexdigest() != item['sha256']:
                raise ValueError('original route operation mismatch: ' + item['va'])
    finally:
        os.close(fd)
