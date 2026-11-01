import copy, importlib.util, unittest
from pathlib import Path
p0=Path(__file__).resolve().parent/'policy.py'
s=importlib.util.spec_from_file_location('policy',p0); p=importlib.util.module_from_spec(s);s.loader.exec_module(p)
class Manager(unittest.TestCase):
 def model(self): return {'name':'UCommonInputManager','package':'/Script/CommonUI','functions':[],'members_decl_order':[{'name':'CurrentlyHeldActionInputHandler','decl':'TScriptInterface<ICommonActionHandlerInterface> CurrentlyHeldActionInputHandler;'}]}
 def block(self): return {'block_va':'0x145a05fd0','functions':[copy.deepcopy(p.MANAGER_EXPECTED)],'properties':[copy.deepcopy(p._outro.HELD_PROPERTY)]}
 def run_it(self,m=None,b=None,owner=None): return p.apply(self.model() if m is None else m,self.block() if b is None else b,{'0x14080e9a0'} if owner is None else owner)
 def test_signature(self):
  f=self.run_it()['functions'][0]; self.assertEqual((f['name'],f['ret'],f['args'],f['const'],f['access'],f['specifiers'],f['runtime_implementation']),('IsInputSuspended','bool',[],True,'public',['BlueprintCallable','BlueprintPure'],True))
 def test_owner(self):
  with self.assertRaises(ValueError):self.run_it(owner=set())
  b=self.block();b['block_va']='0x0'
  with self.assertRaises(ValueError):self.run_it(b=b)
  m=self.model();m['package']='/Script/Other'
  with self.assertRaises(ValueError):self.run_it(m=m)
 def test_original_signature_guards(self):
  for change in ('flags','params','create_func','outer_fn','missing'):
   b=self.block();f=b['functions'][0]
   if change=='missing':b['functions']=[]
   elif change=='params':f['params'][0]['property_flags']='0x0'
   else:f[change]=None
   with self.subTest(change=change),self.assertRaises(ValueError):self.run_it(b=b)
 def test_duplicate(self):
  m=self.model();m['functions']=[{'name':'IsInputSuspended'}]
  with self.assertRaises(ValueError):self.run_it(m=m)
if __name__=='__main__':unittest.main()
