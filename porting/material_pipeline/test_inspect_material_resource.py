#!/usr/bin/env python3
"""Verify exact material wrapper framing and fail closed on false matches."""
import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location('inspect_material_resource',
                                             Path(__file__).with_name('inspect_material_resource.py'))
inspector = importlib.util.module_from_spec(spec)
spec.loader.exec_module(inspector)


def i32(value):
    return struct.pack('<i', value)


def fixture():
    name = i32(5) + b'Test\0' + b'\0' * 4
    payload = i32(1) + i32(1) + b'original bytes'
    wrapper = i32(1) + name + i32(1) + struct.pack('<IBB', 0, 3, 1) + i32(len(payload)) + payload
    return b'P' * 7 + wrapper, 7


class MaterialResourceTests(unittest.TestCase):
    def test_observed_legacy_header_identity_and_local_misalignment(self):
        name = 'M_Tests'
        path = '/Game/Test/M_Tests.M_Tests'
        body = b''.join((i32(1), i32(1), i32(1), i32(3), b'h' * 20,
                         i32(0), i32(len(name) + 1), name.encode() + b'\0',
                         i32(len(path) + 1), path.encode() + b'\0', i32(7)))
        row = inspector.inspect_legacy_header(body, 0, len(body), name)
        self.assertEqual(row['identity_path'], path)
        self.assertEqual(row['next_u32'], 7)
        self.assertEqual(row['local_layout_interpretation']['flags'], len(name) + 1)
        self.assertFalse(row['local_layout_interpretation']['initialized_flag_set'])
        inherited = inspector.inspect_legacy_header(body, 0, len(body), 'M_Instance')
        self.assertFalse(inherited['identity_matches_export'])

    def test_exact_export_boundaries(self):
        raw, start = fixture()
        row = inspector.scan_resource(raw, start, len(raw))
        self.assertEqual(row['wrapper_start'], start)
        self.assertEqual(row['names'], ['Test'])
        self.assertEqual(row['locations'], [{'offset': 0, 'feature_level': 3, 'quality_level': 1}])
        self.assertEqual(row['resource_end'], len(raw))

    def test_bad_resource_size_is_not_accepted(self):
        raw, start = fixture()
        damaged = bytearray(raw)
        damaged[34:38] = i32(100000)
        with self.assertRaisesRegex(ValueError, 'found 0'):
            inspector.scan_resource(damaged, start, len(damaged))

    def test_export_must_fit_file(self):
        raw, start = fixture()
        with self.assertRaisesRegex(ValueError, 'exceeds'):
            inspector.scan_resource(raw, start, len(raw) + 1)


if __name__ == '__main__':
    unittest.main()
