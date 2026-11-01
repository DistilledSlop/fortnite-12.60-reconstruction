import copy
import json
from pathlib import Path
import importlib.util
import unittest

PORTING=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('generator',PORTING / 'gen_native_classes.py')
gen=importlib.util.module_from_spec(spec);spec.loader.exec_module(gen)
FN=dict(name='StartMusic',create_func='0x1428dc630',flags=['0x8020800'],outer_fn='0x1428dc600',params=[])
OWNER={'0x1428dc600'}
class TestMusicEvent(unittest.TestCase):
 def build(self,fn,owner=OWNER):
  r=gen.Resolver({})
  events=gen.build_functions_promoted([fn],r,set(),owner,parameterized_blueprint_events=gen.UI_PARAMETERIZED_BLUEPRINT_EVENTS['AFortEntryMusicController'])
  return events,r.skipped
 def test_render(self):
  events,skipped=self.build(FN);self.assertFalse(skipped);self.assertEqual(len(events),1)
  model=dict(name='AFortEntryMusicController',members_table_order=[],members_decl_order=[],forward_decls=[],uclass='UCLASS()',parent='AActor',functions=events)
  h=gen.render_promoted_header(model,'FORTNITEGAME_API','GameFramework/Actor.h','reflection')
  cpp=gen.render_promoted_cpp(model,'reflection')
  self.assertIn('public:\n\tUFUNCTION(BlueprintImplementableEvent)\n\tvoid StartMusic();',h)
  self.assertNotIn('BlueprintCallable',h);self.assertNotIn('AFortEntryMusicController::StartMusic',cpp)
 def test_wrong_owner_constructor_signature(self):
  self.assertFalse(self.build(FN,{'0x1428dc601'})[0])
  for k,v in [('create_func','0x1428dc631'),('outer_fn','0x1428dc601'),('flags',['0x8080800']),('params',[{'name':'x'}]),('params',None)]:
   with self.subTest(key=k):
    b=copy.deepcopy(FN);b[k]=v;self.assertFalse(self.build(b)[0])
 def test_actual_reflection_when_available(self):
  path=PORTING / 'generated/reflection/fortnite_game_reflection.json'
  if not path.is_file(): self.skipTest('optional local reflection dump unavailable')
  data=json.loads(path.read_text())
  block=next(x for x in data['classes_in_groups_with_fort_names'] if x.get('block_va')=='0x1463bb8b0')
  fn=next(x for x in block['functions'] if x.get('name')=='StartMusic')
  events,skipped=self.build(fn);self.assertFalse(skipped);self.assertEqual(events[0]['access'],'public')
if __name__=='__main__':unittest.main()
