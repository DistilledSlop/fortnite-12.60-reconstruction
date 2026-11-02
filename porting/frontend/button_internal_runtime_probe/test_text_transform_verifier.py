import unittest
import text_transform_verifier as v
import runner

def rows(skip=False):
    events=[e for e in v.EVENTS if e!=v.OPTIONAL or not skip]
    lines=[f'TEXTTRANSFORM v=1 seq={i} event={e} ok=1' for i,e in enumerate(events,1)]
    if skip: lines.insert(v.EVENTS.index(v.OPTIONAL),'TEXTTRANSFORM v=1 event=unicode_length_refusal_unavailable source_len=1 transformed_len=1')
    return ['TEXT_TRANSFORM_ASSET style=/Game/UI/Foundation/Buttons/TextStyles/TextStyle-Button-Primary-M.TextStyle-Button-Primary-M_C']+lines

class TransformVerifierTests(unittest.TestCase):
    def test_both_outcomes(self):
        for skip in (False,True):
            result=v.verify('\n'.join(rows(skip)))
            self.assertEqual(result['unicode_length_refusal_exercised'],not skip)
        self.assertIn('-CommonTextTransformProbe',runner.command('/client','text-transform'))
    def test_reject(self):
        r=rows()
        for i,line in enumerate(r):
            for changed in (r[:i]+r[i+1:],r[:i]+[line,line]+r[i+1:],r[:i]+[line.replace('ok=1','ok=0')]+r[i+1:]):
                if changed==r: continue
                with self.subTest(i=i),self.assertRaises(ValueError):v.verify('\n'.join(changed))
        with self.assertRaises(ValueError):v.verify('\n'.join(rows(True)).replace('transformed_len=1','transformed_len=2'))

if __name__=='__main__':unittest.main()
