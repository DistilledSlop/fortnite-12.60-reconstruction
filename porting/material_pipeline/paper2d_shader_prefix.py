#!/usr/bin/env python3
"""Bounded Paper2D prefix decoder; preserves unknown bytes and fails closed.

Prefix grammar is copied from legacy_shader_map.py with Paper2D identity support.
Known shader bodies reuse its verified record decoder. No unknown record is skipped.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path

def sibling(name):
    spec=importlib.util.spec_from_file_location(name,Path(__file__).with_name(name+'.py'))
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module

legacy=sibling('legacy_shader_map')
records=sibling('legacy_shader_records')
Reader,MAX_BYTES,ARRAYS=legacy.Reader,legacy.MAX_BYTES,legacy.ARRAYS
material_shader_records=records.material_shader_records

def paper2d_mesh_maps(reader,count,map_hash,expression_counts):
    maps=[]
    for _ in range(count):
        start=reader.cursor;vf=reader.name()
        if vf != {'name':'FLocalVertexFactory','number':0}:
            raise ValueError('unsupported mesh vertex factory')
        shader_count=reader.count(); parsed=[]
        item={'offset':start,'vertex_factory':vf,'count':shader_count,'records':parsed}
        maps.append(item)
        for _ in range(shader_count):
            start=reader.cursor
            kind=reader.name();reader.cursor=start
            if kind['name'] not in records.MESH_VS|records.MESH_PS|records.MESH_GS:
                item['first_unknown']={'offset':start,'type':kind}
                return maps
            parsed.append(records.shader_record(reader,map_hash,expression_counts,vf))
        pipelines=reader.count()
        if pipelines: raise ValueError('unsupported nonempty mesh shader pipelines')
        item.update(pipeline_count=pipelines,end=reader.cursor)
    return maps

def parse_prefix(data, names):
    """Return only the verified prefix; never assert a complete shader map.

    Nonempty VT stacks and parameter-binding arrays remain unsupported.
    """
    if len(data) > MAX_BYTES:
        raise ValueError('legacy resource exceeds size bound')
    reader = Reader(data, names)
    cooked, valid, quality, feature = reader.read('4I')
    if cooked != 1 or valid != 1 or quality > 5 or feature > 6:
        raise ValueError('unexpected cooked material resource header')
    shader_map_hash = bytes(reader.read('20B')).hex()
    platform = reader.read('I')
    if platform != 0:
        raise ValueError('only original PCD3D_SM5 maps have been verified')
    friendly, path = reader.string(), reader.string()
    if not path.startswith('/Paper2D/') or not path.endswith('.' + friendly):
        raise ValueError('inconsistent legacy shader-map identity')
    arrays = {}
    for name in ARRAYS:
        if name == 'texture_2d_array':
            # 4.24 inserts VTStacks before Uniform2DArrayTextureExpressions.
            # These are not expression records: never interpret a nonzero stack
            # count as an expression-array count or search for a later boundary.
            start = reader.cursor
            count = reader.count()
            if count:
                raise ValueError(f'unsupported nonempty VT stacks at {start}')
            vt_stacks = {'offset': start, 'end': reader.cursor, 'count': count}
        start = reader.cursor
        count = reader.count()
        expressions = [reader.expression() for _ in range(count)]
        arrays[name] = {'offset': start, 'end': reader.cursor, 'count': count,
                        'expressions': expressions}
    start = reader.cursor
    count = reader.count()
    collections = {'offset': start, 'count': count,
                   'guids': [reader.read('4I') for _ in range(count)],
                   'end': reader.cursor}
    start = reader.cursor
    compilation = {'offset': start, 'used_scene_textures': reader.read('I'),
                   'estimated_texture_samples_vs': reader.read('H'),
                   'estimated_texture_samples_ps': reader.read('H'),
                   'estimated_virtual_texture_lookups': reader.read('H'),
                   'used_uv_scalars': reader.read('B'),
                   'used_custom_interpolator_scalars': reader.read('B'),
                   'end': reader.cursor}
    # Original FMaterialCompilationOutput::Serialize 0x144338180 reads the
    # output mask at object+0x134, then flags shifted into object+0x135 bits1..7.
    start = reader.cursor
    mask, flags = reader.read('BB')
    if flags & 0x80:
        raise ValueError('unsupported compilation flag bit')
    compilation_flags = {'offset': start, 'runtime_virtual_texture_output_attribute_mask': mask,
                         'packed_flags': flags, 'end': reader.cursor}
    flag_names = ('uses_eye_adaptation', 'modifies_mesh_position', 'uses_world_position_offset',
                  'uses_global_distance_field', 'uses_pixel_depth_offset', 'uses_distance_cull_fade',
                  'has_runtime_virtual_texture_output_node')
    compilation_flags.update({name: bool(flags & (1 << bit)) for bit, name in enumerate(flag_names)})
    debug_description = reader.string()
    shader_records = material_shader_records(reader, shader_map_hash,
        [arrays[name]['count'] for name in ARRAYS[:6]] + [arrays['texture_2d_array']['count']])
    mesh_maps = paper2d_mesh_maps(reader, shader_records['mesh_shader_maps_count'], shader_map_hash,
        [arrays[name]['count'] for name in ARRAYS[:6]] + [arrays['texture_2d_array']['count']])
    return {'complete_serialized_shader_map': False, 'runtime_loading_verified': False,
            'cooked': cooked, 'valid': valid, 'quality_level': quality, 'feature_level': feature,
            'shader_platform': platform, 'cooked_shader_map_id_hash': shader_map_hash,
            'friendly_name': friendly, 'material_path': path, 'uniform_expression_arrays': arrays,
            'virtual_texture_stacks': vt_stacks, 'parameter_collections': collections,
            'compilation_output_scalars': compilation, 'compilation_output_flags': compilation_flags,
            'debug_description': debug_description, 'material_shaders': shader_records,
            'mesh_shader_maps': mesh_maps, 'decoded_prefix_bytes': reader.cursor,
            'undecoded_bytes': len(data)-reader.cursor,
            'first_unknown': next((m['first_unknown'] for m in mesh_maps if 'first_unknown' in m), None),
            'limitation': 'Stops at first unknown shader type; no exact EOF or runtime/parity claim'}

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--uexp',type=Path,required=True)
    parser.add_argument('--resource-report',type=Path,required=True)
    parser.add_argument('--shader-archive',type=Path,required=True)
    parser.add_argument('--report',type=Path,required=True)
    args=parser.parse_args()
    generated=Path(__file__).resolve().parents[1]/'generated'
    if generated.resolve() not in args.report.resolve().parents:
        raise ValueError('report must stay in porting/generated')
    framing=json.loads(args.resource_report.read_text());raw=args.uexp.read_bytes()
    if not 0 <= framing['resource_start'] < framing['resource_end'] <= len(raw) <= MAX_BYTES:
        raise ValueError('invalid bounded resource')
    if hashlib.sha256(raw).hexdigest()!=framing['uexp_sha256']: raise ValueError('uexp changed')
    data=raw[framing['resource_start']:framing['resource_end']]
    if hashlib.sha256(data).hexdigest()!=framing['resource_sha256']: raise ValueError('resource changed')
    result=parse_prefix(data,framing['names'])
    result['shader_record_count']=len(result['material_shaders']['records'])+sum(
        len(m['records']) for m in result['mesh_shader_maps'])
    archive_module=sibling('shader_archive')
    with archive_module.ShaderArchive(args.shader_archive,layout='fortnite-12.60') as archive:
        legacy.associate_library(result,archive)
    result['input_sha256']={str(p):archive_module.sha256(p) for p in [args.uexp,args.resource_report,args.shader_archive,Path(__file__),Path(legacy.__file__),Path(records.__file__)]}
    args.report.parent.mkdir(parents=True,exist_ok=True)
    args.report.write_text(json.dumps(result,indent=2)+'\n')
    print(result['shader_record_count'],result['decoded_prefix_bytes'],result['undecoded_bytes'])
if __name__=='__main__': main()
