import copy, importlib.util, unittest
from pathlib import Path
HERE=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('policy',HERE/'text_block_style_policy.py'); p=importlib.util.module_from_spec(spec); spec.loader.exec_module(p)
C=p.CONTRACT
class Tests(unittest.TestCase):
 def fixture(self):
  d={'Style':'TSubclassOf<UCommonTextStyle> Style;', 'ScrollStyle':'TSubclassOf<UCommonTextScrollStyle> ScrollStyle;', 'bDisplayAllCaps':'bool bDisplayAllCaps;', 'bAutoCollapseWithEmptyText':'bool bAutoCollapseWithEmptyText;', 'MobileFontSizeMultiplier':'float MobileFontSizeMultiplier;'}
  m={'name':'UCommonTextBlock','package':'/Script/CommonUI','parent':'UTextBlock','functions':[],'members_decl_order':[{'name':k,'decl':v} for k,v in d.items()]}
  b=dict(copy.deepcopy(C['UCommonTextBlock']['identity']),properties=copy.deepcopy(C['UCommonTextBlock']['properties']),functions=copy.deepcopy(C['UCommonTextBlock']['functions']))
  return m,b,{'0x140812e80'},copy.deepcopy(C['exec'])
 def test_valid_model(self):
  m,b,o,e=self.fixture();p.apply(m,b,o,{'SetStyle':e}); f=m['functions'][0];self.assertEqual((f['name'],f['args'],f['specifiers']),('SetStyle',['TSubclassOf<UCommonTextStyle> InStyle'],['BlueprintCallable']));self.assertIn('virtual void SynchronizeProperties() override',p.declarations(m))
 def test_drift(self):
  for what in ('signature','exec','owner','decl','property','collision','marker'):
   m,b,o,e=self.fixture()
   if what=='signature':b['functions'][0]['candidates'][2]['params'][0]['type_refs']['meta_class']='bad'
   if what=='exec':e['exec_addr']='bad'
   if what=='owner':o.clear()
   if what=='decl':m['members_decl_order'][0]['decl']='bad'
   if what=='property':b['properties'][0]['property_flags']='bad'
   if what=='collision':m['functions'].append({'name':'SynchronizeProperties'})
   if what=='marker':m['common_text_block_style_runtime']=True
   with self.subTest(what=what),self.assertRaises(ValueError):p.apply(m,b,o,{'SetStyle':e})
 def test_class_guards(self):
  blocks={v['identity']['block_va']:dict(copy.deepcopy(v['identity']),properties=copy.deepcopy(v['properties'])) for k,v in C.items() if k!='exec'}; names={k:v['identity']['block_va'] for k,v in C.items() if k!='exec'}
  p.validate_classes(blocks,names)
  for name in names:
   b=copy.deepcopy(blocks);b[names[name]]['properties'][0]['offset']=-1
   with self.subTest(name=name),self.assertRaises(ValueError):p.validate_classes(b,names)
if __name__=='__main__': unittest.main()
