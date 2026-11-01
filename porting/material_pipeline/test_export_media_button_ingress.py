#!/usr/bin/env python3
"""Source and sampler rejection checks for original Media button ingress."""
import copy,importlib.util,json
from pathlib import Path
import unittest
spec=importlib.util.spec_from_file_location('media',Path(__file__).with_name('export_media_button_ingress.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
I=m.GENERATED/'frontend-probes/frontend-material-inventory-textures'; STEM=m.PACKAGE.replace('/','_');F=m.ROOT/'media_button-functions-merged-1';T=m.ROOT/'media-button-texture-1/texture-report.json';S=m.ROOT/'media-button-shaders-1'
class Portable(unittest.TestCase):
 def arrays(self,e):
  a={n:dict(count=0,expressions=[]) for n in m.bundle.decoder.ARRAYS};a['texture_2d']=dict(count=1,expressions=[e]);return dict(uniform_expression_arrays=a,virtual_texture_stacks=dict(count=0),parameter_collections=dict(count=0))
 def test_original_plain_texture_identity_and_sampler(self):
  e=dict(offset=341,end=365,type='FMaterialUniformExpressionTexture',texture=copy.deepcopy(m.BASE_TEXTURE))
  for k,v in [('type','FMaterialUniformExpressionTextureParameter'),('parameter',dict(name='invented')),('offset',342)]:
   with self.subTest(field=k),self.assertRaises(ValueError):d=copy.deepcopy(e);d[k]=v;m.validate_arrays(self.arrays(d))
  for k,v in [('texture_index',1),('sampler_source',0),('virtual_texture',True),('layer_index',0)]:
   with self.subTest(field=k),self.assertRaises(ValueError):d=copy.deepcopy(e);d['texture'][k]=v;m.validate_arrays(self.arrays(d))
 def test_material_sampler_must_be_world_clamp_slot_three(self):
  record=dict(frequency=3,material_uniform_buffer=dict(base_index=1,bound=True),uniform_parameters=[])
  desc=dict(set=0,binding=0,d3d_register=dict(kind='sampler',space=0,index=0),vulkan_descriptor_type=0,resource_table_entries=[dict(uniform_buffer=1,resource=1,bind=0,table='sampler')])
  with self.assertRaisesRegex(ValueError,'resource-table'):m.typed_contract(record,dict(descriptors=[desc]))
@unittest.skipUnless((I/(STEM+'.decoded.json')).exists() and T.exists() and (S/'reflection.json').exists(),'optional original evidence unavailable')
class Source(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  load=lambda p:json.loads(p.read_text());cls.parsed=load(I/(STEM+'.decoded.json'));cls.owner=load(I/(STEM+'.json'));cls.exports=load(I/(STEM+'.export-map.json'));cls.functions=[load(p) for p in F.glob('*.json')];cls.meta=load(T);cls.source=m.EXTRACTED/(m.TEXTURE_PACKAGE+'.uexp');cls.header=cls.source.with_suffix('.uasset').stat().st_size
 def test_ordered_reference_and_complete_function_closure(self):
  c=m.prove_context(self.owner,self.exports,self.functions);self.assertEqual(c['ordered_texture_references'],[m.TEXTURE_PATH]);self.assertEqual(sum('texture_object' in x for x in c['trace']),3);self.assertEqual(sum('function_order' in x for x in c['trace']),11)
 def test_texture_or_function_tampering_rejected(self):
  x=copy.deepcopy(self.owner);e=next(e for e in x['expressionExports'] if e['exportType']=='MaterialExpressionTextureSample');next(y for y in e['properties'] if y['name']=='Texture')['value']='wrong'
  with self.assertRaises(ValueError):m.prove_context(x,self.exports,self.functions)
  with self.assertRaises(ValueError):m.prove_context(self.owner,self.exports,self.functions[:-1])
  functions=copy.deepcopy(self.functions)
  call=next(o for f in functions for o in f['objects'] if o['Type']=='MaterialExpressionMaterialFunctionCall' and o['Properties'].get('MaterialFunction'))
  call['Properties']['MaterialFunction']['ObjectPath']+='9'
  with self.assertRaises(ValueError):m.prove_context(self.owner,self.exports,functions)
 def test_source_frame_and_metadata(self):
  out=m.validate_texture(self.meta,self.source.read_bytes(),self.header);self.assertEqual((out['logical_width'],out['logical_height'],out['stored_bulk_offset']),(128,128,1113));self.assertEqual(out['raw_bgra_sha256'],'b842bc2606ab98a0f5c359e5e8b1b423a9af00d7544eed41900a7c7156b0ffc3');self.assertEqual(out['rgba_sha256'],'2778f9ccfc8816a04839d2c12d595f9bce232e018eca9730e97d2071aed35610')
  for k,v in [('srgb',False),('addressX','TA_Clamp'),('pixelFormat','PF_DXT1')]:
   with self.subTest(field=k),self.assertRaises(ValueError):x=copy.deepcopy(self.meta);x[k]=v;m.validate_texture(x,self.source.read_bytes(),self.header)
 def test_all_seven_original_shader_contracts(self):
  report=json.loads((S/'reflection.json').read_text());table={r['shader_index']:r for r in report['records']};self.assertEqual(set(table),{2480,2481,111433,111434,111435,111436,111437})
  for r in self.parsed['material_shaders']['records']:
   c=m.typed_contract(r,table[r['library_association']['index']]);
   if r['frequency']==3:self.assertIn('WorldClampSampler',c['roles'])
 def test_fixture_preserves_source_and_identity_controls(self):
  p=m.ROOT/'media_button-ingress-1/bundle.json'
  if not p.exists():self.skipTest('optional source bundle absent')
  x=json.loads(p.read_text());f=x['runtime_fixture'];self.assertEqual(len(x['original_shader_code']),19);self.assertEqual(len(f['shaders']),7);self.assertEqual(f['texture_2d_parameters'][0]['sampler_source'],2);self.assertIsNone(f['texture_2d_parameters'][0]['parameter']);self.assertEqual(f['expected_layout'],m.LAYOUT)
  cases={x['name']:x for x in f['uniform_cases']}
  for name in ('unmatched_association','unmatched_index','unmatched_number'):self.assertEqual(cases[name]['numeric_sha256'],cases['defaults']['numeric_sha256'])
if __name__=='__main__':unittest.main()
