#!/usr/bin/env python3
"""Strict, partial decoder of the pre-memory-image Fortnite 12.60 shader map.

This decodes serialized metadata, never an executable shader map. Unknown
expression/shader types fail closed. The complete verified grammar must end
exactly at the declared resource boundary.
The recursive grammar matches Epic's pre-freeze serializers, available at:
https://github.com/folgerwang/UnrealEngine/blob/release/Engine/Source/Runtime/Engine/Private/Materials/MaterialUniformExpressions.h
https://github.com/folgerwang/UnrealEngine/blob/release/Engine/Source/Runtime/Engine/Private/Materials/MaterialUniformExpressions.cpp
Texture base widths int32,int16,int16,int32,bool32 are recovered from the
original runtime and match the 4.24 pre-freeze serializer, also documenting
the collection/compilation scalar order:
https://github.com/HenryZheng1/UnrealEngine-4.24.3-release/blob/master/Engine/Source/Runtime/Engine/Private/Materials/MaterialUniformExpressions.cpp
https://github.com/HenryZheng1/UnrealEngine-4.24.3-release/blob/master/Engine/Source/Runtime/Engine/Private/Materials/MaterialShared.cpp
No LayoutParams are present. Seven Slate and twelve LocalVF mesh records per
original sample are decoded. All offsets are resource-relative.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct


def _load_sibling(name):
    """Load trusted sibling code even when Python -I removes script paths."""
    spec = importlib.util.spec_from_file_location(name, Path(__file__).with_name(name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


_records = _load_sibling('legacy_shader_records')
material_shader_records, mesh_shader_maps = _records.material_shader_records, _records.mesh_shader_maps

MAX_BYTES = 64 * 1024 * 1024
ARRAYS = ('vector', 'scalar', 'texture_2d', 'texture_cube', 'texture_volume',
          'texture_virtual', 'texture_external', 'texture_2d_array')
PREFIX = 'FMaterialUniformExpression'


class Reader:
    def __init__(self, data, names):
        self.data, self.names, self.cursor, self.nodes = data, names, 0, 0
        self.limit = len(data)

    def read(self, fmt):
        size = struct.calcsize('<' + fmt)
        if self.cursor + size > self.limit:
            raise ValueError(f'truncated legacy shader-map field at {self.cursor}')
        value = struct.unpack_from('<' + fmt, self.data, self.cursor)
        self.cursor += size
        return value[0] if len(value) == 1 else list(value)

    def count(self):
        count = self.read('i')
        if not 0 <= count <= 4096:
            raise ValueError(f'invalid legacy array count at {self.cursor - 4}')
        return count

    def name(self):
        index, number = self.read('ii')
        if not 0 <= index < len(self.names) or number < 0:
            raise ValueError(f'invalid resource FName at {self.cursor - 8}')
        return {'name': self.names[index], 'number': number}

    def string(self):
        count = self.read('i')
        if not 1 <= count <= 4096 or self.cursor + count > self.limit:
            raise ValueError('invalid identity FString length')
        raw = self.data[self.cursor:self.cursor + count]
        self.cursor += count
        if raw[-1] != 0 or b'\0' in raw[:-1]:
            raise ValueError('invalid identity FString terminator')
        return raw[:-1].decode('utf-8')

    def parameter(self):
        result = {'name': self.name(), 'association': self.read('B'), 'index': self.read('i')}
        if result['association'] > 2 or result['index'] < -1:
            raise ValueError('invalid material parameter info')
        return result

    def expression(self, depth=0):
        self.nodes += 1
        if depth >= 64 or self.nodes > 65536:
            raise ValueError('legacy expression complexity limit exceeded')
        start = self.cursor
        tag = self.name()
        kind = tag['name'].removeprefix(PREFIX)
        if tag['number'] or not tag['name'].startswith(PREFIX):
            raise ValueError(f'invalid expression type at {start}')
        result = {'offset': start, 'type': tag['name']}
        if kind in ('VectorParameter', 'ScalarParameter'):
            result['parameter'] = self.parameter()
            result['default'] = self.read('4f' if kind == 'VectorParameter' else 'f')
        elif kind in ('Texture', 'TextureParameter'):
            if kind == 'TextureParameter':
                result['parameter'] = self.parameter()
            # Original 0x14433eb60 serializes this base payload in both types.
            texture_index, layer, page_layer, sampler, virtual = self.read('ihhiI')
            if (texture_index < -1 or layer < -1 or page_layer < -1
                    or sampler not in (0, 1, 2) or virtual not in (0, 1)):
                raise ValueError(f'invalid original texture expression at {start}')
            result['texture'] = {'texture_index': texture_index, 'layer_index': layer,
                                 'page_table_layer_index': page_layer,
                                 'sampler_source': sampler, 'virtual_texture': bool(virtual)}
        elif kind == 'TextureProperty':
            result['inputs'] = [self.expression(depth + 1)]
            result['property'] = self.read('B')
            if result['property'] not in (0, 1):
                raise ValueError(f'invalid texture property at {start}')
        elif kind == 'Constant':
            result['value'], result['value_type'] = self.read('4f'), self.read('B')
        elif kind in ('ComponentSwizzle', 'FoldedMath', 'AppendVector', 'TrigMath', 'Max', 'Saturate'):
            result['inputs'] = [self.expression(depth + 1)]
            if kind not in ('ComponentSwizzle', 'Saturate'):
                result['inputs'].append(self.expression(depth + 1))
            if kind == 'ComponentSwizzle':
                channels = self.read('4b')
                elements = self.read('B')
                if not 1 <= elements <= 4 or any(not -1 <= x <= 3 for x in channels):
                    raise ValueError('invalid component swizzle')
                result.update(channels=channels, elements=elements)
            elif kind == 'FoldedMath':
                result['operation'], result['value_type'] = self.read('B'), self.read('I')
                if result['operation'] > 5:
                    raise ValueError('invalid folded math operation')
            elif kind == 'AppendVector':
                result['components_a'] = self.read('I')
                if not 1 <= result['components_a'] <= 4:
                    raise ValueError('invalid append component count')
            elif kind == 'TrigMath':
                result['operation'] = self.read('B')
                if result['operation'] > 6:
                    raise ValueError('invalid trigonometric operation')
        else:
            raise ValueError(f'unsupported expression {tag["name"]} at {start}')
        result['end'] = self.cursor
        return result


def parse_prefix(data, names):
    """Return the verified serialized shader map with exact resource EOF.

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
    if not path.startswith('/Game/') or not path.endswith('.' + friendly):
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
    mesh_maps = mesh_shader_maps(reader, shader_records['mesh_shader_maps_count'], shader_map_hash,
        [arrays[name]['count'] for name in ARRAYS[:6]] + [arrays['texture_2d_array']['count']])
    cooking_offset = reader.cursor
    if reader.read('I') != 1:
        raise ValueError('only cooked original material shader maps are supported')
    if reader.cursor != len(data):
        raise ValueError(f'unexpected trailing shader-map bytes at {reader.cursor}')
    cursor = reader.cursor
    return {'complete_shader_map': True, 'complete_serialized_shader_map': True,
            'runtime_loading_verified': False,
            'limitations': ['Complete serialized metadata does not establish runtime loading, shader semantics or draw parity',
                'Nonempty VT stacks are unsupported',
                'Nonempty parameter-binding arrays and shader pipelines are unsupported',
                'Only the observed Slate and LocalVF mesh shader types are supported'],
            'cooked': cooked, 'valid': valid,
            'quality_level': quality, 'feature_level': feature,
            'cooked_shader_map_id_hash': shader_map_hash, 'shader_platform': platform,
            'friendly_name': friendly, 'material_path': path,
            'uniform_expression_arrays': arrays, 'expression_nodes': reader.nodes,
            'virtual_texture_stacks': vt_stacks, 'parameter_collections': collections,
            'compilation_output_scalars': compilation,
            'compilation_output_flags': compilation_flags,
            'debug_description': debug_description, 'material_shaders': shader_records,
            'mesh_shader_maps': mesh_maps,
            'shader_record_count': shader_records['count'] + sum(m['count'] for m in mesh_maps),
            'cooking': {'offset': cooking_offset, 'value': True, 'end': reader.cursor},
            'original_serializer_evidence': {
                'runtime_dump_sha256': '604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d',
                'texture': '0x14433eb60', 'uniform_expression_set': '0x144364bf0',
                'compilation_output': '0x144338180', 'material_shader_map': '0x1443383f0',
                'material_shader': '0x1434f9590', 'slate_vs': '0x143cfa820',
                'slate_ps': '0x143cfb4e0', 'shader_base': '0x143aaae20',
                'shader_resource': '0x143aaab80', 'mesh_material_shader': '0x1434f99c0',
                'base_pass_shader': '0x1431e2ea0', 'shadow_depth_pixel_shader': '0x143237540',
                'vertex_factory_parameter_ref': '0x143a7b010',
                'local_vertex_factory_parameter_constructor': '0x1442f01e0',
                'local_vertex_factory_parameters': '0x1442fc610'},
            'tail_grammar_sources': [
                'https://github.com/HenryZheng1/UnrealEngine-4.24.3-release/blob/master/Engine/Source/Runtime/Engine/Private/Materials/MaterialUniformExpressions.cpp#L106',
                'https://github.com/HenryZheng1/UnrealEngine-4.24.3-release/blob/master/Engine/Source/Runtime/Engine/Private/Materials/MaterialShared.cpp#L489'],
            'decoded_prefix_bytes': cursor,
            'first_unknown': None,
            'undecoded_bytes': len(data) - cursor,
            'resource_sha256': hashlib.sha256(data).hexdigest()}


