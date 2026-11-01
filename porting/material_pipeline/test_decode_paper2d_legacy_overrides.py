import hashlib
import struct
import unittest
from decode_paper2d_legacy_overrides import decode


def fixture():
    # Three empty static arrays, no shaders; then v379 native bools, float and byte enums.
    data = struct.pack('<IIIIIIIfIBIBII', 0, 0, 0, 0, 1, 1, 0, 0.3333, 1, 2, 0, 1, 0, 1)
    e = {'sourceSha256': hashlib.sha256(data).hexdigest(), 'ue4': 379, 'customVersions': 0,
         'cooked': False, 'filterEditorOnly': False, 'classDefaultObject': False, 'staticPermutation': True,
         'exportOffset': 0, 'exportSize': len(data), 'nativeStart': 0,
         'constants': {'purged': 260, 'fixedSerialization': 382, 'phase2': 314, 'shadowDepth': 378, 'dither': 453}}
    return data, e


class NativeOverrideTests(unittest.TestCase):
    def test_translucent_and_byte_alignment(self):
        data, e = fixture()
        report = decode(data, e)
        self.assertEqual(report['effectiveBlend'], 'BLEND_Translucent')
        self.assertEqual(report['fields']['blendMode']['offset'], 36)
        self.assertEqual(report['fields']['overrideShadingModel']['value'], 0)
        self.assertEqual(report['exportEnd'], 50)

    def test_unsupported_and_corrupt_fail_closed(self):
        data, e = fixture()
        with self.assertRaisesRegex(ValueError, 'hash'):
            decode(data + b'\0', e)
        e['cooked'] = True
        with self.assertRaisesRegex(ValueError, 'flags'):
            decode(data, e)
        e['cooked'] = False
        e['exportSize'] -= 1
        with self.assertRaisesRegex(ValueError, 'truncated'):
            decode(data, e)
        e['exportSize'] += 1
        corrupt = bytearray(data); corrupt[0] = 1
        e['sourceSha256'] = hashlib.sha256(corrupt).hexdigest()
        with self.assertRaisesRegex(ValueError, 'nonempty'):
            decode(corrupt, e)


if __name__ == '__main__':
    unittest.main()
