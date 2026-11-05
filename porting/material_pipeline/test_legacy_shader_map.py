#!/usr/bin/env python3
"""Synthetic grammar tests; original-file evidence is produced by the CLI."""
import struct
import json
import hashlib
import unittest
import importlib.util
from types import SimpleNamespace
from pathlib import Path
spec = importlib.util.spec_from_file_location('legacy_shader_map', Path(__file__).with_name('legacy_shader_map.py'))
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
parse_prefix, Reader, ARRAYS, PREFIX = module.parse_prefix, module.Reader, module.ARRAYS, module.PREFIX


def pack(fmt, *values):
    return struct.pack('<' + fmt, *values)


def string(value):
    raw = value.encode() + b'\0'
    return pack('i', len(raw)) + raw


NAMES = [PREFIX + x for x in ('VectorParameter', 'ScalarParameter', 'Constant',
         'ComponentSwizzle', 'FoldedMath', 'AppendVector', 'TrigMath', 'TextureParameter')]
NAMES += ['Tint', 'Icon', 'NotAnExpression', PREFIX + 'Unknown']
NAMES += ['TSlateMaterialShaderVStrue', 'None', 'Material']
NAMES += ['FLocalVertexFactory',
          'TTranslucencyShadowDepthVS<TranslucencyShadowDepth_Standard>',
          'TTranslucencyShadowDepthPS<TranslucencyShadowDepth_Standard>',
          'TBasePassVSFNoLightMapPolicy', 'TBasePassPSFNoLightMapPolicy',
          'FConvertToUniformMeshVS', 'FConvertToUniformMeshGS']


def name(index):
    return pack('ii', index, 0)


def parameter(index):
    return name(index) + pack('Bi', 2, -1)


VECTOR = name(0) + parameter(8) + pack('4f', 0.125, 0.25, 0.5, 1)
SCALAR = name(1) + parameter(8) + pack('f', 2.5)
CONSTANT = name(2) + pack('4fB', 1, 2, 3, 4, 15)
FOLDED = name(4) + SCALAR + CONSTANT + pack('BI', 2, 15)
TRIG = name(6) + FOLDED + FOLDED + pack('B', 0)
APPEND = name(5) + TRIG + VECTOR + pack('I', 1)
SWIZZLE = name(3) + APPEND + pack('4bB', 0, 1, 2, -1, 3)
TEXTURE = name(7) + parameter(9) + pack('ihhiI', 0, -1, -1, 0, 0)
HEADER = pack('4I', 1, 1, 1, 3) + bytes(range(20)) + pack('I', 0)
HEADER += string('M_Test') + string('/Game/UI/M_Test.M_Test')


def resource(arrays=None, tail=b'', collections=(), vt_count=0):
    arrays = arrays or [[VECTOR, SWIZZLE], [SCALAR], [TEXTURE]] + [[] for _ in range(5)]
    encoded = [pack('i', len(a)) + b''.join(a) for a in arrays]
    encoded.insert(7, pack('i', vt_count))
    prefix = (HEADER + b''.join(encoded) + pack('i', len(collections))
            + b''.join(pack('4I', *guid) for guid in collections)
            + pack('IHHHBB', 5, 2, 3, 4, 6, 7) + pack('BB', 0, 0)
            + string('Compiling M_Test'))
    counts = [len(a) for a in arrays[:6]] + [len(arrays[7])]
    output_hash = bytes(reversed(range(20)))
    body = (pack('HIHIi', 0, 0, 1, 1, 0) + pack('7i', *counts)
            + name(14) + pack('Iii', 128, 0, 0) + string('M_Test')
            + pack('8H', *([0] * 8))  # feedback resource and two scalar parameters
            + output_hash + bytes(range(20)) + name(13) + name(13) + bytes(20)
            + name(12) + pack('i', 0) + bytes(20) + pack('IIi', 0, 0, 0)
            + name(13) + pack('iII', 0, 0, 0) + output_hash
            + pack('I4iII', 17, 0, 0, 0, 0, 1095, 1)
            + pack('3iIH', 0, 0, 0, 0, 65535))
    record_start = len(prefix) + 4
    shader_end = record_start + 16 + len(body)
    return (prefix + pack('i', 1) + name(12) + pack('q', shader_end) + body
            + pack('iiI', 0, 0, 1) + tail)


