import unittest
import binding_creation_verifier as verifier
import runner

def successful():
    # Synthetic verifier fixture only. Native evidence must come from CreationProbe.
    warning = "LogDataTable: Warning: UDataTable::FindRow : '' requested row 'Missing' not in DataTable"
    return '\n'.join([warning] + ['LogUIActionRouter: Error: ' + s for s in verifier.DIAGNOSTICS] + list(verifier.EXPECTED))

class CreationTranscriptTests(unittest.TestCase):
    def test_success(self):
        self.assertEqual(verifier.verify(successful())['rows'], 23)
        self.assertIn('-ActionBindingCreationProbe', runner.command('/client', 'action-binding-creation'))

    def test_incomplete_failed_and_reordered(self):
        original = successful()
        for row in verifier.EXPECTED:
            for old,new in ((row,''),(row,row.replace('ok=1','ok=0')),(row,row+'\n'+row)):
                with self.subTest(row=row),self.assertRaises(ValueError): verifier.verify(original.replace(old,new))
        for row in verifier.DIAGNOSTICS:
            with self.assertRaises(ValueError): verifier.verify(original.replace(row,row+' changed'))
        with self.assertRaises(ValueError): verifier.verify(original.replace('LogUIActionRouter: Error:','LogUIActionRouter: Warning:'))
        with self.assertRaises(ValueError): verifier.verify(original+'\n'+original.splitlines()[0])

if __name__ == '__main__': unittest.main()
