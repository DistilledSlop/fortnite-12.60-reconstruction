#!/usr/bin/env python3
"""Resolve original Paper2D resource descriptors to full uniform member paths.

Current indices are source-derived candidates, not runtime binding receipts.
Original indices follow recovered traversal order; current Engine indices follow
ascending byte offsets. Neither identical indices nor matching names establish
producer-value equivalence or resource ownership.
"""
import argparse
from collections import Counter
import importlib
import json
from pathlib import Path

from export_original_uniform_contracts import FIXED, ROOT, receipt
from translucent_depth_layout_contract import flatten
from view_layout_contract import current
from localvf_layout_contract import source_layout


def require(condition, reason):
    if not condition:
        raise ValueError(reason)


def resources(layout):
    members = flatten(layout)['members']
    result = [m for m in members if m['base_type'] in (5, 6, 7, 8)]
    require(all(m['elements'] == 0 and m['size'] == 8 for m in result),
            'resource arrays or non-pointer slots require an explicit decoder')
    require(len({m['name'] for m in result}) == len(result), 'duplicate resource path')
    require(len({m['offset'] for m in result}) == len(result), 'overlapping resource slots')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--engine', type=Path, default=ROOT.parents[1]/'UnrealEngine')
    parser.add_argument('--out', type=Path, default=ROOT/'generated/paper2d-shader-semantics/resource-paths.json')
    parser.add_argument('--evidence', type=Path, default=ROOT/'material_pipeline/evidence/paper2d_resource_paths_runtime/evidence.json')
    args = parser.parse_args()
    base = ROOT/'generated/paper2d-shader-semantics'
    manifest_path, contracts_path = base/'shader-manifest.json', base/'original-uniform-contracts.json'
    manifest = json.loads(manifest_path.read_text())
    contracts = json.loads(contracts_path.read_text())
    require(contracts['schema'] == 'paper2d-original-uniform-contracts-v1', 'unsupported original contract schema')
    # This bundle is independently decoded by export_original_uniform_contracts;
    # it is never populated from the current Engine source tree.
    originals = contracts['contracts']
    if isinstance(originals, list):
        originals = {c['parameter_name']: c for c in originals}
    require(set(originals) == set(FIXED), 'expected all six original fixed contracts')
    old, candidates, sources = {}, {}, []
    for name, contract in originals.items():
        old[name] = resources(contract['original'])
        if name == 'View':
            layout = current(args.engine)
            sources.extend(layout['sources'])
        elif name == 'LocalVF':
            layout = source_layout(args.engine)
            sources.extend(layout['sources'])
        else:
            layout, paths = importlib.import_module(FIXED[name][3]).source(args.engine)
            sources.extend(receipt(p) for p in paths)
        ordered = sorted(resources(layout), key=lambda m: m['offset'])
        candidates[name] = {m['name']: dict(m, resource_index=i) for i, m in enumerate(ordered)}
    material_path = ROOT/'material_pipeline/evidence/paper2d_material_uniform_layout_runtime/evidence.json'
    material = json.loads(material_path.read_text())
    old['Material'] = resources(material['original_metadata'])
    rows, counts, descriptors = [], Counter(), 0
    require(len(manifest['shaders']) == 33, 'expected 33 unique original shaders')
    for shader in manifest['shaders']:
        names = [x['uniform_buffer_names'] for x in shader['d3d']['optional_data'] if 'uniform_buffer_names' in x]
        require(len(names) == 1, 'missing or duplicate original uniform names')
        require(not shader['mapped']['unmatched_resource_table_entries'], 'unresolved original table entries')
        for binding in shader['mapped']['bindings']:
            descriptors += 1
            entries = binding['resource_table_entries']
            if binding['kind'] == 'cbv':
                require(not entries, 'constant descriptor unexpectedly has a resource entry')
                continue
            require(len(entries) == 1 and binding['count'] == 1, 'ambiguous resource descriptor')
            entry = entries[0]
            slot, index = entry['uniform_buffer'], entry['resource']
            require(0 <= slot < len(names[0]) and names[0][slot] in old, 'unknown original uniform')
            name = names[0][slot]
            require(0 <= index < len(old[name]), 'original resource index out of range')
            member = old[name][index]
            expected = {'texture': 5, 'shader_resource_view': 6, 'unordered_access_view': 7, 'sampler': 8}
            require(member['base_type'] == expected[entry['table']], 'original descriptor resource type mismatch')
            row = dict(shader_index=shader['archive_index'], uniform=name, source_register=slot,
                       original_resource_index=index, path=name+'.'+member['name'],
                       original_offset=member['offset'], base_type=member['base_type'],
                       descriptor_set=binding['set'], descriptor_binding=binding['binding'])
            if name == 'Material':
                row['status'] = 'requires_factory_metadata'
            else:
                candidate = candidates[name].get(member['name'])
                keys = ('base_type', 'shader_type', 'rows', 'columns', 'elements', 'precision')
                if candidate is None or any(candidate[k] != member[k] for k in keys):
                    row['status'] = 'requires_explicit_resource'
                else:
                    row.update(status='source_path_match', current_offset=candidate['offset'],
                               current_resource_index=candidate['resource_index'])
                    counts['changed_index'] += candidate['resource_index'] != index
                    counts['changed_offset'] += candidate['offset'] != member['offset']
            counts[row['status']] += 1
            rows.append(row)
    require(descriptors == 163 and len(rows) == 100, 'incomplete observed descriptor inventory')
    result = dict(schema='paper2d-resource-paths-v1', runtime_verified=False,
                  resources=rows, counts=dict(counts), source_evidence=sorted(
                      {s['path']: s for s in sources}.values(), key=lambda s:s['path']))
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2)+'\n')
    evidence = dict(counts=dict(counts), resource_descriptors=len(rows),
                    inputs=[receipt(p) for p in (manifest_path, contracts_path, material_path)],
                    output=receipt(args.out), runtime_verified=False,
                    limits='Full paths and resource types identify source candidates only. Current indices use ascending offsets, original indices use recovered traversal order. Factory metadata, producer values, resource ownership, shader binding and rendering remain unverified.',
                    reproduction='python3 porting/material_pipeline/paper2d_resource_paths.py')
    args.evidence.parent.mkdir(parents=True, exist_ok=True)
    args.evidence.write_text(json.dumps(evidence, indent=2)+'\n')
    print(json.dumps(dict(resource_descriptors=len(rows), **counts)))


if __name__ == '__main__':
    main()
