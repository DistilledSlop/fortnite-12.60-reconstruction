#!/usr/bin/env python3
"""Export independently decoded fixed original uniforms for the Paper2D inventory.

This is original ABI input for the provider/repacker, not a current-buffer
compatibility assertion. Material is expression-dependent and remains explicit.
"""
import argparse
import hashlib
import importlib
import json
from pathlib import Path
import re

from translucent_depth_layout_contract import flatten
from view_layout_contract import legacy_resource_hash, original

ROOT = Path(__file__).resolve().parents[1]
CAP = ROOT / 'generated/material_pipeline'
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
IMAGE_HASH = '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
FIXED = {
    'View': ('FViewUniformShaderParameters', 'view-identity', '140555220', None),
    'LocalVF': ('FLocalVertexFactoryUniformShaderParameters', 'localvf-identity', '140546830', None),
    'TranslucentDepthPass': ('FTranslucencyDepthPassUniformParameters', 'translucent-depth-identity', '1404dc160', 'translucent_depth_layout_contract'),
    'ShadowDepthPass': ('FShadowDepthPassUniformParameters', 'shadow-depth-identity', '1404d4ac0', 'shadow_depth_layout_contract'),
    'LightmapDensityPass': ('FLightmapDensityPassUniformParameters', 'lightmap-density-identity', '1404abf10', 'lightmap_density_layout_contract'),
    'TranslucentBasePass': ('FTranslucentBasePassUniformParameters', 'translucent-base-identity', '1404912e0', 'translucent_base_layout_contract'),
}


def receipt(path):
    with path.open('rb') as stream:
        digest = hashlib.file_digest(stream, 'sha256').hexdigest()
    return dict(path=str(path), sha256=digest)


def literal(image, address):
    image.seek(address - 0x140000000)
    data = image.read(512)
    for end in range(0, len(data), 2):
        if data[end:end+2] == b'\0\0':
            return data[:end].decode('utf-16le')
    raise ValueError('unterminated identity string')


def decode(name, image):
    typ, directory, initializer, module = FIXED[name]
    asm = CAP / directory / (initializer + '.asm')
    text = asm.read_text()
    types = re.findall(r'LEA R9,\[(0x[0-9a-f]+)\]', text)
    if len(types) != 1 or literal(image, int(types[0], 16)) != typ:
        raise ValueError('original struct type identity mismatch: ' + name)
    names = re.findall(r'LEA RAX,\[(0x[0-9a-f]+)\]\n\w+ MOV dword ptr \[RSP \+ 0x28\],0x[0-9a-f]+\n\w+ MOV qword ptr \[RSP \+ 0x20\],RAX', text)
    if len(names) != 1 or literal(image, int(names[0], 16)) != name:
        raise ValueError('original uniform variable identity mismatch: ' + name)
    if module:
        layout, inputs = importlib.import_module(module).read_original()
    else:
        chain = CAP / directory / 'member-chain.json'
        layout = original(json.loads(chain.read_text()), text)
        inputs = [chain, asm]
    flat = flatten(layout)
    resources = [dict(offset=m['offset'], base_type=m['base_type'], path=m['name'],
                      resource_index=m['resource_index'])
                 for m in flat['members'] if m['base_type'] in (5, 6, 7, 8)]
    return dict(parameter_name=name, struct_type=typ, static_slot_name='',
                original=layout, original_resource_layout_hash=legacy_resource_hash(flat),
                resources=resources, provenance=[receipt(p) for p in inputs],
                identity_literal_address=types[0], uniform_literal_address=names[0])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', type=Path, default=ROOT/'generated/paper2d-shader-semantics/original-uniform-contracts.json')
    args = parser.parse_args()
    report = ROOT/'generated/paper2d-shader-semantics/conversion/d3d.json'
    rows = json.loads(report.read_text())['records']
    if len(rows) != 33 or len({r['shader_index'] for r in rows}) != 33:
        raise ValueError('expected complete 33-shader original inventory')
    if receipt(IMAGE)['sha256'] != IMAGE_HASH:
        raise ValueError('original identity image mismatch')
    with IMAGE.open('rb') as image:
        contracts = {name: decode(name, image) for name in FIXED}
    bindings, counts = [], {name: 0 for name in FIXED}
    for row in rows:
        arrays = [d['uniform_buffer_names'] for d in row['optional_data'] if 'uniform_buffer_names' in d]
        if len(arrays) != 1:
            raise ValueError('missing original uniform names')
        seen = set()
        for slot, name in enumerate(arrays[0]):
            if not name:
                continue
            if name in seen or slot >= 32:
                raise ValueError('duplicate or out-of-range original uniform')
            seen.add(name)
            hashes = row['resource_maps']['layout_hash']
            actual = hashes[slot] if slot < len(hashes) else 0
            active = bool(row['resource_table_bits'] & (1 << slot))
            binding = dict(shader_index=row['shader_index'], source_register=slot,
                           parameter_name=name, resource_table_active=active,
                           recorded_resource_layout_hash=actual)
            if name == 'Material':
                binding['status'] = 'requires_original_material_expression_contract'
            elif name in contracts:
                contract = contracts[name]
                expected = contract['original_resource_layout_hash'] if active else 0
                if actual != expected:
                    raise ValueError('original layout hash mismatch: ' + name)
                binding['status'] = 'original_fixed_layout_decoded'
                counts[name] += 1
            else:
                raise ValueError('unclosed original uniform: ' + name)
            bindings.append(binding)
    if counts != dict(View=30, LocalVF=15, TranslucentDepthPass=3,
                      ShadowDepthPass=4, LightmapDensityPass=1, TranslucentBasePass=2):
        raise ValueError('incomplete original uniform associations')
    result = dict(schema='paper2d-original-uniform-contracts-v1',
                  image=dict(path=str(IMAGE), sha256=IMAGE_HASH), inventory=receipt(report),
                  contracts=list(contracts.values()), bindings=bindings, runtime_verified=False,
                  limits='Original ABI only. Current value semantics, original-layout RHI binding, Material expressions and rendering remain unverified.')
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(dict(fixed_contracts=len(contracts), fixed_uses=sum(counts.values()),
                          material_uses=sum(b['parameter_name'] == 'Material' for b in bindings))))


if __name__ == '__main__':
    main()