def associate_library(result, archive):
    """Associate parsed record identities with a fully validated archive index."""
    records = result['material_shaders']['records'] + [
        record for mesh_map in result.get('mesh_shader_maps', []) for record in mesh_map['records']]
    for record in records:
        index = archive.shader_lookup.get(bytes.fromhex(record['output_hash']['hash']))
        if index is None:
            raise ValueError('decoded shader hash is absent from supplied library')
        _, compressed, expanded, frequency = archive.shaders[index]
        if frequency != record['frequency'] or expanded != record['resource']['uncompressed_code_bytes']:
            raise ValueError('library shader target/size disagrees with decoded resource')
        record['library_association'] = {'index': index, 'frequency': frequency,
                                         'compressed_bytes': compressed, 'uncompressed_bytes': expanded}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--uexp', type=Path, required=True)
    parser.add_argument('--resource-report', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--shader-archive', type=Path,
                        help='validate decoded record identities against the original Fortnite shader library')
    args = parser.parse_args()
    try:
        generated = Path(__file__).resolve().parents[1] / 'generated'
        if generated.resolve() not in args.report.resolve().parents:
            raise ValueError('report must stay in porting/generated')
        framing = json.loads(args.resource_report.read_text())
        raw = args.uexp.read_bytes()
        if len(raw) > MAX_BYTES or hashlib.sha256(raw).hexdigest() != framing['uexp_sha256']:
            raise ValueError('uexp identity does not match resource report')
        start, end = framing['resource_start'], framing['resource_end']
        if not 0 <= start < end <= len(raw):
            raise ValueError('invalid resource bounds')
        result = parse_prefix(raw[start:end], framing['names'])
        if result['resource_sha256'] != framing['resource_sha256']:
            raise ValueError('resource identity does not match resource report')
        result.update(package=framing['package'], resource_start=start, resource_end=end,
                      first_unknown_uexp_offset=None)
        result['original_serializer_evidence']['probe_directory'] = str(generated / 'material_pipeline')
        result['original_serializer_evidence']['decompilation_files'] = {
            name: str(generated / 'material_pipeline' / f'original-{address}.c')
            for name, address in result['original_serializer_evidence'].items()
            if isinstance(address, str) and address.startswith('0x')}
        if args.shader_archive:
            library_parser = _load_sibling('shader_archive')
            with library_parser.ShaderArchive(args.shader_archive, layout='fortnite-12.60') as archive:
                associate_library(result, archive)
                result['shader_library'] = {'path': str(args.shader_archive.resolve()),
                    'sha256': library_parser.sha256(args.shader_archive), 'bytes': len(archive.data),
                    'layout': archive.layout, 'associated_records': result['shader_record_count']}
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(result, indent=2, allow_nan=False) + '\n')
        print(f'{result["expression_nodes"]} expression nodes; '
              f'{result["shader_record_count"]} shader records; exact resource EOF '
              f'{result["resource_end"]}; {args.report}')
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
