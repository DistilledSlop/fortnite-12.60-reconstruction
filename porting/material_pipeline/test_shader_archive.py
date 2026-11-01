#!/usr/bin/env python3
"""Exercise shader archive boundaries, associations and lossless decoding."""
import hashlib
import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest
import zlib

spec = importlib.util.spec_from_file_location('shader_archive', Path(__file__).with_name('shader_archive.py'))
shader = importlib.util.module_from_spec(spec)
spec.loader.exec_module(shader)


def dxbc():
    # One validly bounded code chunk; semantic instruction validation is separate.
    return b'DXBC' + bytes(16) + struct.pack('<IIII', 1, 48, 1, 36) + b'SHDR' + struct.pack('<II', 4, 0x50)


def legacy(code):
    compressed = zlib.compress(code)
    key = hashlib.sha1(b'library output identity').digest()
    return struct.pack('<Ii20sIIB', 2, 1, key, len(compressed), len(code), 1) + compressed, key


def serialized(code, expanded=None, index=0):
    return (struct.pack('<II', 2, 1) + b'm' * 20 + struct.pack('<I', 1) + b's' * 20
            + struct.pack('<IIIII', 1, 0, 1, 0, 1)
            + struct.pack('<IQIIB', 1, 0, len(code), expanded or len(code), 1)
            + struct.pack('<Iqq', 1, 0, len(code))
            + struct.pack('<II', 1, index) + code)


class ArchiveTests(unittest.TestCase):
    def test_legacy_roundtrip_and_material_association(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'library'
            code = b'UE-prefix' + dxbc() + b'optional data'
            data, key = legacy(code)
            path.write_bytes(data)
            material = Path(temp) / 'material.uexp'
            material.write_bytes(b'prefix' + key + bytes(5) + key)
            with shader.ShaderArchive(path) as archive:
                self.assertEqual(archive.shader_code(0), code)
                self.assertEqual(archive.summary()['shader_count'], 1)
                matches = archive.material_matches(material)
                self.assertEqual([m['offset'] for m in matches], [6, 31])
                self.assertTrue(all(m['kind'] == 'shader' and m['index'] == 0 for m in matches))
                containers = shader.dxbc_containers(code)
                self.assertEqual([(c['offset'], c['size']) for c in containers], [(9, 48)])
                self.assertEqual(code[9:57], dxbc())

    def test_serialized_layout_with_shader_map(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'library'
            code = dxbc()
            map_hash, code_hash = b'm' * 20, b's' * 20
            data = serialized(code)
            path.write_bytes(data)
            with shader.ShaderArchive(path, 'ue4-serialized') as archive:
                self.assertEqual(archive.map_shader_indices(0), [0])
                self.assertEqual(archive.shader_code(0), code)
                self.assertEqual(archive.map_lookup[map_hash], 0)
            with self.assertRaises(ValueError):
                shader.ShaderArchive(path, 'fortnite-12.60')

    def test_serialized_compressed_stream_and_invalid_references(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'library'
            block = b'\x32abc\x03\x00\x10X'
            path.write_bytes(serialized(block, 10))
            with shader.ShaderArchive(path, 'ue4-serialized') as archive:
                self.assertEqual(archive.shader_code(0), b'abcabcabcX')
            path.write_bytes(serialized(block + b'\x20YZ', 10))
            with shader.ShaderArchive(path, 'ue4-serialized') as archive:
                with self.assertRaises(ValueError):
                    archive.shader_code(0)
            data = serialized(dxbc())
            for broken in (data[:28], data[:72], data[:93], data[:117], serialized(dxbc(), index=1)):
                path.write_bytes(broken)
                with self.assertRaises(ValueError):
                    shader.ShaderArchive(path, 'ue4-serialized')

    def test_reject_truncated_and_trailing_archive(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'library'
            data, _ = legacy(dxbc())
            for broken in (data[:-1], data + b'unknown trailer', struct.pack('<Ii', 2, -1),
                           struct.pack('<Ii', 3, 0)):
                path.write_bytes(broken)
                with self.assertRaises(ValueError):
                    shader.ShaderArchive(path)

    def test_reject_zlib_stream_size_and_boundaries(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'library'
            data, _ = legacy(dxbc())
            broken = bytearray(data)
            struct.pack_into('<I', broken, 32, len(dxbc()) - 1)
            path.write_bytes(broken)
            with shader.ShaderArchive(path) as archive:
                with self.assertRaises(ValueError):
                    archive.shader_code(0)

    def test_lz4_overlap_and_invalid_offsets(self):
        self.assertEqual(shader.lz4_block(b'\x32abc\x03\x00\x10X', 10), b'abcabcabcX')
        self.assertEqual(shader.lz4_block(b'\x30abc', 3), b'abc')
        for data, size in ((b'\x10A\x00\x00', 5), (b'\x30ab', 3), (b'\xf0', 20),
                           (b'\x30abc', 2), (b'\x30abc', 4)):
            with self.assertRaises(ValueError):
                shader.lz4_block(data, size)

    def test_dxbc_reject_bad_chunk_or_total_boundaries(self):
        data = dxbc()
        self.assertEqual(shader.dxbc_containers(data[:-1]), [])
        bad_offset = bytearray(data)
        struct.pack_into('<I', bad_offset, 32, 12)
        self.assertEqual(shader.dxbc_containers(bytes(bad_offset)), [])
        bad_size = bytearray(data)
        struct.pack_into('<I', bad_size, 40, 1000)
        self.assertEqual(shader.dxbc_containers(bytes(bad_size)), [])


if __name__ == '__main__':
    unittest.main()
