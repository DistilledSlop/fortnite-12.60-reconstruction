import copy, importlib.util, unittest
from pathlib import Path
HERE=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('policy',HERE/'text_block_style_policy.py'); p=importlib.util.module_from_spec(spec); spec.loader.exec_module(p)
C=p.CONTRACT
class Tests(unittest.TestCase):
 def fixture(self):
  d={'Style':'TSubclassOf<UCommonTextStyle> Style;', 'ScrollStyle':'TSubclassOf<UCommonTextScrollStyle> ScrollStyle;', 'bDisplayAllCaps':'bool bDisplayAllCaps;', 'bAutoCollapseWithEmptyText':'bool bAutoCollapseWithEmptyText;', 'MobileFontSizeMultiplier':'float MobileFontSizeMultiplier;'}
  m={'name':'UCommonTextBlock','package':'/Script/CommonUI','parent':'UTextBlock','functions':[],'members_decl_order':[{'name':k,'decl':v} for k,v in d.items()]}
  b=dict(copy.deepcopy(C['UCommonTextBlock']['identity']),properties=copy.deepcopy(C['UCommonTextBlock']['properties']),functions=copy.deepcopy(C['UCommonTextBlock']['functions'])+[copy.deepcopy(C['text_case_function'])])
  return m,b,{'0x140812e80'},{'SetStyle':copy.deepcopy(C['exec']),'SetTextCase':copy.deepcopy(C['text_case_exec'])}
 def test_valid_model(self):
  m,b,o,e=self.fixture();p.apply(m,b,o,e); f=m['functions'][0];self.assertEqual((f['name'],f['args'],f['specifiers']),('SetStyle',['TSubclassOf<UCommonTextStyle> InStyle'],['BlueprintCallable']));self.assertIn('virtual void SynchronizeProperties() override',p.declarations(m)); self.assertEqual((m['functions'][1]['name'],m['functions'][1]['args'],m['functions'][1]['specifiers']),('SetTextCase',['bool bUseAllCaps'],['BlueprintCallable']))
 def test_case_body(self):
  m,b,o,e=self.fixture();p.apply(m,b,o,e)
  body=p.definitions(m).split('void UCommonTextBlock::SetTextCase(bool bUseAllCaps)',1)[1].split('void UCommonTextBlock::SynchronizeProperties()',1)[0]
  self.assertEqual(body.strip(),'{\n    TextTransformPolicy = bUseAllCaps ? ETextTransformPolicy::ToUpper : ETextTransformPolicy::None;\n    SynchronizeProperties();\n}')
 def test_drift(self):
  for what in ('signature','duplicate_style','case_signature','duplicate_case','exec','case_exec','owner','decl','property','collision','case_collision','marker'):
   m,b,o,e=self.fixture()
   if what=='signature':b['functions'][0]['candidates'][2]['params'][0]['type_refs']['meta_class']='bad'
   if what=='duplicate_style':b['functions'].append(copy.deepcopy(b['functions'][0]))
   if what=='case_signature':b['functions'][1]['params'][0]['name']='bad'
   if what=='duplicate_case':b['functions'].append(copy.deepcopy(b['functions'][1]))
   if what=='case_exec':e['SetTextCase']['exec_addr']='bad'
   if what=='case_collision':m['functions'].append({'name':'SetTextCase'})
   if what=='exec':e['SetStyle']['exec_addr']='bad'
   if what=='owner':o.clear()
   if what=='decl':m['members_decl_order'][0]['decl']='bad'
   if what=='property':b['properties'][0]['property_flags']='bad'
   if what=='collision':m['functions'].append({'name':'SynchronizeProperties'})
   if what=='marker':m['common_text_block_style_runtime']=True
   with self.subTest(what=what),self.assertRaises(ValueError):p.apply(m,b,o,e)
 def test_class_guards(self):
  blocks={v['identity']['block_va']:dict(copy.deepcopy(v['identity']),properties=copy.deepcopy(v['properties'])) for k,v in C.items() if k in ('UCommonTextBlock','UCommonTextStyle')}; names={k:v['identity']['block_va'] for k,v in C.items() if k in ('UCommonTextBlock','UCommonTextStyle')}
  names['UTextBlock']='0x146844b60';blocks[names['UTextBlock']]={'properties':[copy.deepcopy(C['text_transform_property'])]}
  p.validate_classes(blocks,names)
  for name in names:
   b=copy.deepcopy(blocks);b[names[name]]['properties'][0]['offset']=-1
   with self.subTest(name=name),self.assertRaises(ValueError):p.validate_classes(b,names)
if __name__=='__main__': unittest.main()
