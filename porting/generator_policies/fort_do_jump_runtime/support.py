"""Complete base jump bodies; the Athena jump/saved-move overrides remain open."""
import hashlib
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent


def sources():
    evidence = json.loads((HERE / 'native-evidence.json').read_text())
    with Path(evidence['image']).open('rb') as stream:
        for window in evidence['source_windows']:
            stream.seek(int(window['address'], 0) - 0x140000000)
            if hashlib.sha256(stream.read(window['length'])).hexdigest() != window['sha256']:
                raise ValueError('base jump evidence changed: ' + window['address'])
    return {'FortJumpMathNative.h': (HERE / 'FortJumpMathNative.h').read_text()}


def validate(ctx, snippets_by_class):
    if ctx['class_pkg'].get('UFortMovementComp_Character') != '/Script/FortniteGame':
        raise ValueError('base jump owner changed')
    fields = [e for e in snippets_by_class.get('UFortMovementComp_Character', [])
              if e.get('function') == 'NativeDoJumpFlag1dfd']
    if (len(fields) != 1 or fields[0].get('kind') != 'member_field'
            or fields[0].get('signature') != 'uint8 NativeDoJumpFlag1dfd'
            or int(str(fields[0].get('offset')), 0) != 0x1dfd
            or fields[0].get('default') is not None):
        raise ValueError('base jump shared native flag changed')
