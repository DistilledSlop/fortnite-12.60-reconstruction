#!/usr/bin/env python3
"""Decode the captured original LocalVF metadata and check its shader wrappers."""
import argparse
import hashlib
import json
from pathlib import Path
import re

from view_layout_contract import align, compare, legacy_resource_hash, original

ROOT = Path(__file__).resolve().parents[1]
CAPTURE = ROOT / 'generated/material_pipeline/localvf-identity'


def source_layout(engine):
    header = engine / 'Engine/Source/Runtime/Engine/Public/LocalVertexFactory.h'
    macros = engine / 'Engine/Source/Runtime/RenderCore/Public/ShaderParameterMacros.h'
    rhi = engine / 'Engine/Source/Runtime/RHI/Public/RHI.h'
    text, types, definitions = header.read_text(), macros.read_text(), rhi.read_text()
    struct_alignment = int(re.search(r'#define SHADER_PARAMETER_STRUCT_ALIGNMENT (\d+)', definitions)[1])
    if not re.search(r'#define SHADER_PARAMETER_POINTER_ALIGNMENT sizeof\(uint64\)', definitions):
        raise ValueError('unsupported pointer alignment')
    body = text.split('BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FLocalVertexFactoryUniformShaderParameters,ENGINE_API)', 1)[1].split('END_GLOBAL_SHADER_PARAMETER_STRUCT()', 1)[0]
    members, offset, resource_index = [], 0, 0
    for line in body.splitlines():
        if not line.strip():
            continue
        m = re.fullmatch(r'\s*SHADER_PARAMETER(_SRV)?\(([^,]+),\s*(\w+)\)\s*', line)
        if not m:
            raise ValueError('unsupported LocalVF declaration: ' + line)
        srv, typ, name = m.groups()
        if srv:
            alignment, size, base, rows, columns, shader_type = 8, 8, 6, 1, 1, typ
        else:
            t = types.split(f'struct TShaderParameterTypeInfo<{typ}>', 1)[1].split('\n};', 1)[0]
            alignment = int(re.search(r'\bAlignment = (\d+)', t)[1])
            rows, columns = (int(re.search(r'\bNum' + n + r' = (\d+)', t)[1]) for n in ('Rows', 'Columns'))
            base = {'UBMT_INT32': 2, 'UBMT_UINT32': 3}[re.search(r'\bBaseType = (UBMT_\w+)', t)[1]]
            size, shader_type = 4 * rows * columns, ''
        offset = align(offset, alignment)
        row = dict(name=name, offset=offset, base_type=base, precision=0, rows=rows, columns=columns, elements=0, size=size, shader_type=shader_type)
        if srv:
            row['resource_index'] = resource_index
            resource_index += 1
        members.append(row)
        offset += size
    return dict(struct_size=align(offset, struct_alignment), members=members,
                sources=[dict(path=str(p), sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in (header, macros, rhi)])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--engine', type=Path, default=ROOT.parent.parent / 'UnrealEngine')
    ap.add_argument('--out', type=Path, default=CAPTURE / 'full-layout-contract.json')
    ap.add_argument('--report', type=Path, default=ROOT / 'generated/paper2d-shader-semantics/conversion/d3d.json')
    args = ap.parse_args()
    chain, initializer = CAPTURE / 'member-chain.json', CAPTURE / '140546830.asm'
    old = original(json.loads(chain.read_text()), initializer.read_text())
    if len(old['members']) != 8 or old['struct_size'] != 64:
        raise ValueError('incomplete original LocalVF metadata')
    expected = legacy_resource_hash(old)
    associations = []
    for record in json.loads(args.report.read_text())['records']:
        arrays = [x['uniform_buffer_names'] for x in record['optional_data'] if 'uniform_buffer_names' in x]
        if len(arrays) != 1 or arrays[0].count('LocalVF') > 1:
            raise ValueError('invalid uniform buffer names')
        if 'LocalVF' not in arrays[0]:
            continue
        slot = arrays[0].index('LocalVF')
        hashes = record['resource_maps']['layout_hash']
        active = bool(record['resource_table_bits'] & (1 << slot))
        actual = hashes[slot] if slot < len(hashes) else 0
        if actual != (expected if active else 0):
            raise ValueError('original LocalVF wrapper hash mismatch')
        associations.append(dict(shader_index=record['shader_index'], uniform_buffer=slot, resource_table_active=active, original_layout_hash=actual))
    if not any(x['resource_table_active'] for x in associations):
        raise ValueError('no active original LocalVF wrappers')
    new = source_layout(args.engine)
    result = dict(original=old, current_source=new, comparison=compare(old, new), resource_layout_hash=expected,
                  shader_associations=associations,
                  evidence=[dict(path=str(p), sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in (chain, initializer, args.report)])
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(dict(original_members=len(old['members']), size=old['struct_size'], resource_layout_hash=expected,
                          shader_associations=len(associations), source_comparison=result['comparison'])))


if __name__ == '__main__':
    main()
