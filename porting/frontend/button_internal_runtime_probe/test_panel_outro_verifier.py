import unittest

import panel_outro_verifier as verifier


def rows():
    return [f'PANELOUTRO v=1 seq={i} case={case} event={event} state={state} introed={introed} ok=1'
            for i, (case, event, state, introed) in enumerate(verifier.expected(), 1)]


class PanelOutroVerifierTests(unittest.TestCase):
    def test_valid_with_prefix_and_unrelated_lines(self):
        data = rows()
        self.assertEqual(len(data), 60)
        result = verifier.verify('unrelated\n' + '\n'.join('[LogTemp] ' + row for row in data))
        self.assertEqual(result, {'passed': True, 'rows': 60, 'scope': verifier.SCOPE})
        self.assertIn('ProcessEvent', result['scope'])

    def test_every_row_required_and_successful(self):
        original = rows()
        for i in range(len(original)):
            for changed in (original[:i] + original[i + 1:],
                            original[:i] + [original[i].replace('ok=1', 'ok=0')] + original[i + 1:],
                            original[:i] + [original[i].replace('event=', 'event=altered_')] + original[i + 1:]):
                with self.subTest(position=i), self.assertRaises(ValueError):
                    verifier.verify('\n'.join(changed))

    def test_malformed_duplicate_reorder_and_bounded_errors(self):
        original = rows()
        cases = [original + [original[-1]], original[:8] + [original[9], original[8]] + original[10:],
                 [original[0] + ' garbage'] + original[1:],
                 [original[0] + ' PANELOUTRO'] + original[1:],
                 [original[0].replace('seq=1 ', 'seq=' + '9' * 10000 + ' ')] + original[1:],
                 [original[0].replace('case=driver', 'case=' + 'a' * 10000)] + original[1:],
                 [original[0].replace('state=-1', 'state=0')] + original[1:],
                 [original[0].replace('introed=-1', 'introed=0')] + original[1:],
                 [original[0].replace('v=1', 'v=2')] + original[1:]]
        for data in cases:
            with self.subTest(prefix=data[0][:80]), self.assertRaises(ValueError) as error:
                verifier.verify('\n'.join(data))
            self.assertLess(len(str(error.exception)), 200)


if __name__ == '__main__':
    unittest.main()
