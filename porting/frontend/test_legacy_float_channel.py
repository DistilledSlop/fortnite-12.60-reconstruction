#!/usr/bin/env python3
"""Check key preservation and rejection of malformed tagged channel frames."""
import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location('channel', Path(__file__).with_name('inspect_legacy_float_channel.py'))
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
NAMES = ['None', 'Times', 'Values', 'ArrayProperty', 'StructProperty', 'FrameNumber', 'MovieSceneFloatValue']


def name(value):
    return struct.pack('<ii', NAMES.index(value), 0)


def array(label, kind, values, count=None):
    inner = (name(label) + name('StructProperty') + struct.pack('<ii', len(values), 0)
             + name(kind) + bytes(16) + b'\0')
    body = struct.pack('<i', 3 if count is None else count) + inner + values
    return (name(label) + name('ArrayProperty') + struct.pack('<ii', len(body), 0)
            + name('StructProperty') + b'\0' + body)


class TaggedChannels(unittest.TestCase):
    def payload(self, time_count=3):
        times = struct.pack('<' + 'i' * time_count, *range(time_count))
        values = b''.join(struct.pack('<fBBffBff', value, 2, 0, 0, 0, 0, 0, 0) for value in [1, 0, 1])
        return array('Times', 'FrameNumber', times, time_count) + array('Values', 'MovieSceneFloatValue', values) + name('None')

    def parse(self, data):
        return module.channel(data, 0, len(data), NAMES)

    def test_preserves_all_keys_and_23_byte_values(self):
        result = self.parse(self.payload())
        self.assertEqual(result['Times']['values'], [0, 1, 2])
        self.assertEqual([value['value'] for value in result['Values']['values']], [1, 0, 1])
        self.assertEqual(len(bytes.fromhex(result['Values']['raw_hex'])), 69)

    def test_count_mismatch_is_rejected(self):
        with self.assertRaisesRegex(ValueError, 'counts differ'):
            self.parse(self.payload(time_count=2))

    def test_truncated_key_data_is_rejected(self):
        with self.assertRaises(ValueError):
            self.parse(self.payload()[:-12])

    def test_trailing_bytes_are_rejected(self):
        with self.assertRaisesRegex(ValueError, 'after None'):
            self.parse(self.payload() + b'\0')

    def test_negative_and_escaping_counts_are_rejected(self):
        for count in [-1, 2147483647]:
            data = bytearray(self.payload())
            struct.pack_into('<i', data, 33, count)
            with self.assertRaisesRegex(ValueError, 'count/size/framing'):
                self.parse(data)

    def test_changed_value_stride_is_rejected(self):
        values = bytes(72)  # current native memory size 24 differs from original wire size 23
        data = array('Times', 'FrameNumber', bytes(12)) + array('Values', 'MovieSceneFloatValue', values) + name('None')
        with self.assertRaisesRegex(ValueError, 'count/size/framing'):
            self.parse(data)

    def test_numbered_property_name_is_rejected(self):
        data = bytearray(self.payload())
        struct.pack_into('<i', data, 4, 1)
        with self.assertRaisesRegex(ValueError, 'numbered'):
            self.parse(data)

    @unittest.skipUnless(module.PACKAGE.with_suffix('.uexp').exists(), 'optional original package unavailable')
    def test_original_package_exact_channel(self):
        result = module.inspect_package(module.PACKAGE.with_suffix('.uasset').read_bytes(), module.PACKAGE.with_suffix('.uexp').read_bytes())
        self.assertEqual((result['curve_logical_offset'], result['curve_bytes']), (22635, 261))
        self.assertEqual(result['arrays']['Times']['values'], [0, 30000, 60000])
        self.assertEqual([value['value'] for value in result['arrays']['Values']['values']], [1, 0, 1])
        self.assertEqual(result['incorrect_manual_prefix'], dict(pre_extrap=38, post_extrap=1, frame_element_size=0, time_count=1310720))


if __name__ == '__main__':
    unittest.main()
