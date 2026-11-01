import copy,importlib.util,unittest
from pathlib import Path
HERE=Path(__file__).resolve().parent
s=importlib.util.spec_from_file_location('outro',HERE/'panel_outro_policy.py');p=importlib.util.module_from_spec(s);s.loader.exec_module(p)
class OutroPolicy(unittest.TestCase):
 def model(self):return {'name':'UCommonActivatablePanel','parent':'UCommonActivatableWidget','package':'/Script/CommonUI','functions':[],'panel_intro_activation_runtime':True}
 def block(self):return {'block_va':'0x145a00360','functions':copy.deepcopy(list(p.EXPECTED.values()))}
 def execs(self):return {n:copy.deepcopy(f['original_native_exec']) for n,f in p.CONTRACT.items()}
 def run_it(self,m=None,b=None,owners=None,execs=None):return p.apply(self.model() if m is None else m,self.block() if b is None else b,{'0x140807980'} if owners is None else owners,self.execs() if execs is None else execs)
 def test_model(self):
  m=self.run_it();self.assertEqual([(f['name'],f['access'],f['specifiers'],f['blueprint_event']) for f in m['functions']],[('EndOutro','protected',['BlueprintCallable'],False),('OnBeginOutro','protected',['BlueprintNativeEvent'],True)])
  self.assertEqual(set(m['used']),set(p.TYPE_CONTRACT));self.assertIn('CommonPanelActionNative.h',p.header_prelude(m)['includes']);self.assertTrue(p.declarations(m));self.assertTrue(p.definitions(m))
 def test_composition_preserves_intro(self):
  spec=importlib.util.spec_from_file_location('combined',HERE/'policy.py');combined=importlib.util.module_from_spec(spec);spec.loader.exec_module(combined)
  m=self.model();del m['panel_intro_activation_runtime'];b=self.block();b['functions']+=copy.deepcopy(list(combined._intro.EXPECTED.values()))
  out=combined.apply(m,b,{'0x140807980'},self.execs());self.assertEqual(len(out['functions']),5)
  self.assertEqual(out['runtime_module_dependencies'],['CommonInput'])
  decl=combined.declarations(out)
  for name in ['NativeInputMethodChanged','NativeInputManager','NativePreActivation','NativePostActivation','NativePreDeactivation','NativePostDeactivation']:self.assertIn(name,decl)
  self.assertEqual(set(combined.header_prelude(out)['includes']),{'CommonPanelActivationNative.h','CommonPanelActionNative.h'})
 def test_descriptor_exec_guards(self):
  for n in p.EXPECTED:
   for key in p.EXPECTED[n]:
    b=self.block();next(f for f in b['functions'] if f['name']==n)[key]='bad'
    with self.subTest(name=n,descriptor=key),self.assertRaises(ValueError):self.run_it(b=b)
   for key in self.execs()[n]:
    e=self.execs();e[n][key]='bad'
    with self.subTest(name=n,exec=key),self.assertRaises(ValueError):self.run_it(execs=e)
   for mode in ['missing','duplicate']:
    b=self.block()
    if mode=='missing':b['functions']=[f for f in b['functions'] if f['name']!=n]
    else:b['functions'].append(copy.deepcopy(p.EXPECTED[n]))
    with self.subTest(name=n,mode=mode),self.assertRaises(ValueError):self.run_it(b=b)
  with self.assertRaises(ValueError):p.apply(self.model(),self.block(),{'0x140807980'})
 def test_identity_collision_intro(self):
  for key in ['package','parent']:
   m=self.model();m[key]='wrong'
   with self.assertRaises(ValueError):self.run_it(m=m)
  b=self.block();b['block_va']='wrong'
  with self.assertRaises(ValueError):self.run_it(b=b)
  with self.assertRaises(ValueError):self.run_it(owners=set())
  for flag,value in [('panel_intro_activation_runtime',False),('panel_outro_runtime',True)]:
   m=self.model();m[flag]=value
   with self.assertRaises(ValueError):self.run_it(m=m)
  for group in ['functions','members_table_order','members_decl_order','fields','helpers','static_helpers']:
   for name in p.NATIVE_NAMES|set(p.EXPECTED):
    m=self.model();m[group]=[{'name':name}]
    with self.subTest(group=group,name=name),self.assertRaises(ValueError):self.run_it(m=m)
 def test_manager(self):
  m={'name':'UCommonInputManager','members_decl_order':[{'name':'CurrentlyHeldActionInputHandler','decl':'TScriptInterface<ICommonActionHandlerInterface> CurrentlyHeldActionInputHandler;'}]};b={'properties':[copy.deepcopy(p.HELD_PROPERTY)]}
  self.assertIs(p.apply(m,b,set()),m)
  for key in p.HELD_PROPERTY:
   bad=copy.deepcopy(b);bad['properties'][0][key]='invalid mutation'
   with self.subTest(key=key),self.assertRaises(ValueError):p.apply(m,bad,set())
  for decl in ['', 'void* CurrentlyHeldActionInputHandler;']:
   bad=copy.deepcopy(m);bad['members_decl_order'][0]['decl']=decl
   with self.assertRaises(ValueError):p.apply(bad,b,set())
 def test_type_guards_and_native_field(self):
  class Info:
   def __init__(self,rec):self.rec=copy.deepcopy(rec);self.package='/Script/CommonUI';self.kind='struct' if rec['name']=='CommonInputActionHandlerData' else 'delegate'
  class Gate:
   def __init__(self):self.entries={fn:Info(rec) for fn,rec in p.TYPE_CONTRACT.items()};self.models={fn:{'forward_decls':[]} for fn in p.TYPE_CONTRACT};self.failure=None
   def info(self,fn):return self.entries.get(fn)
   def reason(self,info):return self.failure
  g=Gate();p.validate_types(g);self.assertIn('NativePopupMenu','\n'.join(p.struct_declarations(g.models['0x140809900'])))
  for fn in p.TYPE_CONTRACT:
   for mode in ['missing','record','package','unavailable']:
    g=Gate()
    if mode=='missing':del g.entries[fn]
    if mode=='record':g.entries[fn].rec['name']='wrong'
    if mode=='package':g.entries[fn].package='/Script/Other'
    if mode=='unavailable':g.failure='unavailable'
    with self.subTest(fn=fn,mode=mode),self.assertRaises(ValueError):p.validate_types(g)
if __name__=='__main__':unittest.main()
