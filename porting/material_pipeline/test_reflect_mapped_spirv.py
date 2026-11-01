#!/usr/bin/env python3
"""Check descriptor kinds and layout spans from declarations, not debug names."""
import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location('reflection', Path(__file__).with_name('reflect_mapped_spirv.py'))
reflection = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reflection)


def op(code, *operands):
    return [(len(operands) + 1) << 16 | code, *operands]


def string(value):
    data = value.encode() + b'\0'
    data += b'\0' * (-len(data) % 4)
    return list(struct.unpack('<' + 'I' * (len(data) // 4), data))


def module(kind='ubo', sampled=1, stride=16, include_offset=True):
    storage = 2 if kind == 'ubo' else 0
    words = [0x07230203, 0x10000, 0, 64, 0]
    words += op(15, 0, 40, *string('main'), 11)
    words += op(21, 1, 32, 0) + op(22, 2, 32) + op(23, 3, 2, 4)
    if kind == 'ubo':
        words += op(43, 1, 8, 2) + op(28, 4, 3, 8) + op(30, 5, 4)
        words += op(71, 4, 6, stride) + op(71, 5, 2)
        if include_offset:
            words += op(72, 5, 0, 35, 32)
    elif kind == 'image':
        words += op(25, 5, 2, 5, 0, 0, 0, sampled, 0)
    elif kind == 'sampler':
        words += op(26, 5)
    words += op(32, 6, storage, 5) + op(59, 6, 10, storage)
    words += op(71, 10, 34, 0) + op(71, 10, 33, 3)
    words += op(32, 7, 1, 3) + op(59, 7, 11, 1) + op(71, 11, 30, 2)
    return struct.pack('<' + 'I' * len(words), *words)


class SpirvReflectionTests(unittest.TestCase):
    def test_block_span_includes_declared_member_offset_and_array_stride(self):
        row = reflection.Module(module()).reflect()
        descriptor, = row['descriptors']
        self.assertEqual(descriptor['vulkan_descriptor_type'], 6)
        self.assertEqual(descriptor['block_span_bytes'], 64)
        interface, = row['interfaces']
        self.assertEqual((interface['direction'], interface['location']), ('input', 2))
        self.assertEqual(interface['type'], dict(kind='vector', count=4, element=dict(kind='float', bits=32)))

    def test_buffer_image_and_sampler_have_different_vulkan_types(self):
        for kind, sampled, expected in [('image', 1, 4), ('image', 2, 5), ('sampler', 1, 0)]:
            descriptor, = reflection.Module(module(kind, sampled)).reflect()['descriptors']
            self.assertEqual(descriptor['vulkan_descriptor_type'], expected)

    def test_unknown_image_usage_fails(self):
        with self.assertRaisesRegex(ValueError, 'usage is unknown'):
            reflection.Module(module('image', 0)).reflect()

    def test_layout_must_be_explicit_and_array_stride_sufficient(self):
        for data in [module(include_offset=False), module(stride=8)]:
            with self.assertRaises(ValueError):
                reflection.Module(data).reflect()


if __name__ == '__main__':
    unittest.main()
