"""Reject falsified size bits, atlas bytes, sampler bindings and draw evidence."""
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('social_verify', Path(__file__).with_name('verify_social_border_runtime_artifacts.py'))
verifier = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verifier)


class SamplingContract(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.folder = Path(self.temp.name)
        self.half = bytes(8192)
        self.layout = dict(constant_bytes=160, resource_offsets=[128, 136, 144, 152], resource_types=[5, 8, 8, 8])
        self.fixture = dict(texture_contract=dict(expression_kind='Texture', parameter=None,
            named_overrides_allowed=False, default_texture_path=verifier.ATLAS,
            source_platform_data=dict(raw_sha256=verifier.sha(self.half))),
            ordered_texture_references=[verifier.ATLAS],
            texture_2d_parameters=[dict(parameter=None, expression_kind='Texture', texture_index=0, sampler_source=2)],
            nodes=[dict(type='TextureSize2D', texture_parameter_index=0)], vector_roots=[0, 0, 0], expected_layout=self.layout)
        self.meta = dict(texture_path=verifier.ATLAS, width=32, height=32, format='PF_FloatRGBA', srgb=False, mips=1, filter='TF_Bilinear', addressX='TA_Clamp', addressY='TA_Clamp', compressionSettings='TC_HDR',
            named_proxy_calls=0, sampler_source=2, bound_sampler_resource=3, root=2,
            root_bits=['42000000', '42000000', '00000000', '00000000'], constant_bytes=160,
            layout_hash=94898423, resources=[dict(offset=o, type=t) for o, t in zip(self.layout['resource_offsets'], self.layout['resource_types'])],
            gpu_half_bytes=8192, gpu_half_sha256=verifier.sha(self.half))
        self.write('SocialTexture.json', self.meta)
        (self.folder / 'SocialTexture.rgba16f').write_bytes(self.half)
        self.frames = [bytes(16384), bytes(16384), bytes(16384)]
        self.draw = dict(width=64, height=64, format='RGBA8', numeric_profile='defaults', source_mips_uploaded=1,
            source_texture_format='PF_FloatRGBA', bound_sampler_resource=3, named_proxy_calls=0,
            source_copy_equal=True, checker_effect_required=False, checker_changed_pixels=0)
        self.write_frames()

    def write(self, name, data):
        (self.folder / name).write_text(json.dumps(data))

    def write_frames(self):
        for name, data in zip(['Source', 'Copy', 'Checker'], self.frames):
            (self.folder / ('SocialDraw-' + name + '.rgba')).write_bytes(data)
            self.draw[name.lower() + '_sha1'] = hashlib.sha1(data).hexdigest()
        self.write('SocialDraw.json', self.draw)

    def check(self, profile='defaults'):
        return verifier.check_sampling(self.fixture, self.folder, profile, self.half)

    def test_default_checker_observation_and_separate_combined_effect(self):
        self.assertEqual(self.check()['checker_changed_pixels'], 0)
        self.draw.update(numeric_profile='combined', checker_effect_required=True)
        self.write_frames()
        with self.assertRaisesRegex(ValueError, 'required combined sampling'):
            self.check('combined')
        self.frames[2] = b'\xff' + bytes(16383)
        self.draw['checker_changed_pixels'] = 1
        self.write_frames()
        self.assertEqual(self.check('combined')['checker_changed_pixels'], 1)

    def test_negative_zero_or_wrong_live_dimensions_rejected(self):
        for bits in [['42000000', '42000000', '80000000', '00000000'],
                     ['40e00000', '41500000', '00000000', '00000000']]:
            with self.subTest(bits=bits), self.assertRaisesRegex(ValueError, 'size bits'):
                self.meta['root_bits'] = bits
                self.write('SocialTexture.json', self.meta)
                self.check()

    def test_world_clamp_cannot_be_asset_sampler_or_named_lookup(self):
        for key, value in [('bound_sampler_resource', 1), ('sampler_source', 0), ('named_proxy_calls', 1)]:
            bad = dict(self.meta, **{key: value})
            self.write('SocialTexture.json', bad)
            with self.subTest(field=key), self.assertRaises(ValueError):
                self.check()

    def test_atlas_constructor_metadata_mutations_are_rejected(self):
        for key, value in [('srgb', True), ('filter', 'TF_Default'), ('addressX', 'TA_Wrap'),
                           ('addressY', 'TA_Wrap'), ('compressionSettings', 'TC_Default')]:
            self.write('SocialTexture.json', dict(self.meta, **{key: value}))
            with self.subTest(field=key), self.assertRaises(ValueError):
                self.check()

    def test_gpu_source_bytes_cannot_be_changed_with_a_matching_self_hash(self):
        wrong = b'\x01' + self.half[1:]
        (self.folder / 'SocialTexture.rgba16f').write_bytes(wrong)
        self.meta['gpu_half_sha256'] = verifier.sha(wrong)
        self.write('SocialTexture.json', self.meta)
        with self.assertRaisesRegex(ValueError, 'half-float bytes'):
            self.check()

    def test_independent_copy_cannot_be_changed_with_a_matching_self_hash(self):
        self.frames[1] = b'\x01' + bytes(16383)
        self.write_frames()
        with self.assertRaisesRegex(ValueError, 'exact-copy draws'):
            self.check()

    def test_actual_checker_delta_and_indexed_identity_required(self):
        self.draw['checker_changed_pixels'] = 1
        self.write_frames()
        with self.assertRaisesRegex(ValueError, 'checker effect'):
            self.check()
        self.draw['checker_changed_pixels'] = 0
        self.write_frames()
        self.fixture['texture_2d_parameters'][0]['parameter'] = dict(name='invented')
        with self.assertRaisesRegex(ValueError, 'dynamic root contract'):
            self.check()


class VMAndCliContract(unittest.TestCase):
    def test_original_staged_pair_and_corruption(self):
        fixture_path = verifier.GENERATED / 'material_pipeline/social-atlas-corrected-ingress-3/runtime-fixture.json'
        source = Path('/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/UI/SocialPanel/Art/CA_SocialPanel')
        asset, exports = source.with_suffix('.uasset'), source.with_suffix('.uexp')
        if not fixture_path.exists() or not asset.exists() or not exports.exists():
            self.skipTest('optional original atlas source unavailable')
        fixture = json.loads(fixture_path.read_text())
        raw, result = verifier.check_source(fixture, asset, exports)
        self.assertEqual(len(raw), 8192)
        self.assertEqual(result['raw_half_sha256'], '36e082f7fafe908ba57a4e2964752bab086991cd7bace6a902af701d92d79836')
        with tempfile.TemporaryDirectory() as temp:
            broken = Path(temp) / 'atlas.uexp'
            data = bytearray(exports.read_bytes())
            data[329] ^= 1
            broken.write_bytes(data)
            with self.assertRaisesRegex(ValueError, 'staged atlas pair'):
                verifier.check_source(fixture, asset, broken)

    def test_missing_case_wrong_bits_and_unrejected_form_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            folder = Path(temp)
            cases = [dict(name=name, bits=bits) for name, bits in [
                ('real_source_in_scratch_context', ['42000000', '42000000', '00000000', '00000000']),
                ('absent_index', ['00000000'] * 4), ('null_resource', ['00000000'] * 4),
                ('alternate_dimensions', ['40e00000', '41500000', '00000000', '00000000']),
                ('restored_source', ['42000000', '42000000', '00000000', '00000000'])]]
            negatives = [dict(name='raw:' + name, rejected_at_expected_gate=True)
                for name in ['property1', 'named source', 'scalar root', 'nested property', 'child source slot']]
            negatives += [dict(name=kind + ':' + name, rejected_at_expected_gate=True)
                for kind in ['json', 'factory'] for name in ['property1', 'bad normalized slot', 'scalar root', 'nested property', 'named source']]
            write = lambda name, obj: (folder / name).write_text(json.dumps(obj))
            write('TextureSize2DScratchBits.json', dict(passed=True, opcode=42, cases=cases))
            write('TextureSize2DRejectedForms.json', dict(passed=True, cases=negatives))
            self.assertEqual(verifier.check_vm_controls(folder)['rejected_form_count'], 15)
            cases[1]['bits'][3] = '80000000'
            write('TextureSize2DScratchBits.json', dict(passed=True, opcode=42, cases=cases))
            with self.assertRaisesRegex(ValueError, 'positive-zero'):
                verifier.check_vm_controls(folder)
            cases[1]['bits'][3] = '00000000'
            write('TextureSize2DScratchBits.json', dict(passed=True, opcode=42, cases=cases))
            negatives[0]['rejected_at_expected_gate'] = False
            write('TextureSize2DRejectedForms.json', dict(passed=True, cases=negatives))
            with self.assertRaisesRegex(ValueError, 'three layers'):
                verifier.check_vm_controls(folder)

    def test_actual_fixture_cli_gate_rejects_another_material(self):
        good = verifier.GENERATED / 'material_pipeline/social-atlas-corrected-ingress-3/runtime-fixture.json'
        wrong = verifier.GENERATED / 'material_pipeline/flogo-ingress-3/runtime-fixture.json'
        if not good.exists() or not wrong.exists():
            self.skipTest('optional original fixtures unavailable')
        self.assertEqual(verifier.sha(good.read_bytes()), verifier.FIXTURE_SHA256)
        with tempfile.TemporaryDirectory(dir=verifier.GENERATED) as temp:
            folder = Path(temp)
            def run(fixture):
                return subprocess.run([sys.executable, '-I', str(Path(verifier.__file__)), '--fixture', str(fixture),
                    '--artifacts', str(folder), '--log', str(folder / 'missing.log'),
                    '--texture-uasset', str(folder / 'missing.uasset'), '--texture-uexp', str(folder / 'missing.uexp'),
                    '--report', str(folder / 'report.json')], capture_output=True, text=True)
            accepted = run(good)
            self.assertNotEqual(accepted.returncode, 0)
            self.assertIn('missing.log', accepted.stderr)
            self.assertNotIn('fixture differs', accepted.stderr)
            rejected = run(wrong)
            self.assertNotEqual(rejected.returncode, 0)
            self.assertIn('fixture differs', rejected.stderr)
            self.assertFalse((folder / 'report.json').exists())


if __name__ == '__main__':
    unittest.main()
