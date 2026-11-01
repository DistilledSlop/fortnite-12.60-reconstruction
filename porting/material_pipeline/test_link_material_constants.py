#!/usr/bin/env python3
"""Validate binding slots and spans against independent serialized inputs."""
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('linker', Path(__file__).with_name('link_material_constants.py'))
linker = importlib.util.module_from_spec(spec)
spec.loader.exec_module(linker)


def reflection(slot=2, size=80, kind=6):
    return dict(descriptors=[dict(d3d_register=dict(kind='cbv', space=0, index=slot),
        vulkan_descriptor_type=kind, block_span_bytes=size, set=0, binding=3,
        set_word_offset=19, binding_word_offset=23)])


class MaterialConstantLinkTests(unittest.TestCase):
    def test_uses_serialized_material_slot_and_accepts_shorter_declaration(self):
        record = dict(material_uniform_buffer=dict(base_index=2, bound=True))
        result = linker.material_binding(record, reflection(), 96)
        self.assertEqual(result['d3d_constant_buffer'], 2)
        self.assertEqual(result['descriptor_binding'], 3)
        self.assertEqual(result['declared_block_span_bytes'], 80)
        self.assertEqual(result['evaluated_numeric_bytes'], 96)

    def test_unbound_material_has_no_numeric_binding(self):
        record = dict(material_uniform_buffer=dict(base_index=0, bound=False))
        self.assertIsNone(linker.material_binding(record, reflection(), 96))

    def test_wrong_slot_kind_and_excess_span_fail(self):
        record = dict(material_uniform_buffer=dict(base_index=2, bound=True))
        for reflected in (reflection(slot=1), reflection(kind=7), reflection(size=112)):
            with self.assertRaises(ValueError):
                linker.material_binding(record, reflected, 96)


if __name__ == '__main__':
    unittest.main()
