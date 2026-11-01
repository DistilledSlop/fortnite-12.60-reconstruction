#!/usr/bin/env python3
"""Proof-bound dynamic texture-size fixture rejection and oracle tests."""
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import unittest
spec=importlib.util.spec_from_file_location('social_ingress',Path(__file__).with_name('export_social_border_ingress.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)

def constant():return dict(type=m.PREFIX+'Constant',value=[1.0,2.0,3.0,4.0],value_type=15)
def size():return dict(type=m.PREFIX+'TextureProperty',property=0,inputs=[dict(type=m.PREFIX+'Texture',texture=copy.deepcopy(m.SOURCE_TEXTURE))])
def parsed():
 a=dict(vector=dict(count=3,expressions=[constant(),constant(),size()]),scalar=dict(count=0,expressions=[]))
 return dict(uniform_expression_arrays=a)
class PortableTests(unittest.TestCase):
 def test_direct_root_remains_dynamic_and_source_zero_lanes(self):
  p=parsed();nodes,roots=m.flatten(p['uniform_expression_arrays']);node=nodes[roots['vector_roots'][2]]
  self.assertEqual(node,dict(type='TextureSize2D',texture_parameter_index=0))
  for dims,expected in [((32,32),(32,32,0,0)),((64,7),(64,7,0,0)),(None,(0,0,0,0))]:
   raw=m.evaluate(p,{},dims);self.assertEqual(raw[32:48],struct.pack('<4f',*expected))
 def test_nested_scalar_or_wrong_root_rejected(self):
  for kind in ('nested','scalar','wrong_vector'):
   with self.subTest(kind=kind),self.assertRaises(ValueError):
    p=parsed();a=p['uniform_expression_arrays']
    if kind=='nested':a['vector']['expressions'][2]=dict(type=m.PREFIX+'AppendVector',inputs=[size(),constant()],components_a=2)
    elif kind=='scalar':a['vector']['expressions'][2]=constant();a['scalar']=dict(count=1,expressions=[size()])
    else:a['vector']['expressions'][1]=size();a['vector']['expressions'][2]=constant()
    m.flatten(a)
 def test_wrong_property_texture_identity_or_sampler_rejected(self):
  for field,value in [('property',1),('type',m.PREFIX+'Texture'),('inputs',[])]:
   with self.subTest(field=field),self.assertRaises(ValueError):
    e=size();e[field]=value;m.check_size_root(e)
  for field,value in [('texture_index',1),('sampler_source',0),('layer_index',0),('virtual_texture',True)]:
   with self.subTest(field=field),self.assertRaises(ValueError):
    e=size();e['inputs'][0]['texture'][field]=value;m.check_size_root(e)
  e=size();e['inputs'][0]['type']=m.PREFIX+'TextureParameter'
  with self.assertRaises(ValueError):m.check_size_root(e)
  e=size();e['inputs'][0]['parameter']=dict(name='invented')
  with self.assertRaises(ValueError):m.check_size_root(e)
 def test_reference_order_and_duplicate_atlas(self):
  atlas=m.social.ATLAS_OBJECT;trace=[dict(position=3,kind='MaterialExpressionCurveAtlasRowParameter',atlas_object=atlas),dict(position=5,kind='MaterialExpressionMaterialFunctionCall',function=m.social.FUNCTION,texture_appends=[]),dict(position=9,kind='MaterialExpressionCurveAtlasRowParameter',atlas_object=atlas)]
  source=dict(source_context=dict(ordered_texture_references=None,candidate_atlas=atlas,trace=trace))
  self.assertEqual(m.derive_references(source),[m.ATLAS_PATH])
  for field,value in [('position',8),('atlas_object','wrong')]:
   with self.subTest(field=field),self.assertRaises(ValueError):
    bad=copy.deepcopy(source);bad['source_context']['trace'][2][field]=value;m.derive_references(bad)
  bad=copy.deepcopy(source);bad['source_context']['trace'][1]['texture_appends']=['unproven']
  with self.assertRaises(ValueError):m.derive_references(bad)
 def test_proof_outputs_and_pin_tampering_rejected(self):
  with tempfile.TemporaryDirectory() as temp:
   root=Path(temp);proof=root/'receipt.json';blob=root/'window.bin';blob.write_bytes(b'proof');entry=dict(path='window.bin',sha256=hashlib.sha256(b'proof').hexdigest(),bytes=5)
   data=dict(runtime_claim=False,original=dict(sha256='604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d'),outputs=[entry]);proof.write_text(json.dumps(data));m.verify_original_proof(data,proof)
   blob.write_bytes(b'wrong')
   with self.assertRaisesRegex(ValueError,'output snapshot'):m.verify_original_proof(data,proof)
   with self.assertRaisesRegex(ValueError,'hash differs'):m.checked_json(proof,'0'*64)
@unittest.skipUnless((m.ROOT/'social_border-ingress-3/bundle.json').exists(),'optional local original fixture unavailable')
class OriginalFixtureTests(unittest.TestCase):
 def test_original_fixture_reproduces_exact_root_and_shader_records(self):
  x=json.loads((m.ROOT/'social_border-ingress-3/bundle.json').read_text());f=x['runtime_fixture'];self.assertEqual(x['schema'],'fortnite.legacy-material-ingress.v5');self.assertEqual(len(x['original_shader_code']),19);self.assertEqual(len(f['shaders']),7)
  self.assertEqual(f['nodes'][f['vector_roots'][2]],dict(type='TextureSize2D',texture_parameter_index=0));self.assertEqual(f['ordered_texture_references'],[m.ATLAS_PATH]);self.assertEqual(f['texture_2d_parameters'][0]['sampler_source'],2)
  raw=struct.pack('<'+'f'*len(f['uniform_cases'][0]['numeric_values']),*f['uniform_cases'][0]['numeric_values']);self.assertEqual(raw[32:48],struct.pack('<4f',32,32,0,0));self.assertEqual(f['texture_size_contract']['missing_resource_result_bits'],['00000000']*4)
  for c in f['shader_binding_contracts']:
   if 'WorldClampSampler' in c['roles']:self.assertEqual(c['world_clamp_sampler_resource_index'],3)
 def test_original_numeric_cases_preserve_full_identity_isolation(self):
  f=json.loads((m.ROOT/'social_border-ingress-3/runtime-fixture.json').read_text());cases={x['name']:x for x in f['uniform_cases']}
  for name in ('unmatched_association','unmatched_index','unmatched_number'):self.assertEqual(cases[name]['numeric_sha256'],cases['defaults']['numeric_sha256'])
if __name__=='__main__':unittest.main()
