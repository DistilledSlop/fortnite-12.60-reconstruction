#!/usr/bin/env python3
"""Audit the eleven multi-material Apollo proxy tiles against local extracted data.

Requires ignored converted GLBs, weightmap layers.json files and baked PNGs.
Output contains only tile IDs and derived structural facts, not asset payloads.
"""
import json
import struct
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[2] / 'generated/converted'
LANDSCAPE = ROOT / 'FortniteGame/Content/Athena/Apollo/Maps/Landscape'
TILES = 'B5 D7 D8 E3 E4 E5 F2 F5 G2 G3 G4'.split()
MATERIALS = {'M_CommonLandscape', 'M_Biome_Swamp', 'M_Biome_Mountains', 'M_Biome_Farm'}


def inspect(tile):
    directory = LANDSCAPE / f'Apollo_Terrain_LS_{tile}'
    components = json.loads((directory / 'weightmaps/layers.json').read_text())
    data = (directory / f'LandscapeStreamingProxy_{tile}.glb').read_bytes()
    if data[:4] != b'glTF':
        raise ValueError(f'{tile}: invalid GLB')
    json_length, json_type = struct.unpack_from('<II', data, 12)
    if json_type != 0x4e4f534a:
        raise ValueError(f'{tile}: missing JSON chunk')
    gltf = json.loads(data[20:20 + json_length])
    offset = 20 + json_length
    binary_length, binary_type = struct.unpack_from('<II', data, offset)
    if binary_type != 0x004e4942:
        raise ValueError(f'{tile}: missing binary chunk')
    binary = memoryview(data)[offset + 8:offset + 8 + binary_length]
    primitives = [p for mesh in gltf['meshes'] for p in mesh['primitives']]
    names = [m['name'] for m in gltf['materials']]
    if len(primitives) != 4 or len(set(names)) != 2 or not set(names) <= MATERIALS:
        raise ValueError(f'{tile}: unexpected primitive or material set')
    sections = []
    for p in primitives:
        accessor = gltf['accessors'][p['attributes']['TEXCOORD_1']]
        if accessor['componentType'] != 5126 or accessor['type'] != 'VEC2' or 'sparse' in accessor:
            raise ValueError(f'{tile}: unsupported UV accessor')
        view = gltf['bufferViews'][accessor['bufferView']]
        start = view.get('byteOffset', 0) + accessor.get('byteOffset', 0)
        stride = view.get('byteStride', 8)
        uv = [struct.unpack_from('<ff', binary, start + i * stride) for i in range(accessor['count'])]
        bounds = [[round(min(v[axis] for v in uv), 6), round(max(v[axis] for v in uv), 6)] for axis in (0, 1)]
        sections.append({'material': names[p['material']], 'uv1_bounds': bounds})
    quadrants = set()
    for section in sections:
        axes = []
        for low, high in section['uv1_bounds']:
            if abs(low) < 0.001 and abs(high - 0.498) < 0.002:
                axes.append(0)
            elif abs(low - 0.498) < 0.002 and abs(high - 0.996) < 0.002:
                axes.append(1)
            else:
                raise ValueError(f'{tile}: unexpected UV quadrant')
        quadrants.add(tuple(axes))
    if quadrants != {(0, 0), (0, 1), (1, 0), (1, 1)}:
        raise ValueError(f'{tile}: missing or duplicate UV quadrants')
    if len(components) != 4:
        raise ValueError(f'{tile}: unexpected component count')
    bases = [c['sectionBase'] for c in components]
    min_x, min_y = min(b[0] for b in bases), min(b[1] for b in bases)
    offsets = [[(b[0] - min_x) // 127, (b[1] - min_y) // 127] for b in bases]
    if sorted(offsets) != [[0, 0], [0, 1], [1, 0], [1, 1]] or any(
        (b[0] - min_x) % 127 or (b[1] - min_y) % 127 for b in bases
    ):
        raise ValueError(f'{tile}: unexpected component layout')
    with Image.open(ROOT / 'apollo-baked' / f'Terrain_{tile}.png') as atlas:
        size = list(atlas.size)
    if size != [1024, 1024]:
        raise ValueError(f'{tile}: unexpected atlas dimensions')
    return {'tile': tile, 'component_offsets': offsets, 'atlas_size': size, 'sections': sections}


def main():
    result = {
        'method': 'Read converted GLB primitive materials and UV1 bounds, exported weightmap component bases, and baked PNG dimensions.',
        'limitations': [
            'GLB and atlas facts do not verify the imported UE mesh section-to-slot mapping or rendered appearance.',
            'The atlas uses simplified layer blends and diffuse textures; original landscape material graphs and roughness are not reconstructed.',
            'Matching component and UV quadrant layouts does not by itself prove texture orientation or pixel-level material parity.',
        ],
        'tiles': [inspect(tile) for tile in TILES],
    }
    # One tile per line keeps the evidence reviewable without expanding every bound.
    print('{')
    print('  "method": ' + json.dumps(result['method']) + ',')
    print('  "limitations": ' + json.dumps(result['limitations']) + ',')
    print('  "tiles": [')
    for index, tile in enumerate(result['tiles']):
        print('    ' + json.dumps(tile, separators=(',', ':')) + (',' if index + 1 < len(result['tiles']) else ''))
    print('  ]\n}')


if __name__ == '__main__':
    main()
