import sys,unittest,tempfile,json
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import verify_events as e
import audit_manifest
import test_events
P=Path(__file__).resolve().parents[1]
ROOT=P.parents[2]
SOURCES=json.loads((P/'source-expectations.json').read_text())
def source(name):
 matches=[ROOT/relative for relative in SOURCES if Path(relative).name==name]
 if len(matches)!=1:raise ValueError('unique source path '+name)
 return matches[0]
class V3(unittest.TestCase):
 def test_manifest_rejects_tampering(self):
  with tempfile.TemporaryDirectory() as tmp:
   d=Path(tmp);a=d/'one';a.write_text('one');(d/'manifest.json').write_text(json.dumps({'files':{'one':{'sha256':audit_manifest.digest(a),'size':3}}}));self.assertTrue(audit_manifest.audit(d)['passed'])
   a.write_text('two')
   with self.assertRaises(ValueError):audit_manifest.audit(d)
   a.write_text('one');(d/'extra').write_text('x')
   with self.assertRaises(ValueError):audit_manifest.audit(d)
 def test_gc_and_post_action_instrumentation(self):
  s=source('ContextRuntimeProbe.cpp').read_text();self.assertIn('SecondWeak.Get(true) == Second',s);self.assertIn('SecondWeak.Get(true) == nullptr',s)
  r=source('BlueprintContextRuntime.cpp').read_text();kill=r[r.index('void MarkPendingKill'):r.index('UWorld* PlayerWorld')];self.assertLess(kill.index('C->MarkPendingKill()'),kill.index('CONTEXT_OBS kill'));self.assertIn('C->IsPendingKill()',kill)
  rm=r[r.index('void RemovePlayer'):r.index('template <typename Fn> void ForEachDerivedClass')];self.assertLess(rm.index('Players.Remove('),rm.index('CONTEXT_OBS remove_map'));self.assertIn('Players.Contains(',rm)
  g=source('gen_native_classes.py').read_text();self.assertIn('ObserveContextDeinitialize(this, true); Deinitialize(); BlueprintContextRuntime::ObserveContextDeinitialize(this, false)',g)
 def test_exact_requested_types_and_auto_driver(self):
  s=source('ContextRuntimeProbe.cpp').read_text();self.assertNotIn('ANY_PACKAGE',s)
  for name in ('/Script/FortniteUI.FortFrontEndContext','/Script/FortniteUI.FortGlobalUIContext','/Script/FortniteUI.FortUIDirector','/Script/CommonUI.CommonUserWidget'):self.assertIn(name,s)
  self.assertIn('UGameplayStatics::OpenLevel(W, FName(*Map), true)',s);self.assertIn('LifecycleCallbacksObserved()',s);self.assertIn('FCoreDelegates::OnEndFrame.AddStatic',s)
 def test_gc_deinitialize_and_measured_states_rejected(self):
  good=test_events.reg();e.registry(good)
  for a,b in [('pending=1','pending=0'),('remains=0','remains=1'),('OBS deinit_begin object=2a','OBS missing object=2a'),('OBS deinit_end object=2a','OBS deinit_end object=2b')]:
   with self.subTest(a=a),self.assertRaises(ValueError):e.registry(good.replace(a,b,1))
 def test_other_players_copied_and_verified(self):
  good=test_events.life(); extra=('CONTEXT_OBS copy_player player=5\nCONTEXT_OBS copy_expected player=5 object=c\nCONTEXT_OBS ready object=c world=4\nCONTEXT_OBS copy_player_end player=5\n')
  good=good.replace('CONTEXT_OBS copy_end\nCONTEXT_OBS deferred_end',extra+'CONTEXT_OBS copy_end\nCONTEXT_OBS deferred_end')
  extra2=extra.replace('ready object=c','cleanup_dispatch object=c')
  good=good.replace('CONTEXT_OBS copy_end\nCONTEXT_OBS world_cleanup_end',extra2+'CONTEXT_OBS copy_end\nCONTEXT_OBS world_cleanup_end')
  e.lifecycle(good)
  with self.assertRaises(ValueError):e.lifecycle(good.replace('CONTEXT_OBS ready object=c world=4\n',''))
 def test_each_dispatch_must_be_complete_once(self):
  good=test_events.life();e.lifecycle(good)
  for a,b in [('CONTEXT_OBS ready object=b world=4\n',''),('CONTEXT_OBS cleanup_dispatch object=b world=4\n',''),('CONTEXT_OBS ready object=a world=4','CONTEXT_OBS ready object=b world=4\nCONTEXT_OBS ready object=a world=4'),('CONTEXT_OBS world_init world=4\n',''),('CONTEXT_OBS consumed world=4 name=W remains=0','CONTEXT_OBS consumed world=4 name=W remains=1')]:
   with self.subTest(a=a),self.assertRaises(ValueError):e.lifecycle(good.replace(a,b,1))
if __name__=='__main__':unittest.main()
