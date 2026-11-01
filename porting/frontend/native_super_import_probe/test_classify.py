import importlib.util
from pathlib import Path
import tempfile
import unittest

spec=importlib.util.spec_from_file_location('classifier',Path(__file__).with_name('classify.py'))
c=importlib.util.module_from_spec(spec);spec.loader.exec_module(c)
class ClassificationTests(unittest.TestCase):
 def test_path_guard(self):
  self.assertEqual(c.parse_path('/Script/CommonUI.CommonButton:BP_OnClicked'),('/Script/CommonUI','CommonButton','BP_OnClicked'))
  for path in ('/Game/A.B:C','/Script/X.Y','/Script/X.Y:Z:Other','/Script/X.Y.Z:Foo','/Script/X.Y:../X','/Script//X.Y:Z','/Script/X..Y:Z',''):
   with self.subTest(path=path),self.assertRaises(ValueError):c.parse_path(path)
 def test_owner_and_flags(self):
  classes={'0x1':'UTest'};parents={'0x1':{'package':'/Script/Module'}}
  fn={'name':'Run','create_func':'0x30','flags':['0x8080800'],'outer_fn':'0x20','params':[]}
  blocks={'0x1':{'functions':[fn]}}
  with tempfile.TemporaryDirectory() as d:
   root=Path(d)
   def run():return c.classify('/Script/Module.Test:Run',classes,parents,blocks,{'0x1':['20']},root,root)
   self.assertEqual(run()['classification'],'observed_non_native_blueprint_event')
   fn['outer_fn']='0x21';self.assertEqual(run()['classification'],'ambiguous_function_owner')
   fn['outer_fn']='0x20';fn['flags']=['0x8080c00'];self.assertEqual(run()['classification'],'native_requires_behavior')
   fn['params']=None;self.assertEqual(run()['classification'],'flags_or_params_unresolved')
   fn['flags']=['0x8080800'];fn['candidates']=[{'outer_fn':'0x20','flags':'0x8080800','nprop':1,'params':[]}]
   self.assertEqual(run()['classification'],'flags_or_params_unresolved')
 def test_ambiguous_class_fails_closed(self):
  with tempfile.TemporaryDirectory() as d:
   rows={'0x1':'UTest','0x2':'ATest'};parents={x:{'package':'/Script/Module'} for x in rows}
   out=c.classify('/Script/Module.Test:Run',rows,parents,{}, {},Path(d),Path(d))
   self.assertEqual(out['classification'],'owner_unresolved_or_ambiguous')
if __name__=='__main__':unittest.main()
