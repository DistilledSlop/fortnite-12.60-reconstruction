#!/usr/bin/env python3
"""Guard exact shader instruction comparison and native payload boundaries."""
import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location('verifier', Path(__file__).with_name('verify_runtime_artifacts.py'))
verifier = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verifier)


def module(name, literal=17):
    raw = name.encode() + b'\0'
    raw += b'\0' * (-len(raw) % 4)
    text = list(struct.unpack('<' + 'I' * (len(raw) // 4), raw))
    words = [0x07230203, 0x10000, 0, 64, 0]
    words += [(len(text) + 4) << 16 | 15, 0, 2, *text, 11]
    words += [4 << 16 | 43, 1, 12, literal]
    return struct.pack('<' + 'I' * len(words), *words)


class RuntimeArtifactVerificationTests(unittest.TestCase):
    def test_rhi_hash_preserves_reverse_type_packing_and_layout_bounds(self):
        self.assertEqual(verifier.uniform_layout_hash(336, [320, 328], [8, 8]), 22544639)
        self.assertEqual(verifier.uniform_layout_hash(208, [192, 200], [8, 8]), 14156031)
        self.assertEqual(verifier.uniform_layout_hash(32, [16, 24], [5, 8]), 0x002500ff)
        self.assertEqual(verifier.uniform_layout_hash(48, [16, 24, 32, 40], [5, 8, 6, 7]), 0x053806f8)
        for offsets, types in [([17], [8]), ([32], [8]), ([16, 24], [8])]:
            with self.assertRaisesRegex(ValueError, 'layout shape'):
                verifier.uniform_layout_hash(32, offsets, types)

    def test_original_typed_registers_distinguish_view_and_material(self):
        record = dict(material_uniform_buffer=dict(base_index=2, bound=True), uniform_parameters=[
            dict(struct_name=dict(name='FViewUniformShaderParameters'), binding=dict(base_index=1, bound=True))])
        shader = dict(reflection=dict(descriptors=[
            dict(binding=9, d3d_register=dict(kind='cbv', space=0, index=2)),
            dict(binding=4, d3d_register=dict(kind='cbv', space=0, index=1))]))
        self.assertEqual(verifier.expected_uniform_bindings(record, shader, 111, 222),
                         {9: ('Material', 111), 4: ('View', 222)})
        with self.assertRaisesRegex(ValueError, 'actual current metadata'):
            verifier.expected_uniform_bindings(record, shader, 111, None)

    def test_name_only_adaptation_preserves_interfaces_and_other_instructions(self):
        original = verifier.normalize_entrypoint(module('main'))
        changed_name = verifier.normalize_entrypoint(module('main_00000000_12345678'))
        self.assertEqual(original[:2], changed_name[:2])
        self.assertNotEqual(original[2], changed_name[2])
        changed_literal = verifier.normalize_entrypoint(module('main_00000000_12345678', 18))
        self.assertNotEqual(original[:2], changed_literal[:2])

    def test_native_payload_requires_exact_spirv_and_optional_trailer_bounds(self):
        words = [0x07230203, 0x10000, 0, 64, 0]
        # Eight empty metadata arrays, flags/source/CRC, two offset arrays,
        # empty FString, then the SPIR-V TArray and FShaderCode trailer.
        header = bytes(8 * 4) + struct.pack('<II20sI', 3, 1, bytes(20), 0)
        header += bytes(3 * 4)
        payload = header + struct.pack('<i5II', 5, *words, 4)
        info, code = verifier.read_payload(payload)
        self.assertEqual(info['inout_mask'], 3)
        self.assertEqual(code, struct.pack('<5I', *words))
        for bad in (payload[:-1], payload + b'\0', payload[:-4] + struct.pack('<I', 8)):
            with self.assertRaises(ValueError):
                verifier.read_payload(bad)

    def test_instruction_cannot_overrun_module(self):
        with self.assertRaisesRegex(ValueError, 'instruction'):
            verifier.normalize_entrypoint(module('main')[:-4])


if __name__ == '__main__':
    unittest.main()
