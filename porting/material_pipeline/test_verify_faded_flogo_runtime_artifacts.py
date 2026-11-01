#!/usr/bin/env python3
"""Reject contradictory named-texture and BC3 sampling receipts."""
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import subprocess
import sys
import unittest

spec=importlib.util.spec_from_file_location('verifier',Path(__file__).with_name('verify_faded_flogo_runtime_artifacts.py'));v=importlib.util.module_from_spec(spec);spec.loader.exec_module(v)
class SamplingIntegrity(unittest.TestCase):
 def setUp(self):
  self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup);self.root=Path(self.temp.name)
  ident=dict(name=dict(name='TextureMask',number=0),association=2,index=-1);path='/Game/Test.Texture';layout=dict(constant_bytes=208,resource_offsets=[176,184,192,200],resource_types=[5,8,8,8])
  self.fixture=dict(texture_contract=dict(expression_kind='TextureParameter',parameter=ident,named_overrides_allowed=True,default_texture_path=path,override_texture_path=path,
   source_platform_data=dict(compressed_sha256='660f15379061b43dcf835dcde3b2b1f141361e32c4f78bd578013d596c28a4d3',compressed_bytes=262144,stored_bulk_offset=1241,mip_count=1),
   default=dict(rgbaSha256='cdab3ce448c5555adebdf2de8a3423f234107a22d2636806b9aca58b6dffe842')),
   texture_2d_parameters=[dict(expression_kind='TextureParameter',parameter=copy.deepcopy(ident),texture_index=0)],ordered_texture_references=[path],expected_layout=layout)
  self.meta=dict(texture_path=path,parent_path='/Game/Athena/UI/Season/Art/M_UI_AthenaSeason_Stylized_Font.M_UI_AthenaSeason_Stylized_Font',width=1024,height=256,format='PF_DXT5',srgb=True,mips=1,indexed_default_match=True,named_override_match=True,named_proxy_calls=2,constant_bytes=208,layout_hash=v.common.uniform_layout_hash(208,layout['resource_offsets'],layout['resource_types']),resources=[dict(offset=o,type=t) for o,t in zip(layout['resource_offsets'],layout['resource_types'])])
  self.draw=dict(width=64,height=64,format='RGBA8',named_proxy_calls=1,numeric_profile='defaults',source_mips_uploaded=1,source_texture_format='BC3_SRGB',checker_changed_pixels=1)
  self.image('Source',bytes(64*64*4));self.image('Copy',bytes(64*64*4));self.image('Checker',b'\1'+bytes(64*64*4-1))
 def image(self,label,data):
  (self.root/('FlogoDraw-'+label+'.rgba')).write_bytes(data);self.draw[label.lower()+'_sha1']=hashlib.sha1(data).hexdigest()
 def check(self):
  (self.root/'FlogoTexture.json').write_text(json.dumps(self.meta));(self.root/'FlogoDraw.json').write_text(json.dumps(self.draw));return v.check_sampling(self.fixture,self.root,'defaults')
 def test_consistent(self):self.assertEqual(self.check()['checker_changed_pixels'],1)
 def test_named_override_and_parent_source_required(self):
  for key,val in [('named_proxy_calls',0),('indexed_default_match',False),('named_override_match',False),('parent_path','/Wrong')]:
   with self.subTest(key=key):
    old=self.meta[key];self.meta[key]=val
    with self.assertRaises(ValueError):self.check()
    self.meta[key]=old
 def test_identity_and_layout_cannot_change(self):
  self.fixture['texture_2d_parameters'][0]['parameter']['name']['number']=1
  with self.assertRaises(ValueError):self.check()
  self.fixture['texture_2d_parameters'][0]['parameter']['name']['number']=0;self.meta['resources'][0]['offset']=184
  with self.assertRaises(ValueError):self.check()
 def test_source_copy_must_match_even_with_own_hash(self):
  self.image('Copy',b'\1'+bytes(64*64*4-1))
  with self.assertRaisesRegex(ValueError,'exact BC3 source-copy'):self.check()
 def test_checker_delta_must_be_real_and_exact(self):
  self.image('Checker',bytes(64*64*4))
  with self.assertRaisesRegex(ValueError,'checker'):self.check()
  self.image('Checker',b'\1'+bytes(64*64*4-1));self.draw['checker_changed_pixels']=2
  with self.assertRaisesRegex(ValueError,'checker'):self.check()
 def test_owner_log_requires_all_checks_once(self):
  f=dict(owner_parameter_expectations=[{}]*14,uniform_cases=[{}]*9,owner_uniform_cases=[{}]*9)
  lines=['Legacy material ingress: verified 14 source override identities and values',
   'Legacy material ingress: real parent map complete and attached on game/render threads; instance overrides verified',
   'Legacy material probe: 9 owner live uniform cases passed',
   'Legacy material probe: 9 plain texture owner live uniform cases passed',
   'Legacy material ingress: Flogo owner complete and attached on both threads; seven shaders and controlled source sampling passed']
  self.assertEqual(v.check_log('\n'.join(lines),f)['verified_source_overrides'],14)
  with self.assertRaises(ValueError):v.check_log('\n'.join(lines[:-1]),f)
  with self.assertRaises(ValueError):v.check_log('\n'.join(lines+[lines[0]]),f)
 def test_profile_and_bc3_metadata(self):
  self.draw['numeric_profile']='combined'
  with self.assertRaisesRegex(ValueError,'profile'):self.check()
  self.draw['numeric_profile']='defaults';self.draw['source_texture_format']='BGRA8'
  with self.assertRaisesRegex(ValueError,'profile'):self.check()
