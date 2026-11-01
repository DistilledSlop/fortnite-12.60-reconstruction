#!/usr/bin/env python3
"""Replay pinned original CommonButton state-dependency windows and bindings."""
import argparse
import hashlib
import json
from pathlib import Path
import struct

BASE = 0x140000000
IMAGE_SHA = '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
PROOF_SHA = 'ef1267eb08186127fc3e868d88647740fb881e605df753f8ad831c00c82b36b9'


def inspect(image_path, proof_path):
    raw = Path(proof_path).read_bytes()
    if hashlib.sha256(raw).hexdigest() != PROOF_SHA:
        raise ValueError('proof hash')
    proof = json.loads(raw)
    image = Path(image_path).read_bytes()
    if hashlib.sha256(image).hexdigest() != IMAGE_SHA:
        raise ValueError('late image hash')
    for window in proof['windows']:
        offset = int(window['va'], 16) - BASE
        value = image[offset:offset + window['size']]
        if (value.hex() != window['bytes'] or
                hashlib.sha256(value).hexdigest() != window['sha256']):
            raise ValueError('original window ' + window['va'])
    vtable = int(proof['vtable'], 16)
    if struct.unpack_from('<Q', image, vtable - BASE + 0x268)[0] != 0x1407f78f0:
        raise ValueError('SetIsEnabled override binding')
    return {'passed': True, 'windows_verified': len(proof['windows']),
            'image_sha256': IMAGE_SHA, 'proof_sha256': PROOF_SHA,
            'class': proof['class'], 'constructor': proof['constructor'],
            'set_enabled_override': proof['set_enabled_override'],
            'default_state_bits': proof['default_state_bits'],
            'scope': 'Byte/binding replay of manually reviewed source dependencies; full button, Slate, visual and callback implementation remains unfinished.',
            'runtime_claim': False}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('image', 'proof', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    report = inspect(args.image, args.proof)
    with args.output.open('x') as out:
        json.dump(report, out, indent=2)
        out.write('\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()
