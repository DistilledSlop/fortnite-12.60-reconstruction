#!/usr/bin/env python3
"""Synthetic grammar tests; original-file evidence is produced by the CLI."""
import struct
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


def resource(arrays=None, tail=b'\x78\x56\x34\x12', collections=(), vt_count=0):
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
            + pack('ii', 0, 1) + tail)


class LegacyPrefixTests(unittest.TestCase):
    def test_recursive_values_and_exact_boundary(self):
        data = resource()
        result = parse_prefix(data, NAMES)
        self.assertFalse(result['complete_shader_map'])
        self.assertEqual(result['decoded_prefix_bytes'], len(data) - 4)
        self.assertEqual(result['first_unknown']['next_32_hex'], '78563412')
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
        self.assertEqual(shaders['mesh_shader_maps_count'], 1)
        self.assertEqual(shaders['end'], result['decoded_prefix_bytes'])

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
