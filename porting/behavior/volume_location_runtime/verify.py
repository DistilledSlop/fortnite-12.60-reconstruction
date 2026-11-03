"""Verify original capture windows; does not execute the candidate behavior."""
from pathlib import Path
import hashlib
import json

here = Path(__file__).resolve().parent
image = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
with image.open('rb') as stream:
    for item in json.loads((here / 'evidence.json').read_text())['windows']:
        stream.seek(int(item['address'], 16) - 0x140000000)
        assert hashlib.sha256(stream.read(item['length'])).hexdigest() == item['sha256'], item['address']
print('Three volume location capture windows verified')
