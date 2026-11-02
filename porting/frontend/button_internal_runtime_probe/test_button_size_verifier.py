import unittest
import button_size_verifier as v

class ButtonSizeVerifierTests(unittest.TestCase):
    def test_complete_prefixed_transcript(self):
        result=v.verify('\n'.join('[runtime] '+row for row in v.EXPECTED))
        self.assertTrue(result['passed'])
        self.assertEqual(result['rows'],11)
    def test_incomplete_failed_reordered_or_duplicate_receipts_rejected(self):
        rows=list(v.EXPECTED)
        cases=[rows[:-1],rows+[rows[-1]],rows[:3]+rows[4:],rows[:1]+list(reversed(rows[1:3]))+rows[3:]]
        cases += [[row.replace('ok=1','ok=0') if i==3 else row for i,row in enumerate(rows)]]
        cases += [['bad_'+row if i==0 else row for i,row in enumerate(rows)]]
        for case in cases:
            with self.subTest(case=case),self.assertRaises(ValueError):v.verify('\n'.join(case))
if __name__=='__main__':unittest.main()
