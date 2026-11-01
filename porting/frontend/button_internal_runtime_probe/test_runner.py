import argparse
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from unittest import mock

import runner


class RunnerTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.binary = self.root / 'client'
        if not shutil.which('cc'):
            self.skipTest('C compiler unavailable')
        source = self.root / 'client.c'
        source.write_text('#include <unistd.h>\n#include <stdio.h>\nint main(void) { puts("BTNINTERNAL v=1 seq=34 event=complete ok=1"); usleep(200000); return 0; }\n')
        subprocess.run(['cc', str(source), '-o', str(self.binary)], check=True)
        self.row = {'target': 'FortniteLinux', 'exit': 0, 'source_tree_unchanged': True,
                    'source_head': 'test-head', 'binary': {'path': str(self.binary),
                    'sha256': runner.digest(self.binary), 'size': self.binary.stat().st_size}}
        self.build = self.root / 'build.json'
        self.write_build()

    def tearDown(self):
        self.tmp.cleanup()

    def write_build(self):
        self.build.write_text(json.dumps([self.row]))

    def args(self, case='button'):
        return argparse.Namespace(binary=self.binary, cwd=self.root, out=self.root / 'out',
                                  build_results=self.build, case=case, timeout=3)

    def test_fixed_commands(self):
        self.assertEqual(runner.command(self.binary, 'button'),
                         [str(self.binary), '-ButtonInternalProbe', *runner.BASE_ARGS])
        self.assertEqual(runner.command(self.binary, 'main-button'),
                         [str(self.binary), '-MainButtonStateProbe', *runner.BASE_ARGS])
        self.assertEqual(runner.command(self.binary, 'panel-intro'),
                         [str(self.binary), '-PanelIntroProbe', *runner.BASE_ARGS])
        self.assertEqual(runner.command(self.binary, 'activation'),
                         [str(self.binary), '-ActivationBaseProbe', *runner.BASE_ARGS])
        for case in ('registry', 'lifecycle'):
            self.assertEqual(runner.command(self.binary, case), [str(self.binary),
                '-ContextRuntimeProbe', '-ContextProbeCase=' + case,
                '-ContextProbeTravelMap=/Engine/Maps/Entry', *runner.BASE_ARGS])

    def test_rejects_failed_build_and_hash_mismatch(self):
        self.row['exit'] = 1
        self.write_build()
        with self.assertRaises(ValueError):
            runner.run(self.args())
        self.row['exit'] = 0
        self.row['binary']['sha256'] = '0' * 64
        self.write_build()
        with self.assertRaises(ValueError):
            runner.run(self.args())

    def test_bound_process_and_terminal_row_cannot_pass(self):
        self.assertFalse(runner.run(self.args()))
        receipt = json.loads((self.root / 'out/receipt.json').read_text())
        self.assertTrue(receipt['proc_exe_inode_verified'])
        self.assertEqual(receipt['returncode'], 0)
        self.assertFalse(receipt['transcript']['passed'])
        self.assertFalse(receipt['passed'])
        with self.assertRaises(FileExistsError):
            runner.run(self.args())

    def test_main_button_requires_transcript(self):
        self.assertFalse(runner.run(self.args('main-button')))
        receipt = json.loads((self.root / 'out/receipt.json').read_text())
        self.assertFalse(receipt['transcript']['passed'])
        self.assertEqual(receipt['transcript'], json.loads((self.root / 'out/verification.json').read_text()))

    def test_panel_intro_requires_transcript(self):
        self.assertFalse(runner.run(self.args('panel-intro')))
        receipt = json.loads((self.root / 'out/receipt.json').read_text())
        self.assertFalse(receipt['transcript']['passed'])
        self.assertEqual(receipt['transcript'], json.loads((self.root / 'out/verification.json').read_text()))

    def test_panel_intro_records_successful_verification(self):
        result = {'passed': True, 'rows': 49, 'scope': 'test'}
        with mock.patch.object(runner.panel_intro_verifier, 'verify', return_value=result) as verify:
            self.assertTrue(runner.run(self.args('panel-intro')))
        verify.assert_called_once()
        receipt = json.loads((self.root / 'out/receipt.json').read_text())
        self.assertEqual(receipt['transcript'], result)
        self.assertEqual(json.loads((self.root / 'out/verification.json').read_text()), result)

    def test_empty_cmdline_is_retried_but_wrong_nonempty_is_rejected(self):
        original = Path.read_bytes
        seen = []

        def transient(path):
            if path.name == 'cmdline' and not seen:
                seen.append(True)
                return b''
            return original(path)

        with mock.patch.object(Path, 'read_bytes', transient):
            self.assertTrue(runner.run(self.args('registry')))
        self.assertTrue(seen)
        args = self.args('registry')
        args.out = self.root / 'wrong-argv'

        def wrong(path):
            return b'wrong\0' if path.name == 'cmdline' else original(path)

        with mock.patch.object(Path, 'read_bytes', wrong):
            self.assertFalse(runner.run(args))
        receipt = json.loads((args.out / 'receipt.json').read_text())
        self.assertEqual(receipt['proc_argv'], ['wrong'])
        self.assertTrue(receipt['proc_exe_inode_verified'])

    def test_non_button_is_process_binding_only(self):
        self.assertTrue(runner.run(self.args('registry')))
        receipt = json.loads((self.root / 'out/receipt.json').read_text())
        self.assertNotIn('transcript', receipt)


if __name__ == '__main__':
    unittest.main()
