import unittest
import text_block_style_verifier as verifier
import runner

def good():
    return '\n'.join(['TEXT_BLOCK_STYLE_ASSET style=/Game/UI/Foundation/Buttons/TextStyles/TextStyle-Button-Primary-M.TextStyle-Button-Primary-M_C'] + list(verifier.EXPECTED))

class TextBlockStyleTranscriptTests(unittest.TestCase):
    def test_valid(self):
        self.assertEqual(verifier.verify(good())['rows'], 22)
        self.assertIn('-CommonTextBlockStyleProbe', runner.command('/client', 'text-block-style'))

    def test_missing_failed_duplicate_and_asset(self):
        original = good()
        for row in verifier.EXPECTED:
            for replacement in ('', row.replace('ok=1','ok=0'), row+'\n'+row):
                with self.subTest(row=row), self.assertRaises(ValueError): verifier.verify(original.replace(row,replacement))
        for changed in ('\n'.join(original.splitlines()[1:]), original+'\n'+original.splitlines()[0], original.replace('TextStyle-Button-Primary-M_C','Other_C')):
            with self.assertRaises(ValueError): verifier.verify(changed)

if __name__ == '__main__': unittest.main()
