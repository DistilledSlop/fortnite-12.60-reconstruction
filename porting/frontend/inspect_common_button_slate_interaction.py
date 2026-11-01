#!/usr/bin/env python3
"""Replay original Slate construction/getter evidence; no runtime claim."""
import argparse
import hashlib
import json
import struct
from pathlib import Path


def inspect(image, manifest):
    proof = json.loads(manifest.read_text())
    digest = hashlib.sha256()
    with image.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(chunk)
    if image.stat().st_size != proof['image_size'] or digest.hexdigest() != proof['image_sha256']:
        raise ValueError('original image size/hash mismatch')
    windows = {}
    with image.open('rb') as stream:
        for row in proof['windows']:
            stream.seek(int(row['va'], 16) - int(proof['image_base'], 16))
            data = stream.read(row['length'])
            if data.hex() != row['bytes_hex'] or hashlib.sha256(data).hexdigest() != row['sha256']:
                raise ValueError('original window mismatch: ' + row['name'])
            windows[row['name']] = data
    if struct.unpack('<Q', windows['internal-vtable-rebuild-slot'])[0] != 0x1407e9150:
        raise ValueError('internal rebuild slot')
    if struct.unpack('<Q', windows['slate-vtable-target'])[0] != 0x1407cf560:
        raise ValueError('custom Slate predicate slot')
    if b'SCommonButton\0' not in windows['slate-debug-source-and-type']:
        raise ValueError('actual Slate construction tag')
    allocation = windows['slate-shared-allocation']
    call = 0x14078f041 - 0x14078eff0
    if allocation[call] != 0xe8 or 0x14078f046 + struct.unpack_from('<i', allocation, call + 1)[0] != 0x1430824d0:
        raise ValueError('actual custom Slate base constructor call')
    constructor = windows['base-slate-constructor']
    if constructor[14:17] != bytes.fromhex('488d05') or constructor[25:28] != bytes.fromhex('488903'):
        raise ValueError('base object vtable installation')
    base_table = 0x1430824e5 + struct.unpack_from('<i', constructor, 17)[0]
    if base_table != 0x14665bcf8 or struct.unpack('<Q', windows['base-slate-predicate-slot'])[0] != 0x1407cf580:
        raise ValueError('actual base Slate predicate slot')
    if windows['base-slate-predicate-body'].hex() != '0fb641282401c3':
        raise ValueError('base predicate must read byte28 bit0')
    if windows['slate-interaction-getter'].hex() != '80b9d90500000074070fb641282401c332c0c3':
        raise ValueError('custom predicate must add independent interaction gate')
    if windows['base-widget-hover-enter'][-4:] != bytes.fromhex('80492801'):
        raise ValueError('mouse enter must set the queried hover bit')
    if windows['base-widget-hover-leave'][-4:] != bytes.fromhex('806128fe'):
        raise ValueError('mouse leave must clear the queried hover bit')
    enter = windows['base-button-enter-call']
    if enter[9] != 0xe8 or 0x1430b2c6a + struct.unpack_from('<i', enter, 10)[0] != 0x142f71f20:
        raise ValueError('base button enter must call widget hover entry')
    leave = windows['base-button-leave-query-call']
    if leave[9:15] != bytes.fromhex('ff90c8010000') or leave[24] != 0xe8 or 0x1430b2e2c + struct.unpack_from('<i', leave, 25)[0] != 0x142f71f90:
        raise ValueError('base button leave must query slot1c8 then clear widget hover')
    return {'passed': True, 'image_sha256': proof['image_sha256'],
            'windows': len(windows), 'window_bytes': sum(map(len, windows.values())),
            'slate_type': 'SCommonButton', 'custom_predicate': '0x1407cf560',
            'base_predicate': '0x1407cf580',
            'relation': 'custom interaction-enabled gate AND base hover state',
            'typed_adapter': 'bInteractionEnabled && SButton::IsHovered()',
            'runtime_verified': False}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest', type=Path, default=Path(__file__).with_name('common_button_slate_interaction_windows.json'))
    parser.add_argument('--image', type=Path)
    args = parser.parse_args()
    image = args.image or Path(json.loads(args.manifest.read_text())['image'])
    print(json.dumps(inspect(image, args.manifest), sort_keys=True))


if __name__ == '__main__':
    main()
