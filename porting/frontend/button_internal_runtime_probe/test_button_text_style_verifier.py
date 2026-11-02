import unittest
import button_text_style_verifier as verifier
import runner

def good():
    asset = 'BUTTON_STYLE_ASSET button=/Game/UI/Foundation/Buttons/IconTextButton.IconTextButton_C style=/Game/UI/TestStyle.TestStyle_C text=/Game/UI/TestText.Default__TestText_C'
    return '\n'.join([asset] + list(verifier.EXPECTED))

class TextStyleTranscriptTests(unittest.TestCase):
    def test_valid(self):
        self.assertEqual(verifier.verify(good())['rows'], 32)
        self.assertIn('-CommonButtonTextStyleProbe', runner.command('/client', 'button-text-style'))

    def test_incomplete_failed_duplicate_and_no_asset(self):
        original = good()
        for row in verifier.EXPECTED:
            for replacement in ('', row.replace('ok=1', 'ok=0'), row+'\n'+row):
                with self.subTest(row=row),self.assertRaises(ValueError): verifier.verify(original.replace(row,replacement))
        for changed in (original.replace('available=1','available=0'), original.replace('IconTextButton_C','Other_C'),
                        '\n'.join(original.splitlines()[1:]), original+'\n'+original.splitlines()[0]):
            with self.assertRaises(ValueError): verifier.verify(changed)

if __name__ == '__main__': unittest.main()
