#!/usr/bin/env python3
"""Independent Media source, runtime receipt and CLI gate checks."""
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

HERE=Path(__file__).resolve().parent
ROOT=HERE.parent
spec=importlib.util.spec_from_file_location('verifier',HERE/'verify_media_button_runtime_artifacts.py')
v=importlib.util.module_from_spec(spec)
spec.loader.exec_module(v)
FIXTURE=ROOT/'generated/material_pipeline/media_button-ingress-1/runtime-fixture.json'
OTHER=ROOT/'generated/material_pipeline/keybind-ingress-3/runtime-fixture.json'
SOURCE=Path('/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/UI/Icons/T_UI_MediaPlayerButton_Layered_A')


@unittest.skipUnless(FIXTURE.is_file() and SOURCE.with_suffix('.uasset').is_file()
                     and SOURCE.with_suffix('.uexp').is_file(), 'optional original Media source unavailable')
class MediaVerifier(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory(); self.addCleanup(self.tmp.cleanup)
        self.p=Path(self.tmp.name)
        self.f=json.loads(FIXTURE.read_bytes())
        self.source,self.source_report=v.check_source(self.f,SOURCE.with_suffix('.uasset'),SOURCE.with_suffix('.uexp'))
        (self.p/'MediaTexture.rgba').write_bytes(self.source)
        self.meta=dict(texture_path=v.TEXTURE,width=128,height=128,format='PF_B8G8R8A8',srgb=True,mips=1,
                       named_proxy_calls=0,sampler_source=2,world_clamp_binding_match=True,
                       source_uasset_sha256=v.ASSET_SHA,source_uexp_sha256=v.EXPORT_SHA,source_bgra_sha256=v.BGRA_SHA,
                       rgba_sha256=v.RGBA_SHA,layout_hash=89655543,constant_bytes=80,
                       resources=[dict(offset=o,type=t) for o,t in zip([48,56,64,72],[5,8,8,8])])
        self.draw=dict(width=64,height=64,format='RGBA8',named_proxy_calls=0,numeric_profile='defaults',
                       checker_delta_policy='observational',source_mips_uploaded=1,source_texture_format='BGRA8_SRGB',checker_changed_pixels=0,
                       source_copy_equal=True,checker_effect_observed=False,checker_effect_required=False)
        for name in ('Source','Copy','Checker'): self.image(name,bytes(16384))

    def image(self,name,data):
        (self.p/('MediaDraw-'+name+'.rgba')).write_bytes(data)
        self.draw[name.lower()+'_sha1']=hashlib.sha1(data).hexdigest()

    def check(self,profile='defaults'):
        (self.p/'MediaTexture.json').write_text(json.dumps(self.meta))
        (self.p/'MediaDraw.json').write_text(json.dumps(self.draw))
        return v.check_sampling(self.f,self.p,profile,self.source)

    def test_default_zero_and_combined_positive(self):
        self.assertEqual(self.check()['checker_changed_pixels'],0)
        self.draw.update(numeric_profile='combined',checker_delta_policy='required_positive',checker_effect_required=True)
        with self.assertRaisesRegex(ValueError,'combined effect'): self.check('combined')
        self.image('Checker',b'\1'+bytes(16383)); self.draw['checker_changed_pixels']=1; self.draw['checker_effect_observed']=True
        self.assertEqual(self.check('combined')['checker_changed_pixels'],1)
        self.draw['checker_delta_policy']='observational'
        with self.assertRaisesRegex(ValueError,'policy'): self.check('combined')

    def test_copy_tampering_with_recomputed_hash(self):
        self.image('Copy',b'\1'+bytes(16383))
        with self.assertRaisesRegex(ValueError,'independent copy'): self.check()

    def test_source_pair_corruption(self):
        asset=SOURCE.with_suffix('.uasset').read_bytes(); exports=SOURCE.with_suffix('.uexp').read_bytes()
        for bad_asset in (True,False):
            a=self.p/'x.uasset'; e=self.p/'x.uexp'
            a.write_bytes((b'\0'+asset[1:]) if bad_asset else asset)
            e.write_bytes(exports if bad_asset else (exports[:-1]+bytes([exports[-1]^1])))
            with self.assertRaisesRegex(ValueError,'source pair'): v.check_source(self.f,a,e)
        self.f['texture_contract']['source_platform_data']['stored_bulk_offset']=1114
        with self.assertRaisesRegex(ValueError,'source frame'): v.check_source(self.f,SOURCE.with_suffix('.uasset'),SOURCE.with_suffix('.uexp'))

    def test_identity_sampler_index_and_layout(self):
        original=copy.deepcopy(self.f)
        for key,value in [('parameter',{'name':'invented'}),('texture_index',1),('sampler_source',0)]:
            self.f=copy.deepcopy(original); self.f['texture_2d_parameters'][0][key]=value
            with self.assertRaisesRegex(ValueError,'identity, index or sampler'): self.check()
        self.f=original
        self.meta['world_clamp_binding_match']=False
        with self.assertRaisesRegex(ValueError,'world-clamp'): self.check()
        self.meta['world_clamp_binding_match']=True; self.meta['named_proxy_calls']=1
        with self.assertRaisesRegex(ValueError,'world-clamp'): self.check()
        self.meta['named_proxy_calls']=0; self.meta['resources'][3]['offset']=64
        with self.assertRaisesRegex(ValueError,'layout'): self.check()

    def test_source_gpu_tamper_with_recomputed_hash(self):
        data=b'\0'+self.source[1:] if self.source[0]!=0 else b'\1'+self.source[1:]
        (self.p/'MediaTexture.rgba').write_bytes(data); self.meta['rgba_sha256']=v.sha(data)
        with self.assertRaisesRegex(ValueError,'GPU RGBA'): self.check()

    def test_log_requires_all_actual_evidence(self):
        sampling=self.check()
        lines=['Legacy material ingress: Media owner complete and attached on both threads; seven shaders and source/copy checks passed',
               'Legacy material probe: 7 owner live uniform cases passed',
               'Legacy material probe: 7 plain texture owner live uniform cases passed',
               'Legacy Media texture: real indexed source, zero named proxy calls, GPU RGBA SHA256='+v.RGBA_SHA,
               'Legacy Media texture: source/copy agree; checker changes 0 pixels, profile=defaults']
        v.check_log('\n'.join(lines),self.f,sampling)
        for i in range(len(lines)):
            with self.assertRaisesRegex(ValueError,'runtime log'): v.check_log('\n'.join(lines[:i]+lines[i+1:]),self.f,sampling)
        with self.assertRaisesRegex(ValueError,'runtime log'): v.check_log('\n'.join(lines+[lines[0]]),self.f,sampling)

    def test_actual_cli_fixture_gate(self):
        self.assertEqual(v.sha(FIXTURE.read_bytes()),v.FIXTURE_SHA256)
        if not OTHER.exists():
            self.skipTest('optional comparison fixture unavailable')
        def call(fixture):
            return subprocess.run([sys.executable,'-I',str(HERE/'verify_media_button_runtime_artifacts.py'),
                '--fixture',str(fixture),'--artifacts',str(self.p),'--log',str(self.p/'absent.log'),
                '--texture-uasset',str(self.p/'absent.uasset'),'--texture-uexp',str(self.p/'absent.uexp'),
                '--report',str(ROOT/'generated/media-verifier-cli-no-output.json')],capture_output=True,text=True)
        accepted=call(FIXTURE); rejected=call(OTHER)
        self.assertNotEqual(accepted.returncode,0); self.assertIn('absent.uasset',accepted.stderr)
        self.assertNotEqual(rejected.returncode,0); self.assertIn('fixture differs',rejected.stderr)


if __name__=='__main__': unittest.main()
