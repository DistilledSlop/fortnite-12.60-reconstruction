#!/usr/bin/env python3
"""Focused checks for opt-in comparison gating of runtime image captures."""
import contextlib
import io
import json
import os
import struct
import tempfile
import unittest
from unittest import mock

import dump_runtime_image as dumper


def fake_image():
    img = bytearray(4096)
    img[:2] = b'MZ'
    struct.pack_into('<I', img, 0x3c, 0x80)
    img[0x80:0x84] = b'PE\0\0'
    struct.pack_into('<H', img, 0x80 + 6, 1)
    struct.pack_into('<H', img, 0x80 + 20, 0xf0)
    struct.pack_into('<H', img, 0x80 + 24, 0x20b)
    struct.pack_into('<I', img, 0x80 + 24 + 56, len(img))
    return bytes(img)


class ComparisonTest(unittest.TestCase):
    def test_ranges_and_bounds(self):
        baseline = bytes(64)
        changed = bytearray(baseline)
        changed[9] = 1
        checks = dumper.changed_ranges(changed, baseline,
                                       [dumper.IMAGE_BASE + 8, dumper.IMAGE_BASE + 20], 4)
        self.assertEqual([c['changed_bytes'] for c in checks], [1, 0])
        with self.assertRaises(ValueError):
            dumper.changed_ranges(changed, baseline, [dumper.IMAGE_BASE + 63], 4)
        with self.assertRaises(ValueError):
            dumper.changed_ranges(changed[:-1], baseline, [], 4)

    def test_capture_retries_until_all_ranges_change(self):
        baseline = fake_image()
        partial = bytearray(baseline)
        partial[0x400] = 1
        complete = bytearray(partial)
        complete[0x420] = 2
        images = iter([baseline, bytes(partial), baseline, bytes(complete)])
        report = {'verdict': 'decrypted', 'code_median_entropy': 6.0,
                  'sections': [], 'decode': {}}
        with tempfile.TemporaryDirectory() as directory:
            baseline_path = os.path.join(directory, 'earlier.bin')
            with open(baseline_path, 'wb') as stream:
                stream.write(baseline)
            argv = ['dump_runtime_image.py', '--out', directory, '--timeout', '5',
                    '--compare-dump', baseline_path, '--changed-va', hex(dumper.IMAGE_BASE + 0x400),
                    '--changed-va', hex(dumper.IMAGE_BASE + 0x420)]
            exists = os.path.exists
            with mock.patch.object(dumper.sys, 'argv', argv), \
                 mock.patch.object(dumper, 'find_pid', return_value=123), \
                 mock.patch.object(dumper.os.path, 'exists', side_effect=lambda p: True if p == '/proc/123' else exists(p)), \
                 mock.patch.object(dumper.os, 'open', return_value=999), \
                 mock.patch.object(dumper.os, 'close'), \
                 mock.patch.object(dumper, 'read_at', side_effect=lambda *args: (next(images), 0)), \
                 mock.patch.object(dumper, 'process_age', return_value=1), \
                 mock.patch.object(dumper, 'quick_sample', return_value=6), \
                 mock.patch.object(dumper.tempfile, 'TemporaryDirectory', return_value=contextlib.nullcontext(directory)), \
                 mock.patch.object(dumper, 'verify', return_value=(True, report)), \
                 mock.patch.object(dumper, 'compare_with_disk', return_value=None), \
                 mock.patch.object(dumper.time, 'sleep'), contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(dumper.main(), 0)
            with open(os.path.join(directory, 'dump.json')) as stream:
                meta = json.load(stream)
            self.assertEqual(meta['full_read_attempts'], 2)
            self.assertTrue(meta['address_comparison']['all_changed'])
            self.assertEqual([c['changed_bytes'] for c in meta['address_comparison']['checks']], [1, 1])
            with open(os.path.join(directory, dumper.DUMP_NAME), 'rb') as stream:
                self.assertEqual(stream.read(), complete)


if __name__ == '__main__':
    unittest.main()
