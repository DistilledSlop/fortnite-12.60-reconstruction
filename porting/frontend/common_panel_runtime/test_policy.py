import copy, importlib.util, unittest
from pathlib import Path
HERE=Path(__file__).resolve().parent
s=importlib.util.spec_from_file_location('policy',HERE/'policy.py');p=importlib.util.module_from_spec(s);s.loader.exec_module(p)
class Policy(unittest.TestCase):
    def model(self):return {'name':'UCommonActivatableWidget','package':'/Script/CommonUI','functions':[]}
    def block(self):return {'block_va':'0x145a004e0','functions':copy.deepcopy(list(p.EXPECTED.values()))}
    def run_policy(self, model=None, block=None, owner=None):
        return p.apply(self.model() if model is None else model,self.block() if block is None else block, {'0x1408079b0'} if owner is None else owner)
    def test_exact(self):
        m=self.run_policy();self.assertTrue(m['common_activation_runtime'])
        self.assertEqual([x['name'] for x in m['functions']],list(p.EXPECTED))
        self.assertTrue(m['functions'][-1]['blueprint_event'])
        self.assertEqual(m['functions'][-1]['access'],'protected')
    def test_flags(self):
        b=self.block();b['functions'][0]['candidates'][0]['flags']='0x4020400'
        with self.assertRaises(ValueError):self.run_policy(block=b)
    def test_parameters(self):
        b=self.block();b['functions'][-1]['params']=[{'name':'Invented'}]
        with self.assertRaises(ValueError):self.run_policy(block=b)
    def test_owner(self):
        with self.assertRaises(ValueError):self.run_policy(owner={'0x140807980'})
    def test_class(self):
        b=self.block();b['block_va']='0x145a00360'
        with self.assertRaises(ValueError):self.run_policy(block=b)
    def test_duplicate(self):
        m=self.model();m['functions']=[{'name':'ActivateWidget'}]
        with self.assertRaises(ValueError):self.run_policy(model=m)
    def test_panel_not_stubbed(self):
        m={'name':'UCommonActivatablePanel','functions':[]};self.assertEqual(p.apply(m,{},set())['functions'],[])
if __name__=='__main__':unittest.main()
