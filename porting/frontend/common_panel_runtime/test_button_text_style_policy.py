import copy
import importlib.util
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('button_text_style_policy', HERE/'button_text_style_policy.py')
p = importlib.util.module_from_spec(spec); spec.loader.exec_module(p)
C = p.CONTRACT

class PolicyTests(unittest.TestCase):
 def fixture(self):
  m = {'name':'UCommonButton','package':'/Script/CommonUI','functions':[], 'members_decl_order':[{'name':'Style','decl':'TSubclassOf<UCommonButtonStyle> Style;'}]}
  b = {'block_va':'0x145a01210','functions':copy.deepcopy(C['UCommonButton']['functions']),'properties':copy.deepcopy(C['UCommonButton']['properties'])}
  return m,b,{'0x140805cb0'},copy.deepcopy(C['exec'])
 def test_apply_and_unrelated(self):
  m,b,o,e = self.fixture(); p.apply(m,b,o,e)
  self.assertEqual([(f['name'],f['ret'],f['const'],f['runtime_implementation']) for f in m['functions']], [('GetSelected','bool',True,True),('GetCurrentTextStyleClass','TSubclassOf<UCommonTextStyle>',True,True)])
  other={'name':'Other','functions':[]}; self.assertIs(p.apply(other,{},set(),None),other); self.assertEqual(other,{'name':'Other','functions':[]})
  self.assertIn('bNativeSelected = false',p.declarations(m)); self.assertIn('CommonButtonTextStyleCore::Get',p.definitions(m))
 def test_signature_exec_and_collisions(self):
  for name in p.NAMES:
   for kind in ('missing','duplicate','signature','exec'):
    m,b,o,e=self.fixture(); f=next(x for x in b['functions'] if x['name']==name)
    if kind=='missing': b['functions'].remove(f)
    elif kind=='duplicate': b['functions'].append(copy.deepcopy(f))
    elif kind=='signature': f['flags']=[]
    else: e[name]['exec_addr']='0x0'
    with self.subTest(name=name,kind=kind), self.assertRaises(ValueError): p.apply(m,b,o,e)
  for group in ('functions','members_table_order','members_decl_order','fields','helpers','static_helpers'):
   for name in (*p.NAMES,'bNativeSelected','NativeInternalButton','IsHovered','FCommonButtonTextStyleProbeAccess'):
    m,b,o,e=self.fixture();m.setdefault(group,[]).append({'name':name})
    with self.subTest(group=group,name=name), self.assertRaises(ValueError):p.apply(m,b,o,e)
  for kind in ('owner','style','decl','marker'):
   m,b,o,e=self.fixture()
   if kind=='owner':o.clear()
   elif kind=='style':b['properties'][0]['type_refs']['meta_class']='0x0'
   elif kind=='decl':m['members_decl_order'][0]['decl']='bad'
   else:m['common_button_text_style_runtime']=True
   with self.subTest(kind=kind), self.assertRaises(ValueError):p.apply(m,b,o,e)
 def test_class_metadata_guards(self):
  blocks={v['identity']['block_va']:dict(copy.deepcopy(v['identity']), properties=copy.deepcopy(v['properties'])) for n,v in C.items() if n!='exec'}
  names={n:v['identity']['block_va'] for n,v in C.items() if n!='exec'}
  p.validate_classes(blocks,names)
  for name in names:
   changed=copy.deepcopy(blocks);changed[names[name]]['class_noregister_fn']='0x0'
   with self.subTest(name=name), self.assertRaises(ValueError):p.validate_classes(changed,names)
  for prop in C['UCommonButtonStyle']['properties']:
   changed=copy.deepcopy(blocks);row=next(x for x in changed[names['UCommonButtonStyle']]['properties'] if x['name']==prop['name']);row['offset']=0
   with self.subTest(prop=prop['name']), self.assertRaises(ValueError):p.validate_classes(changed,names)
  bad=dict(names);bad['UCommonTextStyle']='0x0'
  with self.assertRaises(ValueError):p.validate_classes(blocks,bad)
 def test_independent_header_gate(self):
  with tempfile.TemporaryDirectory() as d:
   path=Path(d)/'CommonUI/Generated';path.mkdir(parents=True)
   p.emit_headers(d,{'UCommonButton'});self.assertEqual((path/'CommonButtonTextStyleCore.h').read_text(),(HERE/'CommonButtonTextStyleCore.h').read_text())
   (path/'CommonButtonTextStyleCore.h').unlink();p.emit_headers(d,{'UCommonActivatableWidget'});self.assertFalse((path/'CommonButtonTextStyleCore.h').exists())
if __name__=='__main__': unittest.main()
