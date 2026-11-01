import hashlib
import json
import tempfile
import unittest
from pathlib import Path

import common_input_action_row_verifier as verifier
import runner


def rows(count=3):
    return ([verifier.LAYOUT] + [f'{verifier.MARKER} row={x} passed=1' for x in verifier.ROWS]
            + [f'{verifier.MARKER} row=asset_values row_count={count}']
            + [f'{verifier.MARKER} row=asset_sample index={i} name=Name{i}' for i in range(min(5, count))]
            + [f'{verifier.MARKER} row=complete passed=1'])


class CommonInputActionRowTests(unittest.TestCase):
    def test_valid_counts_and_prefix(self):
        for count in (1, 3, 5, 100):
            result = verifier.verify('unrelated\n' + '\n'.join('[LogTemp] ' + x for x in rows(count)))
            self.assertEqual(result['asset_row_count'], count)
            self.assertEqual(len(result['asset_samples']), min(count, 5))

    def test_missing_modified_or_failed_rows(self):
        original = rows()
        for i in range(len(original)):
            variants = [original[:i] + original[i + 1:]]
            if 'row=asset_sample' not in original[i]:
                variants.append(original[:i] + [original[i] + ' extra'] + original[i + 1:])
            for changed in variants:
                with self.subTest(i=i, row=changed[i:i + 1]), self.assertRaises(ValueError):
                    verifier.verify('\n'.join(changed))
        for i, row in enumerate(original):
            if 'passed=1' in row:
                changed = original.copy()
                changed[i] = row.replace('passed=1', 'passed=0')
                with self.subTest(i=i), self.assertRaises(ValueError):
                    verifier.verify('\n'.join(changed))

    def test_bad_layout_count_samples_order_duplicates_and_terminal(self):
        original = rows()
        cases = [original + [original[-1]], original[:1] + [original[2], original[1]] + original[3:],
                 [original[0].replace('row_size=1064', 'row_size=1065')] + original[1:],
                 [original[0] + ' ' + verifier.MARKER] + original[1:],
                 [x.replace('row_count=3', 'row_count=0') for x in original],
                 [x.replace('row_count=3', 'row_count=4') for x in original],
                 [x.replace('index=1', 'index=2') for x in original],
                 [x.replace('name=Name1', 'name=Name0') for x in original],
                 [x.replace('name=Name1', 'name=Zed') for x in original],
                 original[:-1] + [f'{verifier.MARKER} row=unknown passed=1', original[-1]]]
        for changed in cases:
            with self.subTest(case=changed), self.assertRaises(ValueError):
                verifier.verify('\n'.join(changed))

    def test_runner_command_and_runtime_args(self):
        self.assertEqual(runner.command(Path('/tmp/game'), 'common-input-row')[1], '-CommonInputActionRowProbe')
        values = ['-FortniteLegacyMaterialIngress=a', '-FortniteLegacyMaterialManifest=b',
                  '-FortniteLegacyMaterialManifestSHA256=c']
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / 'args.json'
            raw = json.dumps(values).encode()
            p.write_bytes(raw)
            self.assertEqual(runner.runtime_args(p), (values, hashlib.sha256(raw).hexdigest()))
            for bad in (values[:-1], values + [values[0]], values + ['-ExecCmds=quit'],
                        values + ['-CommonInputActionRowProbe=0'], [*values[:2], values[2] + '\n']):
                p.write_text(json.dumps(bad))
                with self.subTest(bad=bad), self.assertRaises(ValueError):
                    runner.runtime_args(p)


if __name__ == '__main__':
    unittest.main()
