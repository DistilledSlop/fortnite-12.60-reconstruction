#!/usr/bin/env python3
"""Reject ambiguous assignments and correlate D3D registers without debug names."""
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('mapped', Path(__file__).with_name('translate_mapped_dxbc.py'))
mapped = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mapped)


def assignment(kind, register, binding):
    return dict(type=kind, register_space=0, register_index=register, count=1, set=0, binding=binding)


def original():
    return {'resource_bindings': {'shader_resource_view': [], 'texture': [],
                                 'sampler': [], 'unordered_access_view': []}}


class RegisterMappingTests(unittest.TestCase):
    def test_explicit_register_mapping_ignores_names_and_order(self):
        source = original()
        source['resource_bindings']['texture'] = [dict(uniform_buffer=2, resource=7, bind=4)]
        source['resource_bindings']['sampler'] = [dict(uniform_buffer=2, resource=8, bind=4)]
        rows, unmatched = mapped.correlate(
            [assignment(0, 4, 19), assignment(3, 4, 6), assignment(2, 1, 30)],
            [dict(set=0, binding=30, storage_class=2, name=None),
             dict(set=0, binding=6, storage_class=0, name='misleading-t99'),
             dict(set=0, binding=19, storage_class=0, name='misleading-s99')], source)
        self.assertFalse(unmatched)
        self.assertEqual(rows[0]['resource_table_entries'][0]['resource'], 7)
        self.assertEqual(rows[1]['resource_table_entries'][0]['resource'], 8)
        self.assertEqual(rows[2]['kind'], 'cbv')

    def test_missing_and_extra_compiled_bindings_fail(self):
        for descriptors in ([], [dict(set=1, binding=0, storage_class=0)]):
            with self.assertRaisesRegex(ValueError, 'locations differ'):
                mapped.correlate([assignment(0, 0, 0)], descriptors, original())

    def test_duplicate_register_location_range_and_wrong_storage_fail(self):
        descriptor = dict(set=0, binding=0, storage_class=0)
        invalid = [[assignment(0, 0, 0), assignment(0, 0, 1)],
                   [assignment(0, 0, 0), assignment(3, 0, 0)],
                   [dict(assignment(0, 0, 0), count=2)]]
        for bindings in invalid:
            with self.assertRaisesRegex(ValueError, 'duplicate|unsupported'):
                mapped.correlate(bindings, [descriptor], original())
        with self.assertRaisesRegex(ValueError, 'storage differs'):
            mapped.correlate([assignment(2, 0, 0)], [descriptor], original())

    def test_unmatched_original_resource_is_preserved(self):
        source = original()
        source['resource_bindings']['texture'] = [dict(uniform_buffer=0, resource=8, bind=3)]
        rows, unmatched = mapped.correlate([], [], source)
        self.assertEqual(rows, [])
        self.assertEqual(unmatched, [dict(uniform_buffer=0, resource=8, bind=3, table='texture')])


if __name__ == '__main__':
    unittest.main()
