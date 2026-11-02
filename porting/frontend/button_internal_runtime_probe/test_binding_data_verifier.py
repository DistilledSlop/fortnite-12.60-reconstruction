import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

import binding_data_verifier as verifier
import runner


def rows():
    events = ("begin context reflected_types tag_fields tag_key_partition tag_raw_float_bits "
              "override_text legacy_name_priority invalid_legacy_name legacy_fields legacy_key_order "
              "legacy_float_bits resolver_order copy_arrays delegate_copy map_find map_replace_release "
              "map_remove id_zero id_signed_wrap id_full_wrap reentrant_id_order weak_owner_live "
              "weak_owner_expired weak_callbacks_expired complete").split()
    return [f"BINDDATA v=1 seq={i} event={event} ok=1" for i, event in enumerate(events, 1)]


class BindingDataTests(unittest.TestCase):
    def test_valid_with_engine_lines_and_prefixes(self):
        transcript = 'engine startup\n' + '\n'.join('[LogTemp] ' + row for row in rows())
        self.assertEqual(verifier.verify(transcript)['rows'], 26)

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
                        original + ['bad BINDDATA text'],
                        [original[0] + ' BINDDATA'] + original[1:],
                        ['FALSE' + original[0]] + original[1:],
                        [original[0].replace('seq=1', 'seq=01')] + original[1:]):
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
            self.assertEqual(runner.command(binary, 'action-binding-data'),
                             [str(binary), '-ActionBindingDataProbe', *runner.BASE_ARGS])
            out = root / 'out'
            proc = subprocess.run([sys.executable, str(Path(runner.__file__)),
                '--build-results', str(build), '--binary', str(binary), '--cwd', str(root),
                '--out', str(out), '--case', 'action-binding-data', '--timeout', '3'],
                capture_output=True, text=True)
            self.assertEqual(proc.returncode, 0, proc.stderr)
            receipt = json.loads((out / 'receipt.json').read_text())
            self.assertTrue(receipt['passed'])
            self.assertEqual(receipt['transcript']['rows'], 26)
            self.assertEqual(receipt['transcript'], json.loads((out / 'verification.json').read_text()))


if __name__ == '__main__':
    unittest.main()
