#!/usr/bin/env python3
"""Fail-closed boundaries for the Paper2D metadata reader."""
import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location('paper2d_shader_prefix', Path(__file__).with_name('paper2d_shader_prefix.py'))
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class Boundaries(unittest.TestCase):
    def test_unknown_record_does_not_consume_or_seek(self):
        # An attacker-controlled end offset must not let unknown types skip bytes.
        data = struct.pack('<iiiiiq', 0, 0, 1, 1, 0, 0x7fffffffffffffff)
        reader = module.Reader(data, ['FLocalVertexFactory', 'UnknownPS'])
        result = module.paper2d_mesh_maps(reader, 1, '00' * 20, [0] * 7)
        self.assertEqual(reader.cursor, 12)
        self.assertEqual(result[0]['first_unknown']['offset'], 12)
        self.assertEqual(result[0]['records'], [])

    def test_known_record_requires_bounded_body(self):
        data = struct.pack('<iiiiiq', 0, 0, 1, 1, 0, 99999)
        reader = module.Reader(data, ['FLocalVertexFactory', 'TBasePassVSFNoLightMapPolicy'])
        with self.assertRaisesRegex(ValueError, 'invalid shader end offset'):
            module.paper2d_mesh_maps(reader, 1, '00' * 20, [0] * 7)

    def test_wrong_vertex_factory_rejected_before_records(self):
        reader = module.Reader(struct.pack('<ii', 0, 0), ['OtherVertexFactory'])
        with self.assertRaisesRegex(ValueError, 'unsupported mesh vertex factory'):
            module.paper2d_mesh_maps(reader, 1, '00' * 20, [0] * 7)


if __name__ == '__main__':
    unittest.main()
