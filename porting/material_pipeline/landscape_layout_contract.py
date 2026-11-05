#!/usr/bin/env python3
"""Recover landscape uniform metadata solely from original-image captures."""
import argparse
import json
from pathlib import Path
import re

from export_original_uniform_contracts import IMAGE, IMAGE_HASH, literal, receipt
from translucent_depth_layout_contract import flatten
from view_layout_contract import legacy_resource_hash, original

ROOT = Path(__file__).resolve().parents[1]
CAP = ROOT / 'generated/apollo-material-evidence'
SHADER_REPORT = ROOT / 'generated/material_pipeline/apollo-d4-landscape-spirv/d3d-report.json'
FIXED = (
    ('LandscapeParameters', 'FLandscapeUniformShaderParameters', '140506010', 'landscape', '143af7230'),
    ('LandscapeContinuousLODParameters', 'FLandscapeSectionLODUniformParameters', '140505f00', 'continuous', '143af6650'),
)


def decode(row, image):
    name, typ, address, label, head = row
    asm = CAP / 'terrain-uniform-initializers' / ('0x' + address + '.asm')
    chain_path = CAP / ('terrain-uniform-chain-' + label) / 'member-chain.json'
    # The bounded initializer capture uses two spaces between address/instruction.
    text = '\n'.join(' '.join(line.split(None, 1)) for line in asm.read_text().splitlines())
    chains = json.loads(chain_path.read_text())
    if not chains or chains[0]['function'].lower().removeprefix('0x') != head:
        raise ValueError('initializer helper head mismatch: ' + name)
    heads = re.findall(r'LEA RAX,\[(0x[0-9a-f]+)\]\n\w+ NOP\n\w+ LEA RDX,\[RSP \+ 0x40\]', text)
    types = re.findall(r'LEA R9,\[(0x[0-9a-f]+)\]', text)
    names = re.findall(r'LEA RAX,\[(0x[0-9a-f]+)\]\n\w+ MOV dword ptr \[RSP \+ 0x28\],0x[0-9a-f]+\n\w+ MOV qword ptr \[RSP \+ 0x20\],RAX', text)
    if len(heads) != 1 or int(heads[0], 16) != int(head, 16):
        raise ValueError('initializer chain identity mismatch: ' + name)
    if len(types) != 1 or literal(image, int(types[0], 16)) != typ:
        raise ValueError('original structure identity mismatch: ' + name)
    if len(names) != 1 or literal(image, int(names[0], 16)) != name:
        raise ValueError('original parameter identity mismatch: ' + name)
    layout = original(chains, text)
    flat = flatten(layout)
    resources = [dict(offset=m['offset'], base_type=m['base_type'], path=m['name'],
                      resource_index=m['resource_index'])
                 for m in flat['members'] if m['base_type'] in (5, 6, 7, 8)]
    return dict(parameter_name=name, struct_type=typ, original=layout,
                original_resource_layout_hash=legacy_resource_hash(flat), resources=resources,
                identity_literal_address=types[0], uniform_literal_address=names[0],
                provenance=[receipt(asm), receipt(chain_path)])


def verify_wrappers(contract, report):
    name = contract['parameter_name']
    expected = contract['original_resource_layout_hash']
    associations = []
    for record in report['records']:
        arrays = [item['uniform_buffer_names'] for item in record['optional_data']
                  if 'uniform_buffer_names' in item]
        if len(arrays) != 1 or arrays[0].count(name) > 1:
            raise ValueError('missing or ambiguous wrapper uniform names: ' + name)
        if name not in arrays[0]:
            continue
        index = arrays[0].index(name)
        hashes = record['resource_maps']['layout_hash']
        active = bool(record['resource_table_bits'] & (1 << index))
        recorded = hashes[index] if index < len(hashes) else 0
        if (active and recorded != expected) or (not active and recorded != 0):
            raise ValueError('original shader wrapper layout hash mismatch: ' + name)
        associations.append(dict(shader_index=record['shader_index'], uniform_buffer=index,
                                 resource_table_active=active, original_layout_hash=recorded))
    if not any(row['resource_table_active'] for row in associations):
        raise ValueError('no active original shader wrapper associations: ' + name)
    return associations


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', type=Path, default=ROOT / 'behavior/maps_terrain_material_runtime/landscape-uniform-contract.json')
    args = parser.parse_args()
    if receipt(IMAGE)['sha256'] != IMAGE_HASH:
        raise ValueError('original identity image mismatch')
    with IMAGE.open('rb') as image:
        contracts = [decode(row, image) for row in FIXED]
    report = json.loads(SHADER_REPORT.read_text())
    for contract in contracts:
        contract['original_shader_associations'] = verify_wrappers(contract, report)
    result = dict(schema='landscape-original-uniform-contract-v1',
                  image=dict(path=str(IMAGE), sha256=IMAGE_HASH), contracts=contracts,
                  shader_report=receipt(SHADER_REPORT),
                  runtime_verified=False,
                  limits='Original metadata ABI only; runtime value semantics, binding and rendering remain unverified. Resource hashes are corroborated by the captured shader wrappers.')
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({c['parameter_name']: dict(size=c['original']['struct_size'], members=len(c['original']['members']), resources=len(c['resources']), hash=c['original_resource_layout_hash']) for c in contracts}))


if __name__ == '__main__':
    main()
