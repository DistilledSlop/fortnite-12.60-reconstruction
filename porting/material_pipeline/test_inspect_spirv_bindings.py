#!/usr/bin/env python3
"""Validate bounded SPIR-V descriptor extraction and collision checks."""
import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location('inspect_spirv_bindings',
                                             Path(__file__).with_name('inspect_spirv_bindings.py'))
inspector = importlib.util.module_from_spec(spec)
spec.loader.exec_module(inspector)


def op(code, *operands):
    return ((len(operands) + 1) << 16 | code, *operands)


def shader():
    words = [inspector.SPIRV_MAGIC, 0x10000, 0, 8, 0]
    words += op(5, 2, int.from_bytes(b't0\0\0', 'little'))
    words += op(71, 2, 34, 1)  # DescriptorSet
    words += op(71, 2, 33, 3)  # Binding
    words += op(59, 1, 2, 0)   # UniformConstant variable
    return struct.pack('<' + 'I' * len(words), *words)


class SpirvBindingTests(unittest.TestCase):
    def test_named_descriptor(self):
        self.assertEqual(inspector.spirv_descriptors(shader()),
                         [{'name': 't0', 'set': 1, 'binding': 3, 'storage_class': 0}])

    def test_truncated_instruction(self):
        with self.assertRaisesRegex(ValueError, 'instruction length'):
            inspector.spirv_descriptors(shader()[:-4])

    def test_remapping_offsets_point_to_decoration_operands(self):
        data = shader()
        words = list(struct.unpack('<' + 'I' * (len(data) // 4), data))
        descriptor, = inspector.spirv_descriptors(data, include_offsets=True)
        for field, value in [('set', 9), ('binding', 27)]:
            offset = descriptor[field + '_word_offset']
            self.assertEqual(words[offset], descriptor[field])
            self.assertEqual(words[offset - 1], 34 if field == 'set' else 33)
            words[offset] = value
        changed = struct.pack('<' + 'I' * len(words), *words)
        remapped, = inspector.spirv_descriptors(changed)
        self.assertEqual((remapped['set'], remapped['binding']), (9, 27))

    def test_missing_variable(self):
        data = shader()
        with self.assertRaisesRegex(ValueError, 'incomplete'):
            inspector.spirv_descriptors(data[:-16])


if __name__ == '__main__':
    unittest.main()
