import unittest
from pathlib import Path

import confirmation_description_verifier as verifier
import runner


def rows():
    return [verifier.LAYOUT] + [f'{verifier.MARKER} row={row} passed=1' for row in verifier.ROWS]


class ConfirmationDescriptionVerifierTests(unittest.TestCase):
    def test_valid_with_engine_prefix_and_unrelated_logs(self):
        result = verifier.verify('other log\n' + '\n'.join('[LogTemp] ' + row for row in rows()))
        self.assertEqual(result, {'passed': True, 'rows': 13, 'scope': verifier.SCOPE})
        self.assertIn('Blueprint', result['scope'])

    def test_all_rows_required_and_successful(self):
        original = rows()
        for i in range(len(original)):
            for changed in (original[:i] + original[i + 1:],
                            original[:i] + [original[i] + ' extra'] + original[i + 1:]):
                with self.subTest(position=i), self.assertRaises(ValueError):
                    verifier.verify('\n'.join(changed))
        for i in range(1, len(original)):
            changed = original.copy()
            changed[i] = changed[i].replace('passed=1', 'passed=0')
            with self.subTest(position=i), self.assertRaises(ValueError):
                verifier.verify('\n'.join(changed))

    def test_invalid_layout_duplicate_reorder_and_multiple_markers(self):
        original = rows()
        cases = [original + [original[-1]], original[:2] + [original[3], original[2]] + original[4:],
                 [original[0] + ' ' + verifier.MARKER] + original[1:],
                 [original[0].replace('size=280', 'size=281', 1)] + original[1:],
                 [original[0].replace('ordinary=248', 'ordinary=249')] + original[1:],
                 [original[0].replace('struct_align=8', 'struct_align=16')] + original[1:],
                 original[:1] + [original[1].replace('row=layout', 'row=other')] + original[2:]]
        for changed in cases:
            with self.subTest(row=changed[0][:100]), self.assertRaises(ValueError):
                verifier.verify('\n'.join(changed))

    def test_runner_command(self):
        argv = runner.command(Path('/tmp/game'), 'confirmation-description')
        self.assertEqual(argv[1], '-ConfirmationDescriptionProbe')
        self.assertEqual(tuple(argv[2:]), runner.BASE_ARGS)


if __name__ == '__main__':
    unittest.main()
