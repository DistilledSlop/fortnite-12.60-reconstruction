import tempfile
import unittest
from pathlib import Path

import stage_font_faces


class StageFontFacesTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.source = Path(self.temp.name) / 'source'
        self.target = Path(self.temp.name) / 'stage'
        for root in (self.source, self.target):
            (root / 'Fonts').mkdir(parents=True)
            for ext in ('.uasset', '.uexp'):
                (root / 'Fonts' / ('Face' + ext)).write_bytes(ext.encode())
        (self.source / 'Fonts/Face.ufont').write_bytes(b'original')

    def run_stage(self, check=False):
        return stage_font_faces.stage(self.source, self.target, Path('Fonts'), check)

    def test_copy_and_idempotency(self):
        self.assertEqual(self.run_stage()['files'][0]['status'], 'copied')
        self.assertEqual((self.target / 'Fonts/Face.ufont').read_bytes(), b'original')
        self.assertEqual(self.run_stage()['files'][0]['status'], 'identical')
        self.assertTrue(self.run_stage(check=True)['ok'])

    def test_missing_or_mismatched_package_prevents_copy(self):
        package = self.target / 'Fonts/Face.uexp'
        package.unlink()
        self.assertFalse(self.run_stage()['ok'])
        self.assertFalse((self.target / 'Fonts/Face.ufont').exists())
        package.write_bytes(b'different')
        self.assertFalse(self.run_stage()['ok'])
        self.assertFalse((self.target / 'Fonts/Face.ufont').exists())

    def test_conflicting_payload_is_preserved(self):
        payload = self.target / 'Fonts/Face.ufont'
        payload.write_bytes(b'conflict')
        self.assertFalse(self.run_stage()['ok'])
        self.assertEqual(payload.read_bytes(), b'conflict')

    def test_check_missing_does_not_copy(self):
        result = self.run_stage(check=True)
        self.assertFalse(result['ok'])
        self.assertIn('missing staged payload', result['files'][0]['errors'][0])
        self.assertFalse((self.target / 'Fonts/Face.ufont').exists())


if __name__ == '__main__':
    unittest.main()
