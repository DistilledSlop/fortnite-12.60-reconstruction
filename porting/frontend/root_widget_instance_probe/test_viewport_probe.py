import argparse
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

import runner
import verifier
import viewport_verifier
from test_probe import V2_ROWS

ROWS = [
    'ROOTVIEW v=1 phase=before_attach in_viewport=0 cached_widget=0',
    'ROOTVIEW v=1 phase=after_attach in_viewport=1 cached_widget=1',
    'ROOTVIEW v=1 phase=frame index=1 engine_frame=100 in_viewport=1 cached_widget=1',
    'ROOTVIEW v=1 phase=frame index=2 engine_frame=101 in_viewport=1 cached_widget=1',
    'ROOTVIEW v=1 phase=frame index=3 engine_frame=102 in_viewport=1 cached_widget=1',
    'ROOTVIEW v=1 phase=before_remove in_viewport=1 cached_widget=1',
    'ROOTVIEW v=1 phase=after_remove in_viewport=0 cached_widget=1',
    'ROOTVIEW v=1 phase=complete ok=1',
]


class ViewportTests(unittest.TestCase):
    def test_valid_and_optional_cache(self):
        for first, last in ((0, 0), (0, 1), (1, 0), (1, 1)):
            rows = ROWS.copy()
            rows[0] = rows[0][:-1] + str(first)
            rows[6] = rows[6][:-1] + str(last)
            result = viewport_verifier.verify('\n'.join(rows))
            self.assertTrue(result['passed'])
            self.assertTrue(result['attachment_observed'])
            self.assertEqual(result['engine_frames'], [100, 101, 102])

    def test_reject_bad_lifecycle(self):
        cases = [ROWS[:-1], ROWS + [ROWS[-1]], ROWS[1:] + ROWS[:1],
                 ROWS[:3] + [ROWS[2]] + ROWS[4:],
                 ROWS[:2] + [ROWS[2].replace('index=1', 'index=2')] + ROWS[3:],
                 ROWS[:3] + [ROWS[3].replace('engine_frame=101', 'engine_frame=100')] + ROWS[4:],
                 ROWS[:1] + [ROWS[1].replace('in_viewport=1', 'in_viewport=0')] + ROWS[2:],
                 ROWS[:5] + [ROWS[5].replace('cached_widget=1', 'cached_widget=0')] + ROWS[6:],
                 ROWS[:6] + [ROWS[6].replace('in_viewport=0', 'in_viewport=1')] + ROWS[7:],
                 ROWS[:-1] + [ROWS[-1].replace('ok=1', 'ok=0')],
                 ROWS[:2] + [ROWS[2].replace('index=1 engine_frame=100', 'engine_frame=100 index=1')] + ROWS[3:],
                 ROWS[:3] + ['ROOTVIEW v=1 phase=timeout ok=0', 'ROOTVIEW v=1 phase=complete ok=0']]
        for rows in cases:
            with self.subTest(rows=rows):
                self.assertFalse(viewport_verifier.verify('\n'.join(rows))['passed'])
        self.assertEqual(viewport_verifier.verify('\n'.join(ROWS[:4]))['last_phase'], 'frame')
        self.assertEqual(viewport_verifier.verify('ROOTVIEW v=1 phase=timeout ok=0')['last_phase'], 'timeout')

    def test_partial_attachment_evidence(self):
        result = viewport_verifier.verify('\n'.join(ROWS[:4]))
        self.assertFalse(result['passed'])
        self.assertTrue(result['attachment_observed'])
        self.assertEqual(result['last_phase'], 'frame')
        for rows in ([ROWS[1], ROWS[0]],
                     [ROWS[0] + ' extra=1', ROWS[1]],
                     [ROWS[0], ROWS[1].replace('cached_widget=1', 'cached_widget=0')]):
            self.assertFalse(viewport_verifier.verify('\n'.join(rows))['attachment_observed'])

    def test_command_compatibility(self):
        binary = Path('/tmp/test')
        old = runner.command(binary, ['-example'])
        self.assertEqual(old.count('-RootWidgetInstanceProbe'), 1)
        self.assertNotIn('-RootWidgetInstanceAttach', old)
        self.assertEqual(runner.command(binary, ['-example'], True),
                         old[:-1] + ['-RootWidgetInstanceAttach', '-example'])

    def test_receipt_separates_dependency_failure(self):
        with tempfile.TemporaryDirectory() as tmp:
            d = Path(tmp)
            lines = V2_ROWS[:-1] + ROWS + V2_ROWS[-1:] + [
                'Skipping import Function X, depends on missing native class']
            source = d / 'child.c'
            source.write_text('#include <stdio.h>\n#include <unistd.h>\nint main(void) {\n' +
                              ''.join('puts(%s);\n' % json.dumps(line) for line in lines) +
                              'fflush(stdout); usleep(100000); return 0; }\n')
            binary = d / 'child'
            subprocess.run(['cc', str(source), '-o', str(binary)], check=True)
            build = d / 'build.json'
            build.write_text(json.dumps([{'target': 'FortniteLinux', 'exit': 0,
                'source_tree_unchanged': True, 'source_head': 'test',
                'binary': {'path': str(binary), 'size': binary.stat().st_size,
                           'sha256': runner.digest(binary)}}]))
            flags = d / 'args.json'
            flags.write_text(json.dumps(['-FortniteLegacyMaterialIngress=a',
                '-FortniteLegacyMaterialManifest=b', '-FortniteLegacyMaterialManifestSHA256=c']))
            args = argparse.Namespace(binary=binary, cwd=d, timeout=2, build_results=build,
                                      out=d / 'out', runtime_args=flags, attach=True)
            self.assertFalse(runner.run(args))
            receipt = json.loads((args.out / 'receipt.json').read_text())
            self.assertTrue(receipt['viewport']['passed'])
            self.assertTrue(receipt['viewport']['attachment_observed'])
            self.assertFalse(receipt['transcript']['passed'])
            self.assertEqual(receipt['transcript']['dependency_error_count'], 1)
            self.assertEqual(receipt['argv'].count('-RootWidgetInstanceAttach'), 1)


if __name__ == '__main__':
    unittest.main()
