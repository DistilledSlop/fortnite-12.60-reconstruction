import copy, importlib.util, json, os, unittest
from pathlib import Path
PORTING=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('generator',PORTING / 'gen_native_classes.py');g=importlib.util.module_from_spec(spec);spec.loader.exec_module(g)
P=g.UI_PARAMETERIZED_BLUEPRINT_EVENTS['UFortSplashScreenWidget']['OnInputMethodChanged']
FN=dict(name='OnInputMethodChanged',create_func='0x1452e3e00',flags=['0x8080800'],candidates=[dict(outer_fn='0x1452e3dd0',flags='0x8080800',nprop=2,params=copy.deepcopy(P['params']))])
REC=dict(name='ECommonInputType',cpp_type='ECommonInputType',package='/Script/CommonInput',cpp_form='EnumClass',dynamic_type='NotDynamic',enumerators=[dict(name='ECommonInputType::'+name,value=i) for i,name in enumerate(('MouseAndKeyboard','Gamepad','Touch','Count'))])
def setup():
 t=g.TypeTables.__new__(g.TypeTables);t.by_fn={'0x14081ee20':g.TypeInfo('enum','0x14081ee20',REC,'ECommonInputType','/Script/CommonInput','ECommonInputType.h')}
 r=g.Resolver({'0x14081ee20':'ECommonInputType'});gate=g.TypeGate(t,g.EngineTypeIndex(os.devnull),r);gate.current_package='/Script/FortniteUI';r.gate=gate;return r,gate
class TestSplash(unittest.TestCase):
 def build(self,f=FN,owners={'0x1452e3dd0'},remove=False):
  r,gate=setup()
  if remove:r.types.clear()
  return g.build_functions_promoted([f],r,set(),owners,parameterized_blueprint_events={'OnInputMethodChanged':P}),r,gate
 def test_source_event_and_dependency(self):
  out,r,gate=self.build();self.assertFalse(r.skipped);self.assertEqual(out[0]['args'],['ECommonInputType InputType']);self.assertEqual(out[0]['access'],'protected')
  self.assertEqual(g.own_includes(gate,out[0]['used'],gate.current_package),['ECommonInputType.h']);self.assertEqual(g.cross_module_modules(gate,out[0]['used'],gate.current_package),{'CommonInput'})
  self.assertTrue(r.ctype(P['params'][0],{}).startswith('TODO_EnumOtherModule_'))
 def test_changed_owner_flags_or_type_rejected(self):
  self.assertFalse(self.build(owners={'0x1452e3dd1'})[0]);self.assertFalse(self.build(remove=True)[0])
  for key,val in [('create_func','0x1452e3e01')]:
   f=copy.deepcopy(FN);f[key]=val;self.assertFalse(self.build(f)[0])
  for key,val in [('outer_fn','0x1452e3dd1'),('flags','0x8020800'),('nprop',1)]:
   f=copy.deepcopy(FN);f['candidates'][0][key]=val;self.assertFalse(self.build(f)[0])
  for key,val in [('type_refs',{'enum':'0x14081ee21'}),('kind','Byte'),('property_flags','0x10000000000180')]:
   f=copy.deepcopy(FN);f['candidates'][0]['params'][0][key]=val;self.assertFalse(self.build(f)[0])
 def test_wrong_enum_package_rejected(self):
  r,gate=setup();gate.tables.by_fn['0x14081ee20'].package='/Script/OtherPackage'
  self.assertFalse(g.build_functions_promoted([FN],r,set(),{'0x1452e3dd0'},parameterized_blueprint_events={'OnInputMethodChanged':P}))
 def test_original_reflection_if_available(self):
  path=PORTING / 'generated/reflection/fortnite_game_reflection.json'
  if not path.is_file():self.skipTest('optional original reflection absent')
  block=next(b for b in json.loads(path.read_text())['classes_in_groups_with_fort_names'] if b.get('block_va')=='0x146fe5e80')
  fn=next(f for f in block['functions'] if f.get('name')=='OnInputMethodChanged')
  self.assertEqual(self.build(fn)[0][0]['args'],['ECommonInputType InputType'])
if __name__=='__main__':unittest.main()
