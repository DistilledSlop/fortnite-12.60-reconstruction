import copy,hashlib,importlib.util,json,tempfile,unittest
from pathlib import Path
P=Path(__file__).resolve().parent;s=importlib.util.spec_from_file_location('google_verifier',P/'verify_google_icon_runtime_artifacts.py');v=importlib.util.module_from_spec(s);s.loader.exec_module(v)
R=v.GENERATED/'material_pipeline'
class VerificationTests(unittest.TestCase):
 def setUp(self):
  self.temp=tempfile.TemporaryDirectory();self.a=Path(self.temp.name);self.f=json.loads((R/'google-ingress-candidate-1/runtime-fixture.json').read_text());self.profile='defaults'
  self.meta=dict(texture_path=self.f['ordered_texture_references'][0],parameter_name='Texture',parameter_number=0,association=2,index=-1,width=512,height=512,format='PF_DXT1',srgb=True,mips=10,resident_mips_before=7,resident_mips_after=10,sampler_source=0,asset_sampler_binding_match=True,indexed_default_match=True,raw_bc1_gpu_readback_measured=False,layout_hash=self.f['expected_uniform_layout_hash'],constant_bytes=80,resource_offsets=[48,56,64,72],resource_types=[5,8,8,8],named_proxy_calls=1)
  self.images=[bytes(16384)]*3
 def tearDown(self):self.temp.cleanup()
 def write(self):
  self.d=dict(width=64,height=64,format='RGBA8',numeric_profile=self.profile,source_mips_uploaded=10,source_texture_format='BC1_SRGB',source_copy_equal=self.images[0]==self.images[1],checker_delta_policy='required_positive' if self.profile=='combined' else 'observational',raw_bc1_gpu_readback_measured=False,named_proxy_calls=1,checker_changed_pixels=sum(self.images[0][i:i+4]!=self.images[2][i:i+4] for i in range(0,16384,4)))
  for l,raw in zip(['Source','Copy','Checker'],self.images):
   (self.a/('GoogleDraw-'+l+'.rgba')).write_bytes(raw);self.d[l.lower()+'_sha1']=hashlib.sha1(raw).hexdigest()
  self.save()
 def save(self):
  (self.a/'GoogleTexture.json').write_text(json.dumps(self.meta));(self.a/'GoogleDraw.json').write_text(json.dumps(self.d))
 def check(self):return v.check_sampling(self.f,self.a,self.profile)
 def test_default_zero_is_observed(self):self.write();self.assertEqual(self.check()['checker_changed_pixels'],0)
 def test_combined_zero_is_failure(self):
  self.profile='combined';self.write()
  with self.assertRaises(ValueError):self.check()
 def test_combined_source_effect(self):
  self.profile='combined';self.images[2]=bytes([1])+bytes(16383);self.write();self.assertEqual(self.check()['checker_changed_pixels'],1)
 def test_source_copy_mismatch(self):
  self.images[1]=bytes([1])+bytes(16383);self.write()
  with self.assertRaises(ValueError):self.check()
 def test_exact_named_identity(self):
  self.write();self.meta['parameter_name']='NameTexture';self.save()
  with self.assertRaises(ValueError):self.check()
 def test_asset_sampler_cannot_be_worldclamp(self):
  self.write();self.meta['sampler_source']=2;self.save()
  with self.assertRaises(ValueError):self.check()
 def test_residency_not_assumed(self):
  self.write();self.meta['resident_mips_after']=7;self.save()
  with self.assertRaises(ValueError):self.check()
 def test_no_raw_gpu_compression_claim(self):
  self.write();self.d['raw_bc1_gpu_readback_measured']=True;self.save()
  with self.assertRaises(ValueError):self.check()
 def test_truncated_frame(self):
  self.write();(self.a/'GoogleDraw-Source.rgba').write_bytes(b'123')
  with self.assertRaises(ValueError):self.check()
 def test_real_staged_source_and_corruption(self):
  prefix=Path('/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/UI/Login/Art/T_TempIcon_Login_Google');paths=[prefix.with_suffix(s) for s in ['.uasset','.uexp','.ubulk']]
  self.assertEqual(v.check_compressed_source(self.f,*paths)['mip_count'],10)
  bad=self.a/'bad.ubulk';raw=paths[2].read_bytes();bad.write_bytes(raw[:-1]+bytes([raw[-1]^1]))
  with self.assertRaises(ValueError):v.check_compressed_source(self.f,paths[0],paths[1],bad)
if __name__=='__main__':unittest.main()
