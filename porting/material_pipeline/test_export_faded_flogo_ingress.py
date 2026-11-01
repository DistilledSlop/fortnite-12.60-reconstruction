#!/usr/bin/env python3
"""Fail closed on wrong named-texture, parent, instance and shader contracts."""
import copy
import importlib.util
import json
from pathlib import Path
import unittest

spec=importlib.util.spec_from_file_location('flogo',Path(__file__).with_name('export_faded_flogo_ingress.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
I=m.GENERATED/'frontend-probes/frontend-material-inventory'; F=m.ROOT/'flogo-functions-1';T=m.ROOT/'flogo-texture-1/texture-report.json';S=m.ROOT/'flogo-shaders-1';stem=m.PACKAGE.replace('/','_');parent=m.PARENT_PACKAGE.replace('/','_')

class PortableTests(unittest.TestCase):
 def arrays(self, expr):
  arrays={n:dict(count=0,expressions=[]) for n in m.bundle.decoder.ARRAYS};arrays['texture_2d']=dict(count=1,expressions=[expr])
  return dict(uniform_expression_arrays=arrays,virtual_texture_stacks=dict(count=0),parameter_collections=dict(count=0))
 def test_named_parameter_exact_identity_and_texture_contract(self):
  e=dict(offset=881,end=918,type='FMaterialUniformExpressionTextureParameter',parameter=dict(name=dict(name='TextureMask',number=0),association=2,index=-1),texture=copy.deepcopy(m.BASE_TEXTURE))
  for key,value in [('type','FMaterialUniformExpressionTexture'),('parameter',None),('offset',880)]:
   with self.subTest(key=key),self.assertRaisesRegex(ValueError,'named TextureParameter'):
    d=copy.deepcopy(e);d[key]=value;m.validate_arrays(self.arrays(d))
  for key,value in [('texture_index',1),('sampler_source',1),('layer_index',0),('virtual_texture',True)]:
   with self.subTest(key=key),self.assertRaisesRegex(ValueError,'named TextureParameter'):
    d=copy.deepcopy(e);d['texture'][key]=value;m.validate_arrays(self.arrays(d))
 def test_wrong_resource_mapping_rejected(self):
  r=dict(frequency=3,material_uniform_buffer=dict(base_index=1,bound=True),uniform_parameters=[])
  d=dict(set=0,binding=0,d3d_register=dict(kind='srv',space=0,index=0),vulkan_descriptor_type=2,image_type=copy.deepcopy(m.SAMPLED_IMAGE),resource_table_entries=[dict(uniform_buffer=1,resource=2,bind=0,table='texture')])
  with self.assertRaisesRegex(ValueError,'resource-table'):m.typed_contract(r,dict(descriptors=[d]))

@unittest.skipUnless((I/(stem+'.decoded.json')).exists() and T.exists() and (S/'reflection.json').exists(),'optional original evidence absent')
class EvidenceTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  load=lambda p:json.loads(p.read_text());cls.parsed=load(I/(stem+'.decoded.json'));cls.owner=load(I/(parent+'.json'));cls.exports=load(I/(parent+'.export-map.json'));cls.functions=[load(p) for p in F.glob('*.json')];cls.meta=load(T);cls.source=m.EXTRACTED/(m.TEXTURE_PACKAGE+'.uexp');cls.header=cls.source.with_suffix('.uasset').stat().st_size
 def test_parent_indexed_default_and_function_closure(self):
  c=m.prove_context(self.owner,self.exports,self.functions);self.assertEqual(c['ordered_texture_references'],[m.TEXTURE_PATH]);self.assertEqual(len(c['trace']),2);self.assertEqual(c['trace'][0]['function_order'],[m.DENSITY_PATH,m.FUNCTION_PATH])
 def test_parent_identity_and_missing_function_rejected(self):
  o=copy.deepcopy(self.owner);e=next(x for x in o['expressionExports'] if x['exportType']=='MaterialExpressionTextureSampleParameter2D');next(p for p in e['properties'] if p['name']=='ParameterName')['value']='Wrong (NameProperty)'
  with self.assertRaisesRegex(ValueError,'texture default'):m.prove_context(o,self.exports,self.functions)
  with self.assertRaisesRegex(ValueError,'dependency'):m.prove_context(self.owner,self.exports,[x for x in self.functions if x['path']!=m.DENSITY_PATH])
 def test_exact_dxt5_frame_and_rejections(self):
  result=m.validate_texture(self.meta,self.source.read_bytes(),self.header);self.assertEqual((result['logical_width'],result['logical_height'],result['mip_count'],result['pixel_format']),(1024,256,1,'PF_DXT5'));self.assertEqual(result['rgba_sha256'],'cdab3ce448c5555adebdf2de8a3423f234107a22d2636806b9aca58b6dffe842')
  for key,val in [('srgb',False),('addressX','TA_Wrap'),('pixelFormat','PF_DXT1')]:
   with self.subTest(key=key),self.assertRaises(ValueError):
    meta=copy.deepcopy(self.meta);meta[key]=val;m.validate_texture(meta,self.source.read_bytes(),self.header)
  raw=bytearray(self.source.read_bytes());i=raw.index(b'PF_DXT5\0')+len(b'PF_DXT5\0')+8;raw[i]^=1
  with self.assertRaisesRegex(ValueError,'inline mip'):m.validate_texture(self.meta,bytes(raw),self.header)
 def test_all_original_slate_contracts_and_cases(self):
  nodes,roots=m.validate_arrays(self.parsed);self.assertEqual((len(roots['vector_roots']),len(roots['scalar_roots'])),(7,14));ref=json.loads((S/'reflection.json').read_text());lookup={x['shader_index']:x for x in ref['records']}
  self.assertEqual(set(lookup),{2480,2481,30663,30664,30665,30666,30667})
  for r in self.parsed['material_shaders']['records']:m.typed_contract(r,lookup[r['library_association']['index']])
  cases,_=m.uniform_cases(self.parsed,nodes,[]);cases={c['name']:c for c in cases}
  for n in ['unmatched_association','unmatched_index','unmatched_number']:self.assertEqual(cases[n]['numeric_sha256'],cases['defaults']['numeric_sha256'])
 def test_bundle_retains_original_owner_and_override(self):
  path=m.ROOT/'flogo-ingress-3/bundle.json'
  if not path.exists():self.skipTest('optional bundle absent')
  x=json.loads(path.read_text());f=x['runtime_fixture'];self.assertEqual(x['owner_path'],m.OWNER);self.assertEqual(x['parent_path'],m.PARENT);self.assertEqual(x['shader_identity'],m.PARENT);self.assertEqual(len(x['original_shader_code']),19);self.assertEqual(len(f['shaders']),7);self.assertEqual(len(f['owner_parameter_expectations']),14)
  t=x['original_instance_properties']['TextureParameterValues'];self.assertEqual(t[0]['ParameterInfo'],dict(Name='TextureMask',Association='GlobalParameter',Index=-1));self.assertEqual(t[0]['ParameterValue']['ObjectPath'],m.TEXTURE_PACKAGE+'.0')
if __name__=='__main__':unittest.main()
