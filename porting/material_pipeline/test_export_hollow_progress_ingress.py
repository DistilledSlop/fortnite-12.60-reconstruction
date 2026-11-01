#!/usr/bin/env python3
"""Reject unsupported hollow-material identities, resources and texture layouts."""
import copy
import importlib.util
import json
from pathlib import Path
import unittest

spec=importlib.util.spec_from_file_location('hollow',Path(__file__).with_name('export_hollow_progress_ingress.py'))
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
I=m.GENERATED/'frontend-probes/frontend-material-inventory-textures'; STEM=m.PACKAGE.replace('/','_')
F=m.ROOT/'hollow-function-1';T=m.ROOT/'hollow-texture-1/texture-report.json';S=m.ROOT/'hollow-shaders-1'

class PortableTests(unittest.TestCase):
 def arrays(self, expr):
  arrays={n:dict(count=0,expressions=[]) for n in m.bundle.decoder.ARRAYS};arrays['texture_2d']=dict(count=1,expressions=[expr])
  return dict(uniform_expression_arrays=arrays,virtual_texture_stacks=dict(count=0),parameter_collections=dict(count=0))
 def test_plain_texture_rejects_changed_identity_and_contract(self):
  e=dict(offset=552,end=576,type='FMaterialUniformExpressionTexture',texture=copy.deepcopy(m.BASE_TEXTURE))
  for key,value in [('type','FMaterialUniformExpressionTextureParameter'),('parameter',{'name':'Icon'}),('offset',553)]:
   with self.subTest(key=key),self.assertRaisesRegex(ValueError,'regular Texture'):
    d=copy.deepcopy(e);d[key]=value;m.validate_arrays(self.arrays(d))
  for key,value in [('texture_index',1),('sampler_source',1),('layer_index',0),('virtual_texture',True)]:
   with self.subTest(key=key),self.assertRaisesRegex(ValueError,'regular Texture'):
    d=copy.deepcopy(e);d['texture'][key]=value;m.validate_arrays(self.arrays(d))
 def test_bad_resource_binding_rejected(self):
  record=dict(frequency=3,material_uniform_buffer=dict(base_index=1,bound=True),uniform_parameters=[])
  d=dict(set=0,binding=0,d3d_register=dict(kind='srv',space=0,index=0),vulkan_descriptor_type=2,image_type=copy.deepcopy(m.SAMPLED_IMAGE),resource_table_entries=[dict(uniform_buffer=1,resource=2,bind=0,table='texture')])
  with self.assertRaisesRegex(ValueError,'resource-table'):m.typed_contract(record,dict(descriptors=[d]))

@unittest.skipUnless((I/(STEM+'.decoded.json')).exists() and T.exists() and (S/'reflection.json').exists(),'optional source evidence unavailable')
class EvidenceTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  load=lambda p:json.loads(p.read_text())
  cls.parsed=load(I/(STEM+'.decoded.json'));cls.owner=load(I/(STEM+'.json'));cls.exports=load(I/(STEM+'.export-map.json'))
  cls.functions=[load(p) for p in F.glob('*.json')];cls.meta=load(T);cls.source=m.EXTRACTED/(m.TEXTURE_PACKAGE+'.uexp');cls.header=cls.source.with_suffix('.uasset').stat().st_size
 def test_original_ordered_reference_and_full_function_closure(self):
  c=m.prove_context(self.owner,self.exports,self.functions);self.assertEqual(c['ordered_texture_references'],[m.TEXTURE_PATH]);self.assertEqual(len(c['trace']),8)
  self.assertEqual(sum('texture_object' in r for r in c['trace']),4);self.assertEqual(sum('function_order' in r for r in c['trace']),4)
 def test_source_references_fail_closed(self):
  owner=copy.deepcopy(self.owner);e=next(e for e in owner['expressionExports'] if e['exportType']=='MaterialExpressionTextureSample');next(p for p in e['properties'] if p['name']=='Texture')['value']="Texture2D'Wrong.Wrong' (ObjectProperty)"
  with self.assertRaisesRegex(ValueError,'texture default'):m.prove_context(owner,self.exports,self.functions)
  with self.assertRaisesRegex(ValueError,'dependency'):m.prove_context(self.owner,self.exports,[])
  functions=copy.deepcopy(self.functions);functions[0]['objects'][0]['Properties']['FunctionExpressions'][0]={'ObjectPath':'x.0'}
  with self.assertRaises(ValueError):m.prove_context(self.owner,self.exports,functions)
 def test_all_eight_original_mips_and_pixels(self):
  out=m.validate_texture(self.meta,self.source.read_bytes(),self.header)
  self.assertEqual([p['width'] for p in out['mip_receipts']],[128,64,32,16,8,4,2,1]);self.assertEqual(out['rgba_sha256'],'ded536db457df6d7c718245e8602cc66d6c9f68b278a9155d54ef812f49d0ef6')
 def test_texture_metadata_and_inline_frame_mutations_rejected(self):
  for key,value in [('srgb',True),('addressX','TA_Wrap'),('width',64)]:
   with self.subTest(key=key),self.assertRaises(ValueError):
    meta=copy.deepcopy(self.meta);meta[key]=value;m.validate_texture(meta,self.source.read_bytes(),self.header)
  for at in (376+4,376+8+4,408+65536):
   with self.subTest(offset=at),self.assertRaises((ValueError,IndexError)):
    raw=bytearray(self.source.read_bytes());raw[at]^=1;m.validate_texture(self.meta,bytes(raw),self.header)
 def test_all_source_shaders_and_numeric_cases(self):
  nodes,roots=m.validate_arrays(self.parsed);self.assertEqual((len(roots['vector_roots']),len(roots['scalar_roots'])),(4,7))
  cases={x['name']:x for x in m.uniform_cases(self.parsed,nodes)}
  for key in ['unmatched_association','unmatched_index','unmatched_number']:self.assertEqual(cases[key]['numeric_sha256'],cases['defaults']['numeric_sha256'])
  reflected=json.loads((S/'reflection.json').read_text()); table={r['shader_index']:r for r in reflected['records']}
  self.assertEqual(set(table),{2480,2481,7120,7121,7122,7123,7124})
  for record in self.parsed['material_shaders']['records']:m.typed_contract(record,table[record['library_association']['index']])
 def test_pinned_bundle_full_contract(self):
  p=m.ROOT/'hollow-ingress-3/bundle.json'
  if not p.is_file():self.skipTest('optional locally generated bundle absent')
  d=json.loads(p.read_text());self.assertEqual(d['provenance']['resource_sha1'],'591d3a158913e24dbcc0196440a8c3c40efd24aa');self.assertEqual(len(d['original_shader_code']),19);self.assertEqual(len(d['runtime_fixture']['shaders']),7)
  tex=d['runtime_fixture']['texture_2d_parameters'][0];self.assertEqual(tex['expression_kind'],'Texture');self.assertIsNone(tex['parameter']);self.assertEqual(tex['texture_index'],0)
if __name__=='__main__':unittest.main()
