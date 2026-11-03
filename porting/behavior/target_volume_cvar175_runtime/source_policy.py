"""Emit the original target-selection CVar at module load, before gameplay queries."""
import hashlib
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent

def sources():
    evidence = json.loads((HERE / 'native-evidence.json').read_text())
    with Path(evidence['image']).open('rb') as stream:
        for window in evidence['windows']:
            stream.seek(int(window['address'], 16) - 0x140000000)
            if hashlib.sha256(stream.read(window['length'])).hexdigest() != window['sha256']:
                raise ValueError('target-selection CVar evidence differs from original bytes')
    return {name: (HERE / name).read_text() for name in
            ('TargetSelectionVolumeNative.h', 'TargetSelectionVolumeNative.cpp')}
