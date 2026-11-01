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
