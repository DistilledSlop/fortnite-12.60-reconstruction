#!/usr/bin/env python3
"""Derive the current View prefix layout from checked-out metadata declarations.

This is a source-level ABI calculation, not a build of FViewUniformShaderParameters.
The prefix ends at RealTime, before any arrays or platform-dependent resource types.
The native bridge should still verify the actual metadata offset/STRUCT_OFFSET.
"""
import argparse
import hashlib
import json
import re
from pathlib import Path


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def derive(engine):
    scene = engine / 'Engine/Source/Runtime/Engine/Public/SceneView.h'
    macros = engine / 'Engine/Source/Runtime/RenderCore/Public/ShaderParameterMacros.h'
    text = scene.read_text()
    table = text.split('#define VIEW_UNIFORM_BUFFER_MEMBER_TABLE', 1)[1]
    table = table.split('#define VIEW_UNIFORM_BUFFER_MEMBER(type', 1)[0]
    info = macros.read_text()
    rows = []
    offset = 0
    for typ, name in re.findall(r'VIEW_UNIFORM_BUFFER_MEMBER(?:_EX)?\((\w+),\s*(\w+)', table):
        body = info.split(f'struct TShaderParameterTypeInfo<{typ}>', 1)[1].split('\n};', 1)[0]
        def value(key):
            return int(re.search(r'\b' + key + r'\s*=\s*(\d+)', body)[1])
        alignment = value('Alignment')
        size = 4 * value('NumRows') * value('NumColumns')
        offset = (offset + alignment - 1) // alignment * alignment
        rows.append({'name': name, 'type': typ, 'offset': offset, 'size': size,
                     'alignment': alignment})
        offset += size
        if name == 'RealTime':
            break
    assert rows[-1]['name'] == 'RealTime'
    return {'method': 'source metadata declaration and TShaderParameterTypeInfo ABI calculation',
            'engine_build_performed': False,
            'sources': [{'path': str(p), 'sha256': digest(p)} for p in (scene, macros)],
            'members': rows}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--engine', type=Path, default=Path('/opt/fortnite/UnrealEngine'))
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    result = derive(args.engine)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps([r for r in result['members'] if r['name'] in ('GameTime', 'RealTime')]))
