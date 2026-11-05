#!/usr/bin/env python3
"""Export a validated original landscape uniform inventory for the native loader."""
import argparse
from collections import Counter
import json
from pathlib import Path

import export_original_uniform_contracts as fixed
import landscape_layout_contract as landscape

ROOT = fixed.ROOT
CONTRACT = ROOT / 'behavior/maps_terrain_material_runtime/landscape-uniform-contract.json'
OUTPUT = ROOT / 'generated/material_pipeline/apollo-d4-landscape-spirv/original-uniform-contracts.json'
NAMES = ('LandscapeParameters', 'LandscapeContinuousLODParameters')
EXPECTED = {'View': 37, 'LandscapeParameters': 33, 'Primitive': 30,
            'LandscapeContinuousLODParameters': 24, 'Material': 9,
            'ShadowDepthPass': 6, 'PrecomputedLightingBuffer': 2, 'LightmapDensityPass': 2}


def build():
    recovered = json.loads(CONTRACT.read_text())
    report = json.loads(landscape.SHADER_REPORT.read_text())
    if (recovered['schema'] != 'landscape-original-uniform-contract-v1'
            or recovered['runtime_verified'] is not False
            or recovered['image'] != {'path': str(fixed.IMAGE), 'sha256': fixed.IMAGE_HASH}
            or recovered['shader_report'] != fixed.receipt(landscape.SHADER_REPORT)
            or fixed.receipt(fixed.IMAGE)['sha256'] != fixed.IMAGE_HASH):
        raise ValueError('landscape evidence identity mismatch')
    rows = report['records']
    if len(rows) != 43 or len({r['shader_index'] for r in rows}) != 43:
        raise ValueError('incomplete landscape shader inventory')
    recorded = {c['parameter_name']: c for c in recovered['contracts']}
    if len(recorded) != len(recovered['contracts']) or not set(NAMES) <= set(recorded):
        raise ValueError('missing or duplicate recovered landscape contracts')
    contracts = []
    with fixed.IMAGE.open('rb') as image:
        source_rows = {row[0]: row for row in landscape.FIXED}
        for name in NAMES:
            contract = landscape.decode(source_rows[name], image)
            contract['original_shader_associations'] = landscape.verify_wrappers(contract, report, True)
            if contract != recorded[name]:
                raise ValueError('recovered landscape contract differs from original captures: ' + name)
            del contract['original_shader_associations']
            contracts.append(contract)
    by_name = {c['parameter_name']: c for c in contracts}
    bindings, counts = [], Counter()
    for row in rows:
        arrays = [d['uniform_buffer_names'] for d in row['optional_data'] if 'uniform_buffer_names' in d]
        if len(arrays) != 1:
            raise ValueError('missing or ambiguous wrapper uniform names')
        seen = set()
        for slot, name in enumerate(arrays[0]):
            if not name:
                continue
            if name in seen or slot >= 32 or name not in EXPECTED:
                raise ValueError('duplicate, out-of-range or unknown uniform role')
            seen.add(name)
            counts[name] += 1
            active = bool(row['resource_table_bits'] & (1 << slot))
            hashes = row['resource_maps']['layout_hash']
            actual = hashes[slot] if slot < len(hashes) else 0
            if name in by_name:
                expected = by_name[name]['original_resource_layout_hash'] if active else 0
                if actual != expected:
                    raise ValueError('wrapper layout hash mismatch: ' + name)
                status = 'original_fixed_layout_decoded'
            else:
                status = 'original_layout_unresolved'
            bindings.append(dict(shader_index=row['shader_index'], source_register=slot,
                                 parameter_name=name, resource_table_active=active,
                                 recorded_resource_layout_hash=actual, status=status))
    if counts != EXPECTED:
        raise ValueError('landscape uniform association inventory mismatch')
    return dict(schema='landscape-original-uniform-contracts-v1', image=recovered['image'],
                inventory=fixed.receipt(landscape.SHADER_REPORT), contracts=contracts,
                bindings=bindings, runtime_verified=False,
                limits='Original ABI evidence only. Static slots, other original uniform layouts, runtime values, RHI binding and rendering remain unverified.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', type=Path, default=OUTPUT)
    args = parser.parse_args()
    output = args.out.resolve()
    if (ROOT / 'generated').resolve() not in output.parents:
        raise ValueError('output must stay under porting/generated')
    result = build()
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(dict(contracts=len(result['contracts']), bindings=len(result['bindings']),
                          unresolved=sum(b['status'] == 'original_layout_unresolved' for b in result['bindings']))))


if __name__ == '__main__':
    main()
