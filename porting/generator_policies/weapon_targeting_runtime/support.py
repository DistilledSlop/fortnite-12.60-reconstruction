"""Backed original console settings used by the recovered targeting bodies."""
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent


def sources():
    evidence = json.loads((HERE / 'native-evidence.json').read_text())
    for source_key, windows_key in [('image', 'source_windows'), ('pe', 'pe_windows')]:
        with Path(evidence[source_key]).open('rb') as stream:
            for window in evidence[windows_key]:
                stream.seek(int(window['offset'], 0))
                data = stream.read(window['length'])
                if hashlib.sha256(data).hexdigest() != window['sha256']:
                    raise ValueError('weapon targeting evidence changed: ' + window['address'])
    return {filename: (HERE / filename).read_text() for filename in
            ('FortWeaponTargetingNative.h', 'FortWeaponTargetingNative.cpp')}