def mesh_resource(type_index=16, frequency=0):
    original = resource()
    parsed = parse_prefix(original, NAMES)
    slate = parsed['material_shaders']['records'][0]
    # Reuse the independently checked common material-parameter fixture.
    material = original[slate['body_offset']:slate['output_hash']['offset'] - 12]
    prefix = original[:parsed['material_shaders']['mesh_shader_maps_count_offset']]
    prefix += pack('i', 1) + name(15) + pack('i', 1)
    body_offset = len(prefix) + 16
    vf_head = pack('HI', 1, 1) + name(15) + pack('BB', frequency, 0) + bytes(20)
    vf_body = pack('I3H', 0, 0, 0, 0) if frequency == 0 else b''
    vf_end = body_offset + len(material) + len(vf_head) + 8 + len(vf_body)
    body = material + vf_head + pack('q', vf_end) + vf_body
    if NAMES[type_index].startswith('TBasePass'):
        body += pack('HI', 0, 0) * 4
    elif NAMES[type_index].startswith('TTranslucencyShadowDepthPS'):
        body += pack('3H', 0, 0, 0)
    output_hash = bytes(reversed(range(20)))
    body += (output_hash + bytes(range(20)) + name(13) + name(15) + bytes(20)
             + name(type_index) + pack('i', 0) + bytes(20) + pack('IIi', frequency, 0, 0)
             + name(13) + pack('iII', 0, frequency, 0) + output_hash
             + pack('I4iII', 17, 0, 0, 0, 0, 1095, 1)
             + pack('3iIH', 0, 0, 0, 0, 65535))
    return prefix + name(type_index) + pack('q', body_offset + len(body)) + body + pack('iI', 0, 1)


class OriginalMaxSaturateTests(unittest.TestCase):
    def test_nested_children_and_truncation(self):
        names = NAMES + [PREFIX + 'Max', PREFIX + 'Saturate']
        maximum, saturate = len(NAMES), len(NAMES) + 1
        encoded = name(saturate) + name(maximum) + CONSTANT + SCALAR
        parsed = Reader(encoded, names).expression()
        self.assertEqual(parsed['end'], len(encoded))
        self.assertEqual(parsed['inputs'][0]['end'], len(encoded))
        self.assertEqual(len(parsed['inputs'][0]['inputs']), 2)
        for cut in range(len(encoded)):
            with self.assertRaises(ValueError):
                Reader(encoded[:cut], names).expression()
        nested = name(saturate) * 64 + CONSTANT
        with self.assertRaisesRegex(ValueError, 'complexity'):
            Reader(nested, names).expression()


class RuntimeVirtualTextureUniformTests(unittest.TestCase):
    def test_non_parameter_still_consumes_parameter_info(self):
        names = NAMES + [PREFIX + 'RuntimeVirtualTextureUniform']
        encoded = name(len(NAMES)) + pack('I', 0) + parameter(13) + pack('ii', 164, 2)
        reader = Reader(encoded + CONSTANT, names)
        parsed = reader.expression()
        self.assertFalse(parsed['is_parameter'])
        self.assertEqual(parsed['parameter']['name']['name'], 'None')
        self.assertEqual(parsed['end'], len(encoded))
        self.assertEqual(reader.expression()['end'], len(encoded + CONSTANT))

    def test_parameter_payload_and_truncation(self):
        names = NAMES + [PREFIX + 'RuntimeVirtualTextureUniform']
        encoded = name(len(NAMES)) + pack('I', 1) + parameter(8) + pack('ii', 164, 2)
        parsed = Reader(encoded, names).expression()
        self.assertEqual(parsed, dict(offset=0, end=len(encoded), type=names[-1],
            is_parameter=True, parameter=dict(name=dict(name='Tint', number=0),
            association=2, index=-1), texture_index=164, vector_index=2))
        for cut in range(len(encoded)):
            with self.assertRaises(ValueError):
                Reader(encoded[:cut], names).expression()
        for bad in (2, 0xffffffff):
            with self.assertRaisesRegex(ValueError, 'parameter flag'):
                Reader(encoded[:8] + pack('I', bad) + encoded[12:], names).expression()


