import copy, importlib.util, unittest
from pathlib import Path
HERE=Path(__file__).resolve().parent
s=importlib.util.spec_from_file_location('policy',HERE/'policy.py'); p=importlib.util.module_from_spec(s);s.loader.exec_module(p)
class Button(unittest.TestCase):
 def model(self): return {'name':'UCommonButtonInternal','package':'/Script/CommonUI','parent':'UButton','functions':[],'helpers':[],'fields':[]}
 def run_it(self,m=None,b=None,o=None):return p.apply(self.model() if m is None else m,copy.deepcopy(p.BUTTON_EXPECTED) if b is None else b,{'0x1408096f0'} if o is None else o)
 def test_natively_appended(self):
  m=self.run_it();self.assertTrue(m['common_button_internal_runtime']);self.assertEqual(m['functions'],[])
  d=p.declarations(m);self.assertNotIn('UFUNCTION',d);self.assertNotIn('UPROPERTY',d)
  self.assertEqual(d.count('TSharedPtr<SCommonButton>'),1);self.assertEqual(d.count('TSharedPtr<SBox>'),1)
  self.assertIn('FOnClicked NativeDoubleClickDelegate',d);self.assertIn('FSimpleDelegate NativeFocusReceivedDelegate',d)
 def test_owner_and_parent(self):
  for k,v in [('package','/Script/Other'),('parent','UObject')]:
   m=self.model();m[k]=v
   with self.subTest(k=k),self.assertRaises(ValueError):self.run_it(m=m)
  for o in [set(),{'0x0'},{'0x1408096f0','0x0'}]:
   with self.subTest(o=o),self.assertRaises(ValueError):self.run_it(o=o)
 def test_block_signatures_and_property_guards(self):
  for k in ['block_va','class_flags','dependency_fns','package_fn','base_fn','class_noregister_fn']:
   b=copy.deepcopy(p.BUTTON_EXPECTED);b[k]=None
   with self.subTest(k=k),self.assertRaises(ValueError):self.run_it(b=b)
  b=copy.deepcopy(p.BUTTON_EXPECTED);b['functions']=[{'name':'FakeNativeFunction'}]
  with self.assertRaises(ValueError):self.run_it(b=b)
  b=copy.deepcopy(p.BUTTON_EXPECTED);b['properties']=b['properties'][:-1]
  with self.assertRaises(ValueError):self.run_it(b=b)
  for i,f in enumerate(p.BUTTON_EXPECTED['properties']):
   for k in f:
    b=copy.deepcopy(p.BUTTON_EXPECTED);b['properties'][i][k]='mutated' if f[k] is None else None
    with self.subTest(i=i,k=k),self.assertRaises(ValueError):self.run_it(b=b)
 def test_collisions(self):
  for k,v in [('common_button_internal_runtime',True),('functions',[{'name':'Fake'}]),('helpers',[{}]),('fields',[{}])]:
   m=self.model();m[k]=v
   with self.subTest(k=k),self.assertRaises(ValueError):self.run_it(m=m)
 def test_header(self):
  m=p.header_prelude(self.run_it());self.assertIn('SBox',m['forward_decls']);self.assertIn('SCommonButton',m['forward_decls']);self.assertIn('Widgets/Input/SButton.h',m['includes']);self.assertIn('Input/Reply.h',m['includes'])
 def test_bodies(self):
  s=p.definitions(self.run_it())
  for name in ['RebuildWidget','ReleaseSlateResources','SlateHandleCommonClicked','SlateHandleCommonDoubleClicked','SlateHandleCommonPressed','SlateHandleCommonReleased','SlateHandleCommonFocusReceived','NativeSetInteractionEnabled','IsCommonInputSuspended']:
   self.assertEqual(s.count('UCommonButtonInternal::'+name+'('),1,name)
  self.assertLess(s.index('Super::ReleaseSlateResources'),s.index('NativeCommonButton.Reset()'));self.assertLess(s.index('NativeCommonButton.Reset()'),s.index('NativeButtonBox.Reset()'))
  self.assertIn('MyButton = NativeCommonButton;',s);self.assertIn('return NativeButtonBox.ToSharedRef();',s)
  self.assertIn('.ButtonStyle(&WidgetStyle)',s)
  self.assertNotIn('.PressMethod(',s);self.assertNotIn('.OnHovered(',s);self.assertNotIn('.OnUnhovered(',s)
  self.assertIn('static_cast<float>(MinWidth)',s);self.assertIn('static_cast<float>(MinHeight)',s)
  self.assertIn('NativeFocusReceivedDelegate.ExecuteIfBound();',s)
  s=s[s.index('void UCommonButtonInternal::NativeSetInteractionEnabled'):]
  self.assertLess(s.index('NativeCommonButton.Get()'),s.index('bInteractionEnabled = bEnabled;'))
 def test_other_class_passthrough(self):
  m={'name':'UOther'};self.assertIs(p.apply(m,{},set()),m)
if __name__=='__main__':unittest.main()
