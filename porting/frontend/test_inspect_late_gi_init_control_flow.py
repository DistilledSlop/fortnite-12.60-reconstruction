import copy,importlib.util,json,tempfile,unittest
from pathlib import Path
SPEC=importlib.util.spec_from_file_location('late',Path(__file__).with_name('inspect_late_gi_init_control_flow.py'));M=importlib.util.module_from_spec(SPEC);SPEC.loader.exec_module(M)
PROOF=Path('/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/generated/frontend-native-evidence/late-gi-init-recovery-1/proof.json')
class ProofTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):cls.p=json.loads(PROOF.read_text());cls.d=next(x for x in cls.p['dispatches'] if len(x['targets'])==2)
 def test_both_original_dispatch_outcomes(self):
  for p in self.p['dispatches']:
   for outcome in (False,True):M.dispatch(p,outcome)
 def test_all_cfg_paths_close_stack(self):
  self.assertEqual(M.stack_closure(self.p['exact_cfg'],{int(x['va'],16):x for x in self.p['dispatches']}),4037)
 def test_unbalanced_epilogue_rejected(self):
  cfg=copy.deepcopy(self.p['exact_cfg']);x=next(x for x in cfg['instructions'] if x['va']=='0x14945c39b');x['op_str']='rsp, 0x3d0'
  with self.assertRaises(AssertionError):M.stack_closure(cfg,{int(x['va'],16):x for x in self.p['dispatches']})
 def test_wrong_destination_rejected(self):
  p=copy.deepcopy(self.d);p['targets'][1]='0x141e72150'
  with self.assertRaises(AssertionError):M.dispatch(p,True)
 def test_lost_register_restore_rejected(self):
  p=copy.deepcopy(self.d);i=next(i for i,x in enumerate(p['chain']) if x['mnemonic']=='pop');p['chain'].pop(i)
  with self.assertRaises(AssertionError):M.dispatch(p,True)
 def test_flags_changing_operation_rejected(self):
  p=copy.deepcopy(self.d);p['chain'].insert(1,{'mnemonic':'add','op_str':'rax, 1'})
  with self.assertRaises(AssertionError):M.dispatch(p,True)
 def test_proof_mutation_rejected(self):
  with tempfile.TemporaryDirectory() as t:
   p=Path(t)/'proof.json';p.write_bytes(PROOF.read_bytes()+b' ')
   with self.assertRaisesRegex(AssertionError,'proof SHA mismatch'):M.inspect(Path('/nonexistent'),p)
 def test_direct_displacements(self):
  self.assertEqual(M.relative(0x1000,bytes.fromhex('e8fb0f0000')),0x2000)
  self.assertEqual(M.relative(0x1000,bytes.fromhex('0f84fa0f0000')),0x2000)
if __name__=='__main__':unittest.main()
