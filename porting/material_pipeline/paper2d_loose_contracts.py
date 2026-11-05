#!/usr/bin/env python3
"""Recover Paper2D loose-parameter names from pinned original code and records.

The pinned instruction windows are independently captured by view_original_probe.py;
the mappings below transcribe their constructor and serializer field accesses.
This proves serialized identities, not runtime binding values or rendered output.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[2]
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
IMAGE_SHA = '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
MANIFEST = ROOT / 'porting/generated/paper2d-shader-semantics/shader-manifest.json'
OUTPUT = ROOT / 'porting/behavior/paper2d_loose_contracts_runtime/evidence.json'
BASE = 0x140000000
WINDOWS = {
    0x1432f19f0: (0x185, '5c036e5917d2f7b7d449ba75f3d7acb986a798ca16d47741b5dfb022338567e5'),
    0x1434d4d00: (0xeb, '07b2cb230b723c02fdde8e13237549d574a48fb6f4213af63ea6fa3dd20501f2'),
    0x143558d50: (0xd4, 'b1057adba69fd8ba3eae768188322106625c7e7e1da8854fdb11b8f2a1c31806'),
    0x143237540: (0x3f, '3ed293e550205b6a79adb231f3570b691a70a72ac40a7634d354bbf65e0f7b9d'),
    0x1432e77c0: (0x95, '5d60eb708fa2970863211d94c57314b6bae7727e6aba9bb44121ed736ae88898'),
    0x143a7aee0: (0x125, '0b2800cb18f7437ff4b60aa171f2410d4479b08be3ea6ead48cf6d7fda12d4e2'),
    0x143a81450: (0x3fc, '8dfd52ceeed156d267be4372f935b7b2e9f6f8806360b7b115385464fe0ac163'),
}
LITERALS = {'BuiltLightingAndSelectedFlags': 0x1466b6218, 'LightMapResolutionScale': 0x1466b6258,
            'LightMapDensityDisplayOptions': 0x1466b6288, 'HitProxyId': 0x1466fba70,
            'TranslucentShadowStartOffset': 0x146710ac0}
EXPECTED = {
    2470: ('TLightMapDensityPSFNoLightMapPolicy', 'lightmap_density_parameters',
           [('BuiltLightingAndSelectedFlags', 32, 12), ('LightMapResolutionScale', 20, 8), ('LightMapDensityDisplayOptions', 48, 16)]),
    2474: ('FHitProxyPS', 'hit_proxy_scalar_parameter', [('HitProxyId', 32, 16)]),
    712: ('FHitProxyPS', 'hit_proxy_scalar_parameter', [('HitProxyId', 32, 16)]),
    3376: ('TTranslucencyShadowDepthPS<TranslucencyShadowDepth_Standard>', 'shadow_depth_parameter', [('TranslucentShadowStartOffset', 20, 4)]),
    3377: ('TTranslucencyShadowDepthPS<TranslucencyShadowDepth_PerspectiveCorrect>', 'shadow_depth_parameter', [('TranslucentShadowStartOffset', 20, 4)]),
}


def walk(value, path=()):
    if isinstance(value, dict):
        if 'type' in value and 'output_hash' in value:
            yield path, value
        for k, v in value.items():
            yield from walk(v, path + (k,))
    elif isinstance(value, list):
        for i, v in enumerate(value):
            yield from walk(v, path + (i,))


def digest_file(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for block in iter(lambda: f.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--image', type=Path, default=IMAGE)
    ap.add_argument('--manifest', type=Path, default=MANIFEST)
    ap.add_argument('--out', type=Path, default=OUTPUT)
    a = ap.parse_args()
    if digest_file(a.image) != IMAGE_SHA:
        raise ValueError('original image SHA256 mismatch')
    with a.image.open('rb') as image:
        def read(va, size):
            image.seek(va - BASE)
            return image.read(size)
        for va, (size, expected) in WINDOWS.items():
            if hashlib.sha256(read(va, size)).hexdigest() != expected:
                raise ValueError('instruction window mismatch at %#x' % va)
        for name, va in LITERALS.items():
            expected = (name + '\0').encode('utf-16le')
            if read(va, len(expected)) != expected:
                raise ValueError('literal mismatch at %#x' % va)
        for vt, serializer in [(0x1466b2288, 0x1432e77c0), (0x146694000, 0x143237540)]:
            if struct.unpack('<Q', read(vt + 0x10, 8))[0] != serializer:
                raise ValueError('vtable serializer mismatch')
    manifest = json.loads(a.manifest.read_text())
    hashes = {s['archive_index']: s['archive_output_hash'] for s in manifest['shaders']}
    occurrences = []
    for owner in manifest['owners']:
        for path, rec in walk(owner['decoded_resource']):
            for index, (typename, key, fields) in EXPECTED.items():
                if rec.get('output_hash', {}).get('hash') != hashes[index] or rec['type']['name'] != typename:
                    continue
                raw = rec[key] if isinstance(rec[key], list) else [rec[key]]
                expected = [dict(base_index=offset, byte_count=size, buffer_index=0) for _, offset, size in fields]
                if raw != expected:
                    raise ValueError('unexpected serialized allocation for %s' % index)
                occurrences.append(dict(shader_index=index, owner=owner['owner_path'], record_path=list(path),
                                        record_offset=rec['offset'], output_hash=hashes[index],
                                        parameters=[dict(name=name, **item) for (name, _, _), item in zip(fields, raw)]))
    if sorted(o['shader_index'] for o in occurrences) != sorted(EXPECTED):
        raise ValueError('record occurrence set changed; inspect before regenerating')
    result = {'schema': 'paper2d-original-loose-parameter-identities-v1',
              'image': {'path': str(a.image), 'sha256': IMAGE_SHA, 'address_layout': 'VA = 0x140000000 + file offset'},
              'manifest_sha256': digest_file(a.manifest),
              'instruction_windows': [{'address': hex(va), 'size': size, 'sha256': sha} for va, (size, sha) in WINDOWS.items()],
              'literal_addresses': {n: hex(va) for n, va in LITERALS.items()},
              'original_code_evidence': [
                  'Constructor 0x1432f19f0 binds BuiltLightingAndSelectedFlags, LightMapResolutionScale and LightMapDensityDisplayOptions into object offsets 0xec, 0xf2 and 0xf8 respectively via 0x143a81450. Its vtable 0x1466b2288 slot +0x10 points to serializer 0x1432e77c0, which serializes those same fields in that order after three uniform-buffer fields.',
                  'Constructor 0x1434d4d00 binds HitProxyId at object offset 0xe0; constructor 0x143558d50 binds TranslucentShadowStartOffset at 0xe0. Both install vtable 0x146694000, whose slot +0x10 points to serializer 0x143237540. It serializes field 0xe0 after the mesh-material base.',
                  'The binding helper 0x143a81450 passes object offsets +0, +2 and +4 to parameter-map lookup; serializer 0x143a7aee0 reads three 16-bit values into +2, +4 and +0 in that order. The decoder consequently reads base_index, byte_count, buffer_index in serialized order.',
              ], 'occurrences': occurrences,
              'limitations': ['Identity and serialized allocations only; values, current binding compatibility, shader execution and rendering are not established.', 'Record decoder and association with shader archive hashes are existing generated inputs; this report preserves each original record occurrence.'],
              'reproduce': 'python3 porting/material_pipeline/paper2d_loose_contracts.py'}
    a.out.parent.mkdir(parents=True, exist_ok=True)
    a.out.write_text(json.dumps(result, indent=2) + '\n')
    print('wrote', a.out, 'with', len(occurrences), 'original record occurrences')


if __name__ == '__main__':
    main()
