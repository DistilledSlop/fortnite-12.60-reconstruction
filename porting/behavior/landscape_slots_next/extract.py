#!/usr/bin/env python3
"""Extract named D3D resource registers from captured Apollo shader wrappers."""
import hashlib
import json
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
REPORT = ROOT / 'generated/material_pipeline/apollo-d4-landscape-spirv/d3d-report.json'
CONTRACTS = ROOT / 'generated/material_pipeline/apollo-d4-landscape-spirv/original-uniform-contracts.json'
OUT = Path(__file__).with_name('resource-registers.json')
NAMES = ('LandscapeParameters', 'LandscapeContinuousLODParameters')
TYPES = {'texture': 5, 'shader_resource_view': 6, 'unordered_access_view': 7, 'sampler': 8}

def receipt(path):
    return {'path': str(path.relative_to(ROOT.parent)), 'sha256': hashlib.sha256(path.read_bytes()).hexdigest()}

def main():
    report = json.loads(REPORT.read_text())
    inventory = json.loads(CONTRACTS.read_text())
    assert inventory['inventory']['sha256'] == receipt(REPORT)['sha256']
    contracts = {c['parameter_name']: c for c in inventory['contracts'] if c['parameter_name'] in NAMES}
    assert set(contracts) == set(NAMES)
    counts = Counter()
    rows = []
    for shader in report['records']:
        arrays = [x['uniform_buffer_names'] for x in shader['optional_data'] if 'uniform_buffer_names' in x]
        assert len(arrays) == 1
        names = arrays[0]
        for name in NAMES:
            if name not in names:
                continue
            assert names.count(name) == 1
            slot = names.index(name)
            contract = contracts[name]
            assert shader['resource_table_bits'] & (1 << slot)
            assert shader['resource_maps']['layout_hash'][slot] == contract['original_resource_layout_hash']
            indexed = {r['resource_index']: r for r in contract['resources']}
            assert len(indexed) == len(contract['resources'])
            mapped = []
            seen = set()
            for kind, base_type in TYPES.items():
                for binding in shader['resource_bindings'][kind]:
                    if binding['uniform_buffer'] != slot:
                        continue
                    resource = indexed[binding['resource']]
                    assert resource['base_type'] == base_type
                    assert (kind, binding['bind']) not in seen
                    seen.add((kind, binding['bind']))
                    mapped.append({'resource': resource['path'], 'resource_index': binding['resource'],
                                   'kind': kind, 'd3d_register': binding['bind']})
            assert mapped
            counts[name] += 1
            rows.append({'shader_index': shader['shader_index'], 'frequency': shader['frequency'],
                         'wrapper_sha256': shader['wrapper_sha256'], 'parameter_name': name,
                         'd3d_constant_buffer_register': slot, 'resources': mapped})
    assert counts == {'LandscapeParameters': 33, 'LandscapeContinuousLODParameters': 24}
    result = {'schema': 'apollo-landscape-d3d-resource-registers-v1',
              'inputs': [receipt(REPORT), receipt(CONTRACTS)],
              'counts': dict(counts), 'bindings': rows,
              'limits': 'Captured wrapper D3D registers and named original resource indices only. Constant-buffer registers are per shader, not metadata static slots. This does not establish static-slot semantics, current metadata or providers, SPIR-V/Vulkan descriptor translation, runtime binding, or rendered output. Unlisted resources are not proven bound by a wrapper.'}
    OUT.write_text(json.dumps(result, indent=2) + '\n')
    print('wrote', len(rows), 'uniform associations and', sum(len(r['resources']) for r in rows), 'resource bindings')

if __name__ == '__main__':
    main()