class LegacyPrefixTests(unittest.TestCase):
    def test_recursive_values_and_exact_boundary(self):
        data = resource()
        result = parse_prefix(data, NAMES)
        self.assertTrue(result['complete_serialized_shader_map'])
        self.assertFalse(result['runtime_loading_verified'])
        self.assertEqual(result['decoded_prefix_bytes'], len(data))
        self.assertIsNone(result['first_unknown'])
        arrays = result['uniform_expression_arrays']
        self.assertEqual([arrays[x]['count'] for x in ARRAYS], [2, 1, 1, 0, 0, 0, 0, 0])
        self.assertEqual(arrays['vector']['expressions'][0]['default'], [0.125, 0.25, 0.5, 1])
        swizzle = arrays['vector']['expressions'][1]
        self.assertEqual(swizzle['channels'], [0, 1, 2, -1])
        self.assertEqual(swizzle['inputs'][0]['inputs'][0]['inputs'][0]['value_type'], 15)
        self.assertEqual(arrays['texture_2d']['expressions'][0]['texture'],
                         {'texture_index': 0, 'layer_index': -1,
                          'page_table_layer_index': -1,
                          'sampler_source': 0, 'virtual_texture': False})
        self.assertEqual(arrays['texture_2d_array']['end'], result['parameter_collections']['offset'])
        self.assertEqual(result['virtual_texture_stacks']['count'], 0)
        self.assertEqual(result['compilation_output_scalars']['used_scene_textures'], 5)
        self.assertEqual(result['compilation_output_scalars']['estimated_virtual_texture_lookups'], 4)
        self.assertEqual(result['compilation_output_scalars']['used_custom_interpolator_scalars'], 7)
        self.assertEqual(result['compilation_output_scalars']['end'], result['compilation_output_flags']['offset'])
        shaders = result['material_shaders']
        self.assertEqual(shaders['count'], 1)
        self.assertEqual(shaders['records'][0]['frequency'], 0)
        self.assertEqual(shaders['records'][0]['end'], shaders['records'][0]['serialized_end_offset'])
        self.assertEqual(shaders['mesh_shader_maps_count'], 0)
        self.assertEqual(shaders['end'], result['cooking']['offset'])

    def test_exact_eof_and_cooking_marker_are_required(self):
        with self.assertRaisesRegex(ValueError, 'trailing'):
            parse_prefix(resource(tail=b'\0'), NAMES)
        data = bytearray(resource())
        for marker in (0, 2):
            data[-4:] = pack('I', marker)
            with self.assertRaisesRegex(ValueError, 'cooked'):
                parse_prefix(data, NAMES)

    def test_compilation_mask_and_flags_have_distinct_byte_boundaries(self):
        data = bytearray(resource())
        start = parse_prefix(data, NAMES)['compilation_output_flags']['offset']
        data[start:start + 2] = pack('BB', 0x21, 0x42)
        flags = parse_prefix(data, NAMES)['compilation_output_flags']
        self.assertEqual(flags['runtime_virtual_texture_output_attribute_mask'], 0x21)
        self.assertTrue(flags['modifies_mesh_position'])
        self.assertTrue(flags['has_runtime_virtual_texture_output_node'])
        self.assertFalse(flags['uses_eye_adaptation'])
        data[start + 1] = 0x80
        with self.assertRaisesRegex(ValueError, 'flag bit'):
            parse_prefix(data, NAMES)

    def test_mesh_shader_variants_and_nested_end_offsets(self):
        for type_index, frequency in ((16, 0), (17, 3), (18, 0), (19, 3), (20, 0), (21, 4)):
            with self.subTest(type_index=type_index):
                data = mesh_resource(type_index, frequency)
                parsed = parse_prefix(data, NAMES)
                self.assertEqual(parsed['shader_record_count'], 2)
                record = parsed['mesh_shader_maps'][0]['records'][0]
                self.assertEqual(record['frequency'], frequency)
                vf = record['vertex_factory_parameters']
                self.assertEqual(vf['end'], vf['serialized_end_offset'])
                self.assertEqual(record['end'], record['serialized_end_offset'])
                self.assertEqual(parsed['decoded_prefix_bytes'], len(data))

    def test_mesh_vertex_factory_end_cannot_skip_or_overrun_fields(self):
        original = mesh_resource()
        record = parse_prefix(original, NAMES)['mesh_shader_maps'][0]['records'][0]
        vf = record['vertex_factory_parameters']
        end_offset_field = vf['offset'] + 8 + 2 + 20
        for declared in (vf['end'] - 1, vf['end'] + 1, record['end'] + 1):
            data = bytearray(original)
            data[end_offset_field:end_offset_field + 8] = pack('q', declared)
            with self.subTest(declared=declared), self.assertRaises(ValueError):
                parse_prefix(data, NAMES)

    def test_collection_guids_and_nonempty_vt_rejection(self):
        result = parse_prefix(resource(collections=[(1, 2, 3, 0xffffffff)]), NAMES)
        self.assertEqual(result['parameter_collections']['guids'], [[1, 2, 3, 0xffffffff]])
        with self.assertRaisesRegex(ValueError, 'unsupported nonempty VT stacks'):
            parse_prefix(resource(vt_count=1), NAMES)

    def test_invalid_collection_count_is_rejected(self):
        data = bytearray(resource())
        offset = parse_prefix(data, NAMES)['parameter_collections']['offset']
        for count in (-1, 4097):
            data[offset:offset + 4] = pack('i', count)
            with self.assertRaisesRegex(ValueError, 'array count'):
                parse_prefix(data, NAMES)

    def test_texture_record_width_and_validation(self):
        head = name(7) + parameter(9)
        encoded = head + pack('ihhiI', 2, -1, 3, 2, 1) + SCALAR
        reader = Reader(encoded, NAMES)
        texture = reader.expression()
        self.assertEqual(texture['end'], len(head) + 16)
        self.assertEqual(reader.expression()['type'], PREFIX + 'ScalarParameter')
        self.assertTrue(texture['texture']['virtual_texture'])
        self.assertEqual(texture['texture']['page_table_layer_index'], 3)
        for words in [(-2, -1, -1, 0, 0), (0, -2, -1, 0, 0),
                      (0, -1, -2, 0, 0), (0, -1, -1, 3, 0), (0, -1, -1, 0, 2)]:
            with self.subTest(words=words), self.assertRaisesRegex(ValueError, 'texture expression'):
                Reader(head + pack('ihhiI', *words), NAMES).expression()

    def test_texture_property_nested_grammar_and_bounds(self):
        names = NAMES + [PREFIX + 'Texture', PREFIX + 'TextureProperty']
        texture = name(len(NAMES)) + pack('ihhiI', 2, -1, 3, 2, 1)
        for child in (texture, TEXTURE):
            for prop in (0, 1):
                encoded = name(len(NAMES) + 1) + child + pack('B', prop)
                node = Reader(encoded, names).expression()
                self.assertEqual(node['end'], len(encoded))
                self.assertEqual(node['property'], prop)
                self.assertEqual(node['inputs'][0]['end'], len(encoded) - 1)
                for cut in range(len(encoded)):
                    with self.subTest(cut=cut), self.assertRaises(ValueError):
                        Reader(encoded[:cut], names).expression()
                with self.assertRaisesRegex(ValueError, 'texture property'):
                    Reader(encoded[:-1] + pack('B', 2), names).expression()
        for words in [(-2, -1, -1, 0, 0), (0, -2, -1, 0, 0),
                      (0, -1, -2, 0, 0), (0, -1, -1, 3, 0), (0, -1, -1, 0, 2)]:
            with self.subTest(words=words), self.assertRaisesRegex(ValueError, 'texture expression'):
                Reader(name(len(NAMES)) + pack('ihhiI', *words), names).expression()
        with self.assertRaisesRegex(ValueError, 'complexity'):
            Reader(name(len(NAMES) + 1) * 64 + texture + bytes(64), names).expression()

    def test_new_texture_expressions_remain_unsupported_by_numeric_bridge(self):
        evaluator = module._load_sibling('evaluate_uniforms')
        fixture = module._load_sibling('export_runtime_fixture')
        for kind in ('Texture', 'TextureProperty'):
            node = {'type': PREFIX + kind}
            with self.subTest(kind=kind), self.assertRaisesRegex(ValueError, 'unsupported'):
                evaluator.evaluate(node, {}, set())
            with self.subTest(kind=kind), self.assertRaisesRegex(ValueError, 'unsupported'):
                fixture.flatten_expressions({'vector': {'count': 0, 'expressions': []},
                                             'scalar': {'count': 1, 'expressions': [node]}})

    def test_recovered_texture_resources_exact_eof_and_archive(self):
        generated = Path(__file__).resolve().parents[1] / 'generated'
        summary = generated / 'frontend-probes/frontend-material-inventory-textures/summary.json'
        extraction = Path('/mnt/vibrato/fortnite-extracted-full')
        archive_path = extraction / 'FortniteGame/Content/ShaderArchive-FortniteGame-PCD3D_SM5.ushaderbytecode'
        if not summary.is_file() or not archive_path.is_file():
            self.skipTest('local original extraction and overlay are not present')
        data = json.loads(summary.read_text())
        updated = set(data['overlay_provenance']['changed_packages'])
        self.assertEqual(len(updated), 10)
        archive_module = module._load_sibling('shader_archive')
        with archive_module.ShaderArchive(archive_path, layout='fortnite-12.60') as archive:
            for entry in data['entries']:
                if entry['package'] not in updated:
                    continue
                with self.subTest(package=entry['package']):
                    framing = json.loads(Path(entry['report']).with_suffix('.inline-resource.json').read_text())
                    raw = (extraction / (entry['package'] + '.uexp')).read_bytes()
                    self.assertEqual(hashlib.sha256(raw).hexdigest(), framing['uexp_sha256'])
                    blob = raw[framing['resource_start']:framing['resource_end']]
                    self.assertEqual(hashlib.sha256(blob).hexdigest(), framing['resource_sha256'])
                    result = parse_prefix(blob, framing['names'])
                    self.assertEqual(result['undecoded_bytes'], 0)
                    self.assertEqual(result['decoded_prefix_bytes'], len(blob))
                    self.assertEqual(result['shader_record_count'], 19)
                    module.associate_library(result, archive)
                    records = result['material_shaders']['records'] + [record for mesh in result['mesh_shader_maps'] for record in mesh['records']]
                    self.assertEqual(sum('library_association' in record for record in records), 19)

    def test_shader_identities_and_declared_end_are_checked(self):
        original = resource()
        result = parse_prefix(original, NAMES)
        shader = result['material_shaders']['records'][0]
        mutations = [(shader['offset'] + 8, pack('q', shader['end'] - 1)),
                     (shader['offset'] + 8, pack('q', len(original) + 1)),
                     (shader['resource']['output_hash']['offset'], bytes(20)),
                     (shader['material_shader_map_hash']['offset'], bytes(20))]
        for offset, value in mutations:
            data = bytearray(original)
            data[offset:offset + len(value)] = value
            with self.subTest(offset=offset), self.assertRaises(ValueError):
                parse_prefix(data, NAMES)

    def test_record_reads_cannot_cross_declared_end(self):
        original = resource()
        shader = parse_prefix(original, NAMES)['material_shaders']['records'][0]
        for end in range(shader['body_offset'] + 1, shader['end']):
            data = bytearray(original)
            data[shader['offset'] + 8:shader['offset'] + 16] = pack('q', end)
            with self.subTest(end=end), self.assertRaises(ValueError):
                parse_prefix(data, NAMES)

    def test_library_association_checks_hash_frequency_and_size(self):
        result = parse_prefix(resource(), NAMES)
        record = result['material_shaders']['records'][0]
        archive = SimpleNamespace(shader_lookup={bytes.fromhex(record['output_hash']['hash']): 0},
                                  shaders=[(0, 100, 1095, 0)])
        module.associate_library(result, archive)
        self.assertEqual(record['library_association']['index'], 0)
        for records in ([(0, 100, 1096, 0)], [(0, 100, 1095, 3)]):
            archive.shaders = records
            with self.assertRaisesRegex(ValueError, 'target/size'):
                module.associate_library(result, archive)
        archive.shader_lookup = {}
        with self.assertRaisesRegex(ValueError, 'absent'):
            module.associate_library(result, archive)

    def test_one_byte_trig_operation_followed_by_another_expression(self):
        # The constructor takes ETrigMathOperation, but serialized member Op is
        # uint8. A nonzero tag immediately after it catches accidental widening.
        reader = Reader(TRIG + SCALAR, NAMES)
        self.assertEqual(reader.expression()['end'], len(TRIG))
        self.assertEqual(reader.expression()['type'], PREFIX + 'ScalarParameter')

    def test_every_truncation_of_verified_prefix_fails(self):
        data = resource(tail=b'')
        for end in range(len(data)):
            with self.subTest(end=end), self.assertRaises(ValueError):
                parse_prefix(data[:end], NAMES)

    def test_invalid_name_index_number_and_unknown_type(self):
        for tag in (pack('ii', -1, 0), pack('ii', len(NAMES), 0),
                    pack('ii', 0, -1), pack('ii', 0, 1), name(10), name(11)):
            with self.subTest(tag=tag), self.assertRaises(ValueError):
                Reader(tag, NAMES).expression()

    def test_invalid_counts_are_rejected(self):
        for count in (-1, 4097):
            with self.assertRaisesRegex(ValueError, 'array count'):
                parse_prefix(HEADER + pack('i', count), NAMES)

    def test_depth_is_bounded(self):
        # Nested swizzles exceed the bound before reaching their leaf.
        nested = name(3) * 65 + VECTOR + pack('4bB', 0, -1, -1, -1, 1) * 65
        with self.assertRaisesRegex(ValueError, 'complexity'):
            Reader(nested, NAMES).expression()

    def test_header_identity_and_platform_checks(self):
        for offset, replacement in ((0, pack('I', 0)), (36, pack('I', 1))):
            changed = bytearray(resource())
            changed[offset:offset + 4] = replacement
            with self.assertRaises(ValueError):
                parse_prefix(changed, NAMES)
        changed = resource().replace(b'/Game/UI/M_Test.M_Test', b'/Game/UI/M_Test.X_Test')
        with self.assertRaisesRegex(ValueError, 'identity'):
            parse_prefix(changed, NAMES)


if __name__ == '__main__':
    unittest.main()
