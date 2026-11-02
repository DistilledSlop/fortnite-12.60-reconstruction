"""Check only pinned original instruction windows; does not prove interpretation."""
import hashlib
import json
import sys
from pathlib import Path

record = json.loads(Path(__file__).with_name('windows.json').read_text())
with Path(sys.argv[1]).open('rb') as image:
    for window in record['windows']:
        image.seek(int(window['va'], 16) - 0x140000000)
        data = image.read(window['size'])
        if len(data) != window['size'] or hashlib.sha256(data).hexdigest() != window['sha256']:
            raise SystemExit('original window mismatch: ' + window['va'])
print('PASS: 4 bounded original instruction windows')
