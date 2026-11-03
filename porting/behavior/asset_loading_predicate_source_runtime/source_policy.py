"""Emit the partial actual player loading-screen controller support type."""
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
MODULE = 'FortniteGame'


def sources():
    evidence = json.loads((HERE / 'native-evidence.json').read_text())
    base = int(evidence['image_base'], 16)
    with Path(evidence['image']).open('rb') as stream:
        for window in evidence['windows']:
            begin = int(window['begin'], 16)
            stream.seek(begin - base)
            data = stream.read(int(window['end_exclusive'], 16) - begin)
            if hashlib.sha256(data).hexdigest() != window['sha256']:
                raise ValueError('loading predicate original capture changed: ' + hex(begin))
    return {'FortPlayerLoadingScreenState.h': (HERE / 'FortPlayerLoadingScreenState.h').read_text()}
