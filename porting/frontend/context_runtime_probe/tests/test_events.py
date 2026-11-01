import sys, unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import verify_events as v

def reg():
 out=['CONTEXT_OBS owner manager=1 gi=2','CONTEXT_CASE identity player=3 gi=2 world=4']
 def add(s):out.append('CONTEXT_'+s)
 old={}
 for phase in (1,2):
  add(f'CASE create{phase}_begin');new={}
  for c in ('a','b'):
   obj=f'{phase}{c}';add(f'OBS insert player=3 class={c} old={old.get(c,"0")}');now={**old,**new,c:'0'}
   add(f'OBS init_begin player=3 class={c} object={obj} self=0 slot=1')
   for cls,x in now.items(): add(f'OBS peer player=3 creating={c} class={cls} value={x}')
   add(f'OBS init_end player=3 class={c} object={obj} self=0 slot=1');add(f'OBS stored player=3 class={c} value={obj}');new[c]=obj
  for obj in new.values():add(f'OBS post object={obj}')
  add(f'CASE create{phase}_end');old=new
 add('CASE remove_begin')
 for obj in old.values():add(f'OBS removing object={obj} world=4');add(f'OBS kill object={obj} pending=1')
 add('OBS remove_map player=3 remains=0');add('CASE remove_end');add('CASE gc_begin object=2a');add('OBS deinit_begin object=2a');add('OBS deinit_end object=2a');add('CASE gc_end')
 return '\n'.join(out)
def life():return '''CONTEXT_OBS owner manager=1 gi=2
CONTEXT_LIFE begin player=3 gi=2 world=4
CONTEXT_OBS stored player=3 class=a value=a
CONTEXT_OBS stored player=3 class=b value=b
CONTEXT_LIFE created player=3
CONTEXT_OBS world_init world=4
CONTEXT_OBS pending world=4 name=W
CONTEXT_OBS deferred name=W
CONTEXT_OBS consumed world=4 name=W remains=0
CONTEXT_OBS owner_match world=4 actual=2 first=2 match=1
CONTEXT_OBS copy_begin
CONTEXT_OBS copy_player player=3
CONTEXT_OBS copy_expected player=3 object=a
CONTEXT_OBS copy_expected player=3 object=b
CONTEXT_OBS ready object=a world=4
CONTEXT_OBS ready object=b world=4
CONTEXT_OBS copy_player_end player=3
CONTEXT_OBS copy_end
CONTEXT_OBS deferred_end name=W
CONTEXT_OBS world_cleanup world=4
CONTEXT_OBS owner_match world=4 actual=2 first=2 match=1
CONTEXT_OBS copy_begin
CONTEXT_OBS copy_player player=3
CONTEXT_OBS copy_expected player=3 object=a
CONTEXT_OBS copy_expected player=3 object=b
CONTEXT_OBS cleanup_dispatch object=a world=4
CONTEXT_OBS cleanup_dispatch object=b world=4
CONTEXT_OBS copy_player_end player=3
CONTEXT_OBS copy_end
CONTEXT_OBS world_cleanup_end world=4
CONTEXT_LIFE end player=3 world=4
CONTEXT_OBS removing object=a world=4
CONTEXT_OBS kill object=a pending=1
CONTEXT_OBS removing object=b world=4
CONTEXT_OBS kill object=b pending=1
CONTEXT_OBS remove_map player=3 remains=0
CONTEXT_LIFE removed player=3
'''
class Events(unittest.TestCase):
 def test_registry_and_mutations(self):
  text=reg();self.assertEqual(v.registry(text)['classes'],2)
  for old,new in [('self=0 slot=1','self=1 slot=1'),('old=1a','old=0'),('value=1b','value=0'),('CASE create2_end','CASE missing'),('OBS kill object=2a','OBS kill object=2b'),('pending=1','pending=0'),('remains=0','remains=1'),('OBS deinit_end object=2a','OBS absent object=2a')]:
   with self.subTest(old=old),self.assertRaises((ValueError,KeyError)):v.registry(text.replace(old,new,1))
 def test_lifecycle_and_mutations(self):
  text=life();v.lifecycle(text)
  for old,new in [('remains=0','remains=1'),('first=2 match=1','first=9 match=1'),('OBS deferred name=W','OBS wrong name=W'),('OBS ready object=a','OBS wrong object=a'),('OBS cleanup_dispatch','OBS missing'),('CONTEXT_OBS ready object=b world=4\n',''),('CONTEXT_OBS cleanup_dispatch object=b world=4\n',''),('CONTEXT_OBS ready object=a world=4','CONTEXT_OBS ready object=b world=4\nCONTEXT_OBS ready object=a world=4'),('CONTEXT_OBS pending world=4 name=W','CONTEXT_OBS invalid world=4 name=W')]:
   with self.subTest(old=old),self.assertRaises((ValueError,KeyError)):v.lifecycle(text.replace(old,new,1))
if __name__=='__main__':unittest.main()
