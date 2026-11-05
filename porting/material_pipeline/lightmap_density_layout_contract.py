#!/usr/bin/env python3
"""Compare captured LightmapDensityPass metadata to source and shader wrappers."""
import argparse
import hashlib
import json
from pathlib import Path
import re

from translucent_depth_layout_contract import flatten
from view_layout_contract import align, compare, legacy_resource_hash, original

ROOT = Path(__file__).resolve().parents[1]
CAP = ROOT / 'generated/material_pipeline'


def evidence(path):
    return dict(path=str(path), sha256=hashlib.sha256(path.read_bytes()).hexdigest())


def read_original():
    scene_chain = CAP / 'translucent-depth-scene/member-chain.json'
    scene_asm = CAP / 'translucent-depth-nested/1404c9240.asm'
    top_chain = CAP / 'lightmap-density-identity/member-chain.json'
    top_asm = CAP / 'lightmap-density-identity/1404abf10.asm'
    scene_helpers, top_helpers = (json.loads(p.read_text()) for p in (scene_chain, top_chain))
    scene_init, top_init = (p.read_text() for p in (scene_asm, top_asm))
    for initializer, tokens in [(scene_init, ('LEA RCX,[0x1482d2390]', 'LEA RAX,[0x' + scene_helpers[0]['function'] + ']')), (top_init, ('LEA RCX,[0x14829f260]', 'LEA RAX,[0x1466b2b18]', 'LEA RAX,[0x' + top_helpers[0]['function'] + ']'))]:
        if any(t not in initializer for t in tokens):
            raise ValueError('initializer identity mismatch')
    scene = original(scene_helpers, scene_init)
    layout = original(top_helpers, top_init, {0x1482d2390: dict(type='FSceneTexturesUniformParameters', layout=scene)})
    if sum(m.get('metadata_object') == '0x1482d2390' for m in layout['members']) != 1:
        raise ValueError('missing or duplicate SceneTextures metadata')
    return layout, [scene_chain, scene_asm, top_chain, top_asm]


def source(engine):
    base = engine / 'Engine/Source/Runtime'
    files = [base/'Renderer/Private/LightMapDensityRendering.h', base/'Renderer/Public/SceneRenderTargetParameters.h', base/'RenderCore/Public/ShaderParameterMacros.h', base/'RHI/Public/RHI.h']
    texts = [p.read_text() for p in files]
    if not re.search(r'#define SHADER_PARAMETER_POINTER_ALIGNMENT sizeof\(uint64\)', texts[3]):
        raise ValueError('unsupported pointer ABI')
    struct_align = int(re.search(r'#define SHADER_PARAMETER_STRUCT_ALIGNMENT (\d+)', texts[3])[1])
    bodies = {}
    for typ, text in [('FLightmapDensityPassUniformParameters', texts[0]), ('FSceneTexturesUniformParameters', texts[1])]:
        matches = re.findall(r'BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT\(' + typ + r'\s*,[^\n]*\)(.*?)END_GLOBAL_SHADER_PARAMETER_STRUCT\(\)', text, re.S)
        if len(matches) != 1: raise ValueError('missing source structure ' + typ)
        bodies[typ] = matches[0]
    def decode(typ):
        members, offset = [], 0
        for line in bodies[typ].splitlines():
            line = line.split('//', 1)[0].strip()
            if not line: continue
            match = re.fullmatch(r'SHADER_PARAMETER(_STRUCT|_TEXTURE|_SAMPLER|_SRV)?\(([^,]+),\s*(\w+)\)', line)
            if not match: raise ValueError('unsupported source member ' + line)
            kind, t, name = match.groups()
            if kind == '_STRUCT':
                child = decode(t); size = child['struct_size']; alignment = struct_align
                m = dict(name=name, base_type=17, precision=0, rows=1, columns=1, elements=0, size=size, shader_type=t, members=child['members'])
            elif kind:
                size = alignment = 8
                m = dict(name=name, base_type={'_TEXTURE':5, '_SAMPLER':8, '_SRV':6}[kind], precision=0, rows=1, columns=1, elements=0, size=size, shader_type=t)
            else:
                definition = texts[2].split(f'struct TShaderParameterTypeInfo<{t}>', 1)[1].split('\n};', 1)[0]
                def val(k): return int(re.search(r'\b' + k + r'\s*=\s*(\d+)', definition)[1])
                alignment = val('Alignment'); rows = val('NumRows'); cols = val('NumColumns'); size = 4 * rows * cols
                m = dict(name=name, base_type={'UBMT_FLOAT32':4}[re.search(r'BaseType = (UBMT_\w+)', definition)[1]], precision=0, rows=rows, columns=cols, elements=0, size=size, shader_type='')
            offset = align(offset, alignment); m['offset'] = offset; offset += size; members.append(m)
        return dict(struct_size=align(offset, struct_align), members=members)
    return decode('FLightmapDensityPassUniformParameters'), files


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--engine', type=Path, default=ROOT.parent.parent/'UnrealEngine')
    p.add_argument('--report', type=Path, default=ROOT/'generated/paper2d-shader-semantics/conversion/d3d.json')
    p.add_argument('--out', type=Path, default=ROOT/'behavior/lightmap_density_uniform_layout_runtime/evidence.json')
    args = p.parse_args()
    old, inputs = read_original(); new, files = source(args.engine)
    flat_old, flat_new = flatten(old), flatten(new)
    expected = legacy_resource_hash(flat_old); associations = []
    for record in json.loads(args.report.read_text())['records']:
        arrays = [x['uniform_buffer_names'] for x in record['optional_data'] if 'uniform_buffer_names' in x]
        if len(arrays) != 1 or arrays[0].count('LightmapDensityPass') > 1: raise ValueError('invalid uniform names')
        if 'LightmapDensityPass' not in arrays[0]: continue
        slot = arrays[0].index('LightmapDensityPass'); hashes = record['resource_maps']['layout_hash']
        active = bool(record['resource_table_bits'] & (1 << slot)); actual = hashes[slot] if slot < len(hashes) else 0
        if actual != (expected if active else 0): raise ValueError('shader wrapper layout hash mismatch')
        associations.append(dict(shader_index=record['shader_index'], uniform_buffer=slot, resource_table_active=active, original_layout_hash=actual))
    if not associations or not any(a['resource_table_active'] for a in associations): raise ValueError('no active shader wrapper corroboration')
    comparison = compare(flat_old, flat_new)
    comparison['member_order_match'] = [m['name'] for m in flat_old['members']] == [m['name'] for m in flat_new['members']]
    comparison['exact_source_layout_match'] &= comparison['member_order_match']
    result = dict(original_metadata=dict(initializer='0x1404abf10', metadata_object='0x14829f260', first_helper='0x143329650', nested_initializer='0x1404c9240', **old), current_source=new, comparison=comparison, computed_resource_layout_hash=expected, shader_associations=associations, hash_corroborated=True, runtime_verified=False, current_source_evidence=[evidence(x) for x in files], capture_evidence=[evidence(x) for x in inputs + [args.report]], reproduction='From repository root: python3 porting/material_pipeline/lightmap_density_layout_contract.py', limits='Original and nested metadata are decoded from captured instructions. The source layout is derived, not compiled; no runtime binding or rendering validation.')
    args.out.parent.mkdir(parents=True, exist_ok=True); args.out.write_text(json.dumps(result, indent=2, allow_nan=False) + '\n')
    print(json.dumps(dict(original_size=old['struct_size'], current_size=new['struct_size'], differences=len(comparison['differences']), member_order_match=comparison['member_order_match'], computed_hash=expected, associations=associations)))

if __name__ == '__main__': main()