class OptionalOriginalSource(unittest.TestCase):
 def test_original_compressed_source_and_corruption(self):
  fixture=Path(__file__).resolve().parents[1]/'generated/material_pipeline/flogo-ingress-3/runtime-fixture.json'
  source=Path('/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/UI/Foundation/Textures/Logo/T-Logo-Wordmark-1024x256')
  if not fixture.is_file() or not source.with_suffix('.uexp').is_file():self.skipTest('optional original texture source unavailable')
  data=json.loads(fixture.read_text());uasset=source.with_suffix('.uasset');uexp=source.with_suffix('.uexp')
  self.assertEqual(v.check_compressed_source(data,uasset,uexp)['compressed_bytes'],262144)
  with tempfile.TemporaryDirectory() as directory:
   changed=Path(directory)/'changed.uexp';raw=bytearray(uexp.read_bytes());raw[337]^=1;changed.write_bytes(raw)
   with self.assertRaisesRegex(ValueError,'staged texture package hashes'):v.check_compressed_source(data,uasset,changed)
class FixtureGate(unittest.TestCase):
 def test_exact_original_fixture_pin(self):
  self.assertEqual(v.FIXTURE_SHA256,'a17dc9d8b8adddbe95c953695804a56620096681406a5a7d3cf2a3fd98f396f9')
  self.assertNotEqual(v.FIXTURE_SHA256,'26a3e93fc381a7eb20bc42f0f0bcf6ad453adf06348f199848cd5c8437dd6f02')
 def test_cli_accepts_flogo_gate_and_rejects_keybind_fixture(self):
  generated=Path(__file__).resolve().parents[1]/'generated'
  good=generated/'material_pipeline/flogo-ingress-3/runtime-fixture.json'
  wrong=generated/'material_pipeline/keybind-ingress-3/runtime-fixture.json'
  if not good.is_file() or not wrong.is_file():self.skipTest('optional pinned local fixtures unavailable')
  with tempfile.TemporaryDirectory(dir=generated) as directory:
   root=Path(directory)
   def run(fixture):
    return subprocess.run([sys.executable,'-I',str(Path(v.__file__)), '--fixture',str(fixture),
     '--artifacts',str(root), '--log',str(root/'missing.log'), '--texture-uasset',str(root/'missing.uasset'),
     '--texture-uexp',str(root/'missing.uexp'), '--report',str(root/'report.json')],capture_output=True,text=True)
   accepted=run(good)
   self.assertNotEqual(accepted.returncode,0)
   self.assertNotIn('fixture differs',accepted.stderr)
   self.assertIn('missing.log',accepted.stderr) # Flogo passed the real CLI gate before the expected absent-artifact failure.
   rejected=run(wrong)
   self.assertNotEqual(rejected.returncode,0)
   self.assertIn('fixture differs',rejected.stderr)
   self.assertFalse((root/'report.json').exists())
if __name__=='__main__':unittest.main()
