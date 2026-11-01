#!/usr/bin/env python3
"""Source-evidenced parameterless Blueprint super events, without local assets."""
import copy
import importlib.util
import json
from pathlib import Path
import unittest

PORTING = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('gen', PORTING / 'gen_native_classes.py')
gen = importlib.util.module_from_spec(spec); spec.loader.exec_module(gen)

class ObservedEvents(unittest.TestCase):
 def check_event(self, owner, name, outer, create, flags, access, fn=None):
  f = fn or dict(name=name, create_func=create, flags=[hex(flags)], outer_fn=outer, params=[])
  def build(f, owners={outer}):
   r=gen.Resolver({}); out=gen.build_functions_promoted([f],r,set(),owners,
      parameterized_blueprint_events=gen.UI_PARAMETERIZED_BLUEPRINT_EVENTS[owner]); return out,r.skipped
  out, skipped=build(f); self.assertFalse(skipped); self.assertEqual(len(out),1)
  self.assertEqual(out[0]['access'],access); self.assertEqual(out[0]['args'],[])
  self.assertEqual(out[0]['specifiers'],['BlueprintImplementableEvent'] + (['BlueprintCallable'] if flags & 0x04000000 else []))
  model=dict(name=owner,members_table_order=[],members_decl_order=[],forward_decls=[],uclass='UCLASS()',parent='UObject',functions=out)
  header=gen.render_promoted_header(model,'API','Object.h','reflection'); cpp=gen.render_promoted_cpp(model,'reflection')
  self.assertIn(access+':\n\tUFUNCTION('+', '.join(out[0]['specifiers'])+')\n\tvoid '+name+'();',header)
  self.assertNotIn(owner+'::'+name,cpp)
  self.assertFalse(build(f,{'0xdeadbeef'})[0])
  bad=copy.deepcopy(f);bad['create_func']='0xdeadbeef';self.assertFalse(build(bad)[0])
  bad=copy.deepcopy(f)
  if bad.get('candidates'):
   selected=next(x for x in bad['candidates'] if x.get('outer_fn')==outer);selected['flags']=hex(flags | 0x400)
  else:bad['flags']=[hex(flags | 0x400)]
  self.assertFalse(build(bad)[0])
  bad=copy.deepcopy(f)
  if bad.get('candidates'):
   selected=next(x for x in bad['candidates'] if x.get('outer_fn')==outer);selected['params']=[{'name':'Unobserved'}]
  else:bad['params']=[{'name':'Unobserved'}]
  self.assertFalse(build(bad)[0])
 def test_portable_all_events(self):
  self.assertEqual(len(gen.OBSERVED_PARAMETERLESS_SUPER_EVENTS),14)
  for row in gen.OBSERVED_PARAMETERLESS_SUPER_EVENTS:
   with self.subTest(event=row[:2]): self.check_event(*row)
 def test_original_reflection_if_available(self):
  path=PORTING/'generated/reflection/fortnite_game_reflection.json'
  if not path.exists():self.skipTest('optional original reflection absent')
  data=json.loads(path.read_text())
  by_create={f.get('create_func'):f for b in data['classes_in_groups_with_fort_names'] for f in b.get('functions',[])}
  for row in gen.OBSERVED_PARAMETERLESS_SUPER_EVENTS:
   with self.subTest(event=row[:2]): self.check_event(*row,fn=by_create[row[3]])
if __name__=='__main__': unittest.main()
