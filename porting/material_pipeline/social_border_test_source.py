#!/usr/bin/env python3
"""Rejection tests for source-only Social border evidence."""
import copy
import importlib.util
import json
from pathlib import Path
import struct
import unittest
spec=importlib.util.spec_from_file_location('social',Path(__file__).with_name('social_border_source.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
class Portable(unittest.TestCase):
 def parsed(self):
  child=dict(type='FMaterialUniformExpressionTexture',texture=dict(texture_index=0,layer_index=-1,page_table_layer_index=-1,sampler_source=2,virtual_texture=False))
  a={n:dict(count=0,expressions=[]) for n in m.bundle.decoder.ARRAYS};a['vector']=dict(count=5,expressions=[{}, {},dict(type='FMaterialUniformExpressionTextureProperty',property=0,inputs=[copy.deepcopy(child)]),{},{}]);a['scalar']=dict(count=9,expressions=[]);a['texture_2d']=dict(count=1,expressions=[child])
  return dict(uniform_expression_arrays=a,virtual_texture_stacks=dict(count=0),parameter_collections=dict(count=0),shader_record_count=19,material_shaders=dict(records=[dict(debug_uniform_buffer_layout=copy.deepcopy(m.LAYOUT)) for _ in range(7)]))
 def test_no_texture_lanes_inferred(self):
  x=m.check_arrays(self.parsed());self.assertIn('unresolved',x['lane_semantics']);self.assertEqual(x['child_texture']['sampler_source'],2)
 def test_changed_property_texture_or_sampler_rejected(self):
  for field,value in [('property',1),('type','FMaterialUniformExpressionTexture')]:
   with self.subTest(field=field),self.assertRaises(ValueError):
    p=self.parsed();p['uniform_expression_arrays']['vector']['expressions'][2][field]=value;m.check_arrays(p)
  for field,value in [('texture_index',1),('sampler_source',0),('virtual_texture',True)]:
   with self.subTest(field=field),self.assertRaises(ValueError):
    p=self.parsed();p['uniform_expression_arrays']['vector']['expressions'][2]['inputs'][0]['texture'][field]=value;m.check_arrays(p)
 def test_unproven_named_identity_rejected(self):
  p=self.parsed();p['uniform_expression_arrays']['texture_2d']['expressions'][0]['type']='FMaterialUniformExpressionTextureParameter'
  with self.assertRaisesRegex(ValueError,'regular texture'):m.check_arrays(p)
 def test_clamp_world_sampler_slot_is_not_asset_sampler(self):
  record=dict(frequency=3,material_uniform_buffer=dict(base_index=1,bound=True));d=dict(d3d_register=dict(kind='sampler',space=0,index=0),set=0,vulkan_descriptor_type=0,resource_table_entries=[dict(uniform_buffer=1,resource=1,bind=0,table='sampler')])
  with self.assertRaisesRegex(ValueError,'resource table'):m.descriptor_contract(record,dict(descriptors=[d]))
 def test_atlas_native_half_float_metadata_and_mutations(self):
  meta=dict(schema='fortnite.social-border-atlas-metadata.v1',package=m.ATLAS,name='CA_SocialPanel',export_type='CurveLinearColorAtlas',pixelFormat='PF_FloatRGBA',srgb=True,filter='TF_Default',lodGroup='TEXTUREGROUP_World',addressX='TA_Wrap',addressY='TA_Wrap',firstMip=0,mips=[dict(width=32,height=32,depth=1,bulkBytes=8192)])
  marker=b'PF_FloatRGBA\0';header=2802;prefix=struct.pack('<IIIi',32,32,1,len(marker));cursor=len(prefix)+len(marker)+32;raw=prefix+marker+struct.pack('<iiiiiiq',0,1,1,0x48,8192,8192,header+cursor)+bytes(8192)+struct.pack('<iiii',32,32,1,0)
  out=m.atlas_frame(meta,raw,header);self.assertEqual(out['raw_bytes'],8192);self.assertIn('unconverted',out['raw_encoding'])
  for field,value in [('pixelFormat','PF_B8G8R8A8'),('srgb',False),('addressX','TA_Clamp')]:
   with self.subTest(field=field),self.assertRaises(ValueError):
    other=copy.deepcopy(meta);other[field]=value;m.atlas_frame(other,raw,header)
  bad=bytearray(raw);bad[-4]=1
  with self.assertRaises(ValueError):m.atlas_frame(meta,bytes(bad),header)

@unittest.skipUnless((m.ROOT/'social_border-source-1/source.json').is_file(),'optional local source evidence unavailable')
class SourceEvidence(unittest.TestCase):
 def setUp(self):self.e=json.loads((m.ROOT/'social_border-source-1/source.json').read_text())
 def test_original_resource_preserved_without_runtime_claims(self):
  self.assertEqual(self.e['resource_sha1'],'c505c4a9977115634e87809ccd0b5e981bfc417d');self.assertEqual(len(self.e['original_shader_code']),19);self.assertEqual(len(self.e['shader_records']),7)
  self.assertIsNone(self.e['runtime_fixture']);self.assertIsNone(self.e['numeric_values']);self.assertIsNone(self.e['source_context']['ordered_texture_references'])
 def test_parent_source_atlas_or_function_changes_rejected(self):
  root=m.GENERATED/'frontend-probes/frontend-material-inventory-textures';stem=m.PACKAGE.replace('/','_');owner=json.loads((root/(stem+'.json')).read_text());exports=json.loads((root/(stem+'.export-map.json')).read_text());fn=json.loads((m.ROOT/'social_border-function-1/Engine_Content_Functions_UserInterface_GetUserInterfaceUV.json').read_text())
  self.assertEqual(m.source_context(owner,exports,fn)['candidate_atlas'],m.ATLAS_OBJECT)
  changed=copy.deepcopy(owner);e=next(e for e in changed['expressionExports'] if e['exportType']=='MaterialExpressionCurveAtlasRowParameter');next(p for p in e['properties'] if p['name']=='Atlas')['value']='wrong'
  with self.assertRaisesRegex(ValueError,'atlas object'):m.source_context(changed,exports,fn)
  changed=copy.deepcopy(fn);changed['objects'][0]['Properties']['FunctionExpressions'][0]={'ObjectPath':'unproven.0'}
  with self.assertRaisesRegex(ValueError,'unproven expression'):m.source_context(owner,exports,changed)
if __name__=='__main__':unittest.main()
