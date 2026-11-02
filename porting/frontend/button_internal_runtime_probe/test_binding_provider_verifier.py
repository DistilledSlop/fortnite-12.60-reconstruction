import unittest
import binding_provider_verifier as verifier
import runner

class ProviderTranscriptTests(unittest.TestCase):
    def test_full_ordered_transcript(self):
        self.assertEqual(verifier.verify('\n'.join('[LogTemp] ' + row for row in verifier.EXPECTED))['rows'], 20)
        self.assertIn('-ActionBindingProviderProbe', runner.command('/client', 'action-binding-provider'))

    def test_incomplete_failed_extra_and_reordered(self):
        original = list(verifier.EXPECTED)
        for i, row in enumerate(original):
            for rows in (original[:i] + original[i+1:],
                         original[:i] + [row.replace('ok=1', 'ok=0')] + original[i+1:],
                         original[:i] + [row, row] + original[i+1:]):
                with self.subTest(i=i), self.assertRaises(ValueError):
                    verifier.verify('\n'.join(rows))
        for rows in (original[1:] + original[:1], original + ['BINDPROVIDER malformed'],
                     ['FALSE' + original[0]] + original[1:]):
            with self.assertRaises(ValueError): verifier.verify('\n'.join(rows))

if __name__ == '__main__': unittest.main()
