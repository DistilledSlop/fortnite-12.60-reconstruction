import unittest

import main_button_verifier as verifier


def rows():
    result = [f'MAINBTN v=1 seq={i} event={event} ok=1'
              for i, event in enumerate(('begin', 'defaults', 'metadata'), 1)]
    for visibility in range(5):
        for bits in range(8):
            value = int(visibility not in (1, 2) and bits == 7)
            result.append(f'MAINBTN v=1 seq={len(result) + 1} event=state visibility={visibility} '
                          f'bits={bits} direct={value} alias={value} reflected={value} ok=1')
    result.append('MAINBTN v=1 seq=44 event=complete ok=1')
    return result


class MainButtonVerifierTests(unittest.TestCase):
    def test_valid_with_log_prefix_and_unrelated_lines(self):
        text = 'unrelated\n' + '\n'.join('[LogTemp] ' + row for row in rows())
        self.assertEqual(verifier.verify(text)['rows'], 44)

    def test_every_state_outcome_is_checked(self):
        for index in range(3, 43):
            for field in ('direct', 'alias', 'reflected'):
                with self.subTest(index=index, field=field):
                    data = rows()
                    old = field + '=1' if field + '=1' in data[index] else field + '=0'
                    data[index] = data[index].replace(old, field + ('=0' if old.endswith('1') else '=1'))
                    with self.assertRaises(ValueError):
                        verifier.verify('\n'.join(data))

    def test_missing_reordered_duplicate_failed_and_malformed(self):
        original = rows()
        cases = [original[1:], original[:-1], [original[-1]], original + [original[-1]],
                 original[:3] + [original[4], original[3]] + original[5:],
                 [original[0].replace('ok=1', 'ok=0')] + original[1:],
                 [original[0] + ' extra'] + original[1:],
                 [original[0] + ' MAINBTN'] + original[1:],
                 original[:3] + [original[3].replace('bits=0', 'bits=1')] + original[4:]]
        for data in cases:
            with self.subTest(first=data[0]):
                with self.assertRaises(ValueError):
                    verifier.verify('\n'.join(data))


if __name__ == '__main__':
    unittest.main()
