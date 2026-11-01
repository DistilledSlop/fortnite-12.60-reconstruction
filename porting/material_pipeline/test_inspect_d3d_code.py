#!/usr/bin/env python3
"""Boundaries and binding decoding for the original D3D shader wrapper reader."""
import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location('inspect_d3d_code', Path(__file__).with_name('inspect_d3d_code.py'))
inspector = importlib.util.module_from_spec(spec)
spec.loader.exec_module(inspector)
inspect_wrapper = inspector.inspect_wrapper


def i32(value):
    return struct.pack('<I', value)


def wrapper():
    # One active UB, one texture resource at index 17 bound to slot 2.
    prefix = i32(1)
    for entries in ([1, 0x00001102, 0xffffffff], [0, 0xffffffff],
                    [0, 0xffffffff], [0x12345678], [1, 0x00001102, 0xffffffff]):
        prefix += i32(len(entries)) + b''.join(i32(value) for value in entries)
    dxbc = b'DXBC' + b'\0' * 28
    packed = b'p' + i32(5) + bytes((1, 0, 1, 1, 0))
    names = i32(1) + i32(5) + b'View\0'
    optional = packed + b'u' + i32(len(names)) + names
    return prefix + dxbc + optional + i32(len(optional) + 4), [{'offset': len(prefix), 'size': len(dxbc)}]


class InspectWrapperTests(unittest.TestCase):
    def test_resource_binding_and_optional_names(self):
        code, containers = wrapper()
        row = inspect_wrapper(code, containers)
        self.assertEqual(row['resource_table_bits'], 1)
        self.assertEqual(row['resource_bindings']['texture'],
                         [{'uniform_buffer': 0, 'resource': 17, 'bind': 2}])
        self.assertEqual(row['optional_data'][1]['uniform_buffer_names'], ['View'])
        self.assertEqual(row['resource_table_bytes'], containers[0]['offset'])

    def test_wrong_dxbc_boundary_is_rejected(self):
        code, containers = wrapper()
        containers[0]['offset'] += 1
        with self.assertRaisesRegex(ValueError, 'boundaries'):
            inspect_wrapper(code, containers)

    def test_truncated_optional_value_is_rejected(self):
        code, containers = wrapper()
        damaged = bytearray(code)
        damaged[-4:] = i32(7)
        with self.assertRaises(ValueError):
            inspect_wrapper(damaged, containers)

    def test_inactive_uniform_buffer_binding_is_rejected(self):
        code, containers = wrapper()
        damaged = bytearray(code)
        damaged[:4] = i32(0)
        with self.assertRaisesRegex(ValueError, 'inactive uniform buffer'):
            inspect_wrapper(damaged, containers)


if __name__ == '__main__':
    unittest.main()
