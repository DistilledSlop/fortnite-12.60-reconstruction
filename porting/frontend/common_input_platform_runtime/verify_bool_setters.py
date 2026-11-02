"""Verify bounded original Bool setter windows, not an entire image hash."""
import hashlib
import json
import struct
import sys
from pathlib import Path

records = json.loads(Path(__file__).with_name('bool-setters.json').read_text())
with Path(sys.argv[1]).open('rb') as image:
    for record in records:
        image.seek(int(record['property_record'], 16) + 48 - 0x140000000)
        pointer = struct.unpack('<Q', image.read(8))[0]
        if pointer != int(record['setter'], 16):
            raise SystemExit('Bool setter pointer changed: ' + record['name'])
        image.seek(pointer - 0x140000000)
        if hashlib.sha256(image.read(record['size'])).hexdigest() != record['sha256']:
            raise SystemExit('Bool setter changed: ' + record['name'])
print('PASS: 5 bounded original Bool setter records/windows')
