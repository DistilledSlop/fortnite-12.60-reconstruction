import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

import current_input_type_verifier as verifier
import runner


def rows():
    result = ['INPUTTYPE v=1 seq=1 event=begin ok=1',
              'INPUTTYPE v=1 seq=2 event=context ok=1',
              'INPUTTYPE v=1 seq=3 event=metadata ok=1']
    for i, value in enumerate((0, 2, 1, 0, 3, 255, 1, 2)):
        result.append(f'INPUTTYPE v=1 seq={i + 4} event=state index={i} value={value} '
                      f'direct={value} reflected={value} ok=1')
    return result + ['INPUTTYPE v=1 seq=12 event=restored ok=1',
                     'INPUTTYPE v=1 seq=13 event=complete ok=1']


class CurrentInputTypeTests(unittest.TestCase):
    def test_valid_with_engine_lines_and_prefixes(self):
        transcript = 'engine startup\n' + '\n'.join('[LogTemp] ' + row for row in rows())
        self.assertEqual(verifier.verify(transcript)['rows'], 13)

    def test_missing_modified_failed_duplicate_and_reordered_rows(self):
        original = rows()
        for i, row in enumerate(original):
            cases = [original[:i] + original[i + 1:],
                     original[:i] + [row + ' extra'] + original[i + 1:],
                     original[:i] + [row.replace('ok=1', 'ok=0')] + original[i + 1:],
                     original[:i] + [row, row] + original[i + 1:]]
            for changed in cases:
                with self.subTest(i=i, changed=changed[i:i + 2]), self.assertRaises(ValueError):
                    verifier.verify('\n'.join(changed))
        for changed in (original[:3] + [original[4], original[3]] + original[5:],
                        original[:4] + [original[-1]] + original[4:],
                        original + ['bad INPUTTYPE text'],
                        [original[0] + ' INPUTTYPE'] + original[1:],
                        [x.replace('direct=255', 'direct=1') for x in original],
                        [x.replace('index=1', 'index=01') for x in original]):
            with self.subTest(changed=changed), self.assertRaises(ValueError):
                verifier.verify('\n'.join(changed))

    def test_runner_cli_dispatch_and_receipt(self):
        if not shutil.which('cc'):
            self.skipTest('C compiler unavailable')
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            binary = root / 'client'
            source = root / 'client.c'
            lines = '\\n'.join(rows())
            source.write_text('#include <stdio.h>\n#include <unistd.h>\n'
                              f'int main(void) {{ puts("{lines}"); usleep(200000); return 0; }}\n')
            subprocess.run(['cc', str(source), '-o', str(binary)], check=True)
            build = root / 'build.json'
            build.write_text(json.dumps([{'target': 'FortniteLinux', 'exit': 0,
                'source_tree_unchanged': True, 'source_head': 'test-head',
                'binary': {'path': str(binary), 'sha256': runner.digest(binary),
                           'size': binary.stat().st_size}}]))
            self.assertEqual(runner.command(binary, 'common-input-current-type'),
                             [str(binary), '-CommonInputCurrentTypeProbe', *runner.BASE_ARGS])
            out = root / 'out'
            proc = subprocess.run([sys.executable, str(Path(runner.__file__)),
                '--build-results', str(build), '--binary', str(binary), '--cwd', str(root),
                '--out', str(out), '--case', 'common-input-current-type', '--timeout', '3'],
                capture_output=True, text=True)
            self.assertEqual(proc.returncode, 0, proc.stderr)
            receipt = json.loads((out / 'receipt.json').read_text())
            self.assertTrue(receipt['passed'])
            self.assertEqual(receipt['transcript']['rows'], 13)
            self.assertEqual(receipt['transcript'], json.loads((out / 'verification.json').read_text()))


if __name__ == '__main__':
    unittest.main()
