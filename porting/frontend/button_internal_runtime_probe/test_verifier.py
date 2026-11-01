import unittest

from verifier import ROWS, verify


def transcript():
    return [f'LogTemp: Display: BTNINTERNAL v=1 seq={i} event={event} ok=1'
            for i, event in enumerate(ROWS, 1)]


class Contract(unittest.TestCase):
    def test_exact(self):
        self.assertEqual(len(ROWS), 34)
        self.assertEqual(verify('\n'.join(transcript()))['rows'], 34)

    def test_each_missing_failed_or_wrong_event(self):
        lines = transcript()
        for i in range(len(lines)):
            for altered in (lines[:i] + lines[i + 1:],
                            lines[:i] + [lines[i].replace('ok=1', 'ok=0')] + lines[i + 1:],
                            lines[:i] + [lines[i].replace('event=', 'event=bad_')] + lines[i + 1:]):
                with self.subTest(index=i, row=altered[i:i + 1]), self.assertRaises(ValueError):
                    verify('\n'.join(altered))

    def test_duplicate_and_out_of_order(self):
        lines = transcript()
        for altered in (lines + lines[-1:], lines[:5] + lines[4:5] + lines[5:],
                        lines[:5] + [lines[6], lines[5]] + lines[7:]):
            with self.assertRaises(ValueError):
                verify('\n'.join(altered))

    def test_malformed_markers(self):
        lines = transcript()
        for bad in (lines[0] + ' junk', lines[0] + ' BTNINTERNAL',
                    lines[0].replace('v=1', 'v=2'), lines[0].replace('ok=1', 'ok=10'),
                    lines[0].replace('seq=1', 'seq=' + '9' * 10000),
                    'BTNINTERNAL garbage'):
            with self.subTest(row=bad[:100]), self.assertRaises(ValueError) as error:
                verify('\n'.join([bad] + lines[1:]))
            self.assertLess(len(str(error.exception)), 300)

    def test_unrelated_log_lines_are_allowed(self):
        self.assertTrue(verify('startup\n' + '\n'.join(transcript()) + '\nshutdown')['passed'])


if __name__ == '__main__':
    unittest.main()
