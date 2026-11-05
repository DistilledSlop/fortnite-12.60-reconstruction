#!/usr/bin/env python3
"""Recover the original View member ABI and compare the current source layout.

Uses already captured member helpers; does not launch Ghidra or build the Engine.
The resulting source comparison is not a runtime metadata compatibility receipt.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct

ROOT = Path(__file__).resolve().parents[1]
EVIDENCE = ROOT / 'generated/material_pipeline/view-identity'


def align(value, alignment):
    return (value + alignment - 1) // alignment * alignment


def original(chain, initializer):
    size = re.findall(r'MOV dword ptr \[RSP \+ 0x28\],(0x[0-9a-f]+)', initializer)
    if len(size) != 1 or not chain:
        raise ValueError('missing original View size or helper chain')
    size = int(size[0], 16)
    members, seen = [], set()
    for index, helper in enumerate(chain):
        if helper['function'] in seen or helper['next'] != (chain[index+1]['function'] if index+1 < len(chain) else None):
            raise ValueError('broken or duplicate metadata helper chain')
        seen.add(helper['function'])
        if not helper['strings']:
            if index != len(chain)-1 or [line.split(' ', 1)[1] for line in helper['asm']] != ['XOR EAX,EAX', 'RET']:
                raise ValueError('unsupported unnamed metadata helper')
            continue
        values, known, rax = bytearray(48), set(), None
        pointers = {}
        for line in helper['asm']:
            instruction = line.split(' ', 1)[1]
            if instruction == 'XOR EAX,EAX':
                rax = 0
            elif match := re.fullmatch(r'LEA RAX,\[(0x[0-9a-f]+)\]', instruction):
                rax = int(match[1], 16)
            elif match := re.fullmatch(r'MOV (byte|dword|qword) ptr \[RSP \+ (0x[0-9a-f]+)\],(0x[0-9a-f]+|EAX|RAX)', instruction):
                offset = int(match[2], 16) - 0x20
                if not 0 <= offset < 48:
                    continue
                width = {'byte': 1, 'dword': 4, 'qword': 8}[match[1]]
                value = rax if match[3] in ('EAX', 'RAX') else int(match[3], 16)
                if value is None or offset + width > 48:
                    raise ValueError('unknown metadata field store')
                values[offset:offset+width] = value.to_bytes(width, 'little')
                known.update(range(offset, offset+width))
                if offset in (0, 8):
                    pointers[offset] = value
            elif re.match(r'(?:LEA|MOV) E?R?AX,', instruction):
                rax = None
        # The three padding bytes after BaseType need not be initialized.
        if not (set(range(0, 21)) | set(range(24, 48))).issubset(known):
            raise ValueError('incomplete metadata member fields')
        strings = {int(s['address'], 16): s['text'] for s in helper['strings']}
        if pointers.get(0) not in strings or int.from_bytes(values[40:48], 'little'):
            raise ValueError('unsupported member name or nested metadata')
        offset, = struct.unpack_from('<I', values, 16)
        precision, rows, columns, elements = struct.unpack_from('<4I', values, 24)
        base = values[20]
        if base not in (2, 3, 4, 5, 6, 8) or precision not in (0, 1) or not 1 <= rows <= 4 or not 1 <= columns <= 4 or elements > 4096:
            raise ValueError('unsupported original member shape')
        if base in (5, 6, 8) and (rows != 1 or columns != 1 or elements):
            raise ValueError('unsupported original resource shape')
        member_size = (align(4*rows*columns, 16)*elements if elements else 4*rows*columns) if base in (2, 3, 4) else 8
        if offset + member_size > size:
            raise ValueError('member exceeds original View size')
        members.append(dict(name=strings[pointers[0]], offset=offset, base_type=base,
                            precision=precision, rows=rows, columns=columns, elements=elements,
                            size=member_size, shader_type=strings.get(pointers.get(8), ''),
                            helper=helper['function']))
    members.reverse()
    if len({m['name'] for m in members}) != len(members) or any(a['offset']+a['size'] > b['offset'] for a, b in zip(members, members[1:])):
        raise ValueError('duplicate or overlapping original metadata members')
    resources = [m for m in members if m['base_type'] in (5, 6, 8)]
    for index, member in enumerate(resources):
        member['resource_index'] = index
    return dict(struct_size=size, members=members)


def current(engine):
    paths = {name: engine / path for name, path in {
        'scene': 'Engine/Source/Runtime/Engine/Public/SceneView.h',
        'macros': 'Engine/Source/Runtime/RenderCore/Public/ShaderParameterMacros.h',
        'defines': 'Engine/Source/Runtime/Engine/Public/EngineDefines.h',
        'distance': 'Engine/Source/Runtime/Renderer/Public/GlobalDistanceFieldParameters.h',
        'rhi': 'Engine/Source/Runtime/RHI/Public/RHI.h',
    }.items()}
    text = {name: path.read_text() for name, path in paths.items()}
    struct_align = int(re.search(r'#define SHADER_PARAMETER_STRUCT_ALIGNMENT (\d+)', text['rhi'])[1])
    array_align = int(re.search(r'#define SHADER_PARAMETER_ARRAY_ELEMENT_ALIGNMENT (\d+)', text['rhi'])[1])
    if not re.search(r'#define SHADER_PARAMETER_POINTER_ALIGNMENT sizeof\(uint64\)', text['rhi']):
        raise ValueError('unsupported resource pointer ABI')
    enum = text['scene'].split('enum ETranslucencyVolumeCascade', 1)[1].split('};', 1)[0]
    entries = re.findall(r'\bTVC_\w+\b', enum)
    if '=' in enum or entries[-1] != 'TVC_MAX':
        raise ValueError('unsupported cascade enumeration')
    constants = {'TVC_MAX': len(entries)-1,
                 'NUM_ATMOSPHERE_LIGHTS': int(re.search(r'#define NUM_ATMOSPHERE_LIGHTS (\d+)', text['defines'])[1]),
                 'GMaxGlobalDistanceFieldClipmaps': int(re.search(r'GMaxGlobalDistanceFieldClipmaps = (\d+)', text['distance'])[1])}
    table = text['scene'].split('#define VIEW_UNIFORM_BUFFER_MEMBER_TABLE', 1)[1].split('#define VIEW_UNIFORM_BUFFER_MEMBER(type', 1)[0]
    members, offset = [], 0
    for line in table.splitlines():
        if 'VIEW_UNIFORM_BUFFER_MEMBER' not in line:
            continue
        match = re.fullmatch(r'\s*VIEW_UNIFORM_BUFFER_MEMBER(_EX|_ARRAY)?\((\w+),\s*(\w+)(?:,\s*([^)]*))?\)\s*\\?\s*', line)
        if not match:
            raise ValueError('unsupported numeric View declaration')
        kind, typ, name, extra = match.groups()
        body = text['macros'].split(f'struct TShaderParameterTypeInfo<{typ}>', 1)[1].split('\n};', 1)[0]
        def value(key):
            return int(re.search(r'\b' + key + r'\s*=\s*(\d+)', body)[1])
        base_name = re.search(r'BaseType = (UBMT_\w+)', body)[1]
        base = {'UBMT_INT32': 2, 'UBMT_UINT32': 3, 'UBMT_FLOAT32': 4}[base_name]
        rows, columns = value('NumRows'), value('NumColumns')
        precision = {'EShaderPrecisionModifier::Float': 0, 'EShaderPrecisionModifier::Half': 1}.get(extra, 0) if kind == '_EX' else 0
        if kind == '_EX' and extra not in ('EShaderPrecisionModifier::Float', 'EShaderPrecisionModifier::Half'):
            raise ValueError('unsupported precision modifier')
        elements = 0
        if kind == '_ARRAY':
            bound = re.fullmatch(r'\[(\w+)\]', extra)[1]
            elements = int(bound) if bound.isdigit() else constants[bound]
        size = align(4*rows*columns, array_align)*elements if elements else 4*rows*columns
        offset = align(offset, array_align if elements else value('Alignment'))
        members.append(dict(name=name, offset=offset, base_type=base, precision=precision,
                            rows=rows, columns=columns, elements=elements, size=size, shader_type=''))
        offset += size
    resource_body = text['scene'].split('BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT_WITH_CONSTRUCTOR(FViewUniformShaderParameters, ENGINE_API)', 1)[1].split('END_GLOBAL_SHADER_PARAMETER_STRUCT()', 1)[0]
    resource_index = 0
    for line in resource_body.splitlines():
        line = line.split('//', 1)[0].strip()
        if not line or line == 'VIEW_UNIFORM_BUFFER_MEMBER_TABLE':
            continue
        match = re.fullmatch(r'SHADER_PARAMETER_(TEXTURE|SAMPLER|SRV|UAV)\(([^,]+),\s*(\w+)\)', line)
        if not match:
            raise ValueError('unsupported View resource declaration')
        kind, typ, name = match.groups()
        offset = align(offset, 8)
        members.append(dict(name=name, offset=offset, base_type={'TEXTURE': 5, 'SRV': 6, 'UAV': 7, 'SAMPLER': 8}[kind],
                            precision=0, rows=1, columns=1, elements=0, size=8,
                            shader_type=typ, resource_index=resource_index))
        offset += 8
        resource_index += 1
    return dict(struct_size=align(offset, struct_align), members=members,
                sources=[dict(path=str(p), sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in paths.values()])


def compare(old, new):
    left, right = ({m['name']: m for m in layout['members']} for layout in (old, new))
    fields = ('offset', 'base_type', 'precision', 'rows', 'columns', 'elements', 'size', 'shader_type', 'resource_index')
    differences = []
    for name in left.keys() | right.keys():
        changes = {key: [left[name].get(key), right[name].get(key)] for key in fields
                   if name in left and name in right and left[name].get(key) != right[name].get(key)}
        if name not in left or name not in right:
            changes['presence'] = [name in left, name in right]
        if changes:
            differences.append(dict(name=name, changes=changes))
    return dict(exact_source_layout_match=not differences and old['struct_size'] == new['struct_size'],
                differences=sorted(differences, key=lambda row: row['name']))


def legacy_resource_hash(layout):
    """Pre-static-slot resource layout hash, checked against original wrappers."""
    resources = [m for m in layout['members'] if 'resource_index' in m]
    result = layout['struct_size'] << 16
    for member in resources:
        if member['offset'] % 8:
            raise ValueError('unaligned original resource pointer')
        result ^= member['offset']
    count = len(resources)
    while count >= 4:
        for shift in (0, 8, 16, 24):
            count -= 1
            result ^= resources[count]['base_type'] << shift
    while count >= 2:
        for shift in (0, 16):
            count -= 1
            result ^= resources[count]['base_type'] << shift
    while count:
        count -= 1
        result ^= resources[count]['base_type']
    return result & 0xffffffff


def verify_shader_views(layout, report):
    expected, associations = legacy_resource_hash(layout), []
    for record in report['records']:
        name_arrays = [item['uniform_buffer_names'] for item in record['optional_data'] if 'uniform_buffer_names' in item]
        if len(name_arrays) != 1:
            raise ValueError('missing or duplicate original uniform buffer names')
        names = name_arrays[0]
        if names.count('View') > 1:
            raise ValueError('duplicate original View binding')
        if 'View' not in names:
            continue
        index = names.index('View')
        hashes = record['resource_maps']['layout_hash']
        active = bool(record['resource_table_bits'] & (1 << index))
        recorded = hashes[index] if index < len(hashes) else 0
        # Constant-only uses have no active resource table or layout hash.
        if (active and recorded != expected) or (not active and recorded != 0):
            raise ValueError('recovered View layout differs from original shader wrapper')
        associations.append(dict(shader_index=record['shader_index'], uniform_buffer=index,
                                 resource_table_active=active, original_layout_hash=recorded))
    if not any(row['resource_table_active'] for row in associations):
        raise ValueError('no original View shader associations')
    return associations


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--evidence', type=Path, default=EVIDENCE)
    parser.add_argument('--engine', type=Path, default=Path('/opt/fortnite/UnrealEngine'))
    parser.add_argument('--shader-report', type=Path, default=ROOT / 'generated/paper2d-shader-semantics/conversion/d3d.json')
    parser.add_argument('--out', type=Path, default=EVIDENCE / 'full-layout-contract.json')
    args = parser.parse_args()
    if (ROOT / 'generated').resolve() not in args.out.resolve().parents:
        parser.error('output must stay under porting/generated')
    chain = args.evidence / 'member-chain.json'
    initializer = args.evidence / '140555220.asm'
    old = original(json.loads(chain.read_text()), initializer.read_text())
    new = current(args.engine)
    associations = verify_shader_views(old, json.loads(args.shader_report.read_text()))
    old['resource_layout_hash'] = legacy_resource_hash(old)
    result = dict(original=old, current=new, comparison=compare(old, new), runtime_verified=False,
                  original_shader_view_associations=associations,
                  evidence=[dict(path=str(p), sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in (chain, initializer, args.shader_report)])
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, allow_nan=False) + '\n')
    print(f'View: original {len(old["members"])} members/{old["struct_size"]} bytes; current {len(new["members"])} members/{new["struct_size"]} bytes; {len(result["comparison"]["differences"])} differences')


if __name__ == '__main__':
    main()
