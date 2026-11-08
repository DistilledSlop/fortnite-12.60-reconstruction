#!/usr/bin/env python3
"""Rejection, source-evidence and native-pin checks for the M_SkewButtonShape_DF base bundle and its Chip instance."""
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import unittest

HERE = Path(__file__).resolve().parent


def load_module(name):
    spec = importlib.util.spec_from_file_location(name, HERE / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


m = load_module('export_skew_button_ingress')
frame = load_module('skew_button_frame')
CPP = HERE.parent / 'native/Source/FortniteGame/Private/LegacyMaterialShaderMapProbe.cpp'
DF_BUNDLE = m.ROOT / 'skewdf-ingress-1/bundle.json'
CHIP_BUNDLE = m.ROOT / 'skewdf-instances-1/M_SkewButtonShape_Chip.bundle.json'
TEXTURES = {'T_SkewButtonShape_B_DF': m.ROOT / 'skew-texture-B_DF-1/texture-report.json',
            'T_SkewButtonShape_DF': m.ROOT / 'skew-texture-DF-1/texture-report.json'}


def arrays(expression):
    result = {n: dict(count=0, expressions=[]) for n in m.bundle.decoder.ARRAYS}
    result['texture_2d'] = dict(count=1, expressions=[expression])
    return dict(uniform_expression_arrays=result, virtual_texture_stacks=dict(count=0), parameter_collections=dict(count=0))


class Portable(unittest.TestCase):
    def setUp(self):
        m.configure('DF')

    def test_named_texture_identity_and_asset_sampler(self):
        offset, end = m.PROFILE['texture_expression']
        good = dict(offset=offset, end=end, type='FMaterialUniformExpressionTextureParameter', parameter=copy.deepcopy(m.PARAMETER), texture=copy.deepcopy(m.BASE_TEXTURE))
        for field, value in [('type', 'FMaterialUniformExpressionTexture'), ('offset', offset + 1)]:
            with self.subTest(field=field), self.assertRaises(ValueError):
                bad = copy.deepcopy(good); bad[field] = value; m.validate_arrays(arrays(bad))
        for path, value in [(('parameter', 'name', 'name'), 'Other'), (('parameter', 'association'), 0), (('parameter', 'index'), 0),
                            (('texture', 'sampler_source'), 2), (('texture', 'texture_index'), 1), (('texture', 'virtual_texture'), True)]:
            with self.subTest(field=path), self.assertRaises(ValueError):
                bad = copy.deepcopy(good); node = bad
                for key in path[:-1]:
                    node = node[key]
                node[path[-1]] = value; m.validate_arrays(arrays(bad))

    def test_material_sampler_must_be_asset_slot_one(self):
        record = dict(frequency=3, material_uniform_buffer=dict(base_index=1, bound=True), uniform_parameters=[])
        desc = dict(set=0, binding=0, d3d_register=dict(kind='sampler', space=0, index=0), vulkan_descriptor_type=0,
                    resource_table_entries=[dict(uniform_buffer=1, resource=3, bind=0, table='sampler')])
        with self.assertRaisesRegex(ValueError, 'resource-table'):
            m.typed_contract(record, dict(descriptors=[desc]))

    def test_profile_hashes_are_original_when_staged(self):
        for relative, expected in m.SOURCE_HASHES.items():
            path = m.EXTRACTED / relative
            if not path.exists():
                self.skipTest('original extracted tree unavailable')
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), expected, relative)


class Frames(unittest.TestCase):
    def sources(self, name):
        report = TEXTURES[name]
        base = frame.Path('/mnt/vibrato/fortnite-extracted-full') / frame.SPECS[name]['package']
        if not report.exists() or not base.with_suffix('.uexp').exists():
            self.skipTest('texture evidence unavailable')
        return base.with_suffix('.uasset').read_bytes(), base.with_suffix('.uexp').read_bytes(), json.loads(report.read_text())

    def test_both_frames_validate_and_tampering_fails(self):
        for name in TEXTURES:
            header, exports, cue = self.sources(name)
            out = frame.frame(header, exports, cue, name)
            self.assertEqual((out['logical_width'], out['logical_height'], out['mip_count'], out['first_mip_serialized']), (128, 64, 8, 1))
            self.assertEqual([r['width'] for r in out['mip_receipts']], [128, 64, 32, 16, 8, 4, 2, 1])
            with self.assertRaises(ValueError):
                frame.frame(header, exports[:-1] + b'\x00', cue, name)
            bad = copy.deepcopy(cue); bad['firstMip'] = 0
            with self.assertRaises(ValueError):
                frame.frame(header, exports, bad, name)
            bad = copy.deepcopy(cue); bad['mips'][3]['bulkOffset'] += 1
            with self.assertRaises(ValueError):
                frame.frame(header, exports, bad, name)


class Generated(unittest.TestCase):
    def need(self, path):
        if not path.exists():
            self.skipTest('generated bundle absent')
        return json.loads(path.read_bytes())

    def test_native_pins_and_offsets_match_generated_bundles(self):
        text = CPP.read_text()
        df = self.need(DF_BUNDLE); chip = self.need(CHIP_BUNDLE)
        for path in (DF_BUNDLE, CHIP_BUNDLE):
            self.assertIn(hashlib.sha1(path.read_bytes()).hexdigest(), text)
        self.assertEqual(df['provenance']['resource_sha1'], chip['provenance']['resource_sha1'])
        self.assertEqual(text.count(df['provenance']['resource_sha1']), 2)
        offsets = [int(x) for x in re.search(r'GSkewDFMipOffsets\[8\]=\{([\d,]+)\}', text)[1].split(',')]
        receipts = df['runtime_fixture']['texture_contract']['source_platform_data']['mip_receipts']
        self.assertEqual(offsets, [r['uexp_offset'] for r in receipts])
        source = re.search(r'GSkewDFSource=\{(\d+),(\d+),TEXT\("(\w+)"\),TEXT\("(\w+)"\)\}', text)
        hashes = m.PROFILES['DF']['source_hashes']
        self.assertEqual(source.groups(), ('936', '44301', hashes['T_SkewButtonShape_B_DF.uasset'], hashes['T_SkewButtonShape_B_DF.uexp']))

    def test_df_is_complete_original_base_material(self):
        df = self.need(DF_BUNDLE); f = df['runtime_fixture']
        self.assertEqual((df['owner_type'], df['parent_path']), ('UMaterial', ''))
        self.assertEqual((len(df['original_shader_code']), len(f['shaders'])), (19, 7))
        self.assertEqual(f['texture_2d_parameters'][0]['sampler_source'], 0)
        self.assertEqual(f['texture_contract']['parameter'], m.PARAMETER)
        self.assertEqual(f['expected_layout'], m.PROFILES['DF']['layout'])
        self.assertEqual(f['numeric_comparison_bytes'], 16 * len(f['vector_roots']) + 4 * len(f['scalar_roots']))
        self.assertEqual(sum('texture_object' in x for x in f['texture_context_proof']['trace']), 2)
        self.assertEqual(f['texture_context_proof']['ordered_texture_references'], f['ordered_texture_references'])
        self.assertEqual(len(f['ordered_texture_references']), 1)

    def test_chip_inherits_df_resource_and_overrides_texture(self):
        df = self.need(DF_BUNDLE); chip = self.need(CHIP_BUNDLE)
        self.assertEqual((chip['owner_type'], chip['parent_path'], chip['resource_owner_path']), ('UMaterialInstanceConstant', df['owner_path'], df['owner_path']))
        self.assertEqual(chip['resource_raw_hex'], df['resource_raw_hex'])
        f = chip['runtime_fixture']
        self.assertEqual({e['parameter']['name']['name'] for e in f['owner_parameter_expectations']}, {'ShapeSize', 'RefractionDepthBias', 'ShadowColor', 'ShapeColor'})
        texture = f['owner_texture_expectations'][0]
        self.assertEqual(texture['texture_path'], '/Game/UI/Login/Art/T_SkewButtonShape_DF.T_SkewButtonShape_DF')
        self.assertNotEqual(texture['texture_path'], df['runtime_fixture']['texture_2d_parameters'][0]['default_texture_path'])
        self.assertEqual((texture['width'], texture['height'], texture['mip_count']), (128, 64, 8))
        self.assertEqual(texture['source_platform_data']['mip_count'], 8)


if __name__ == '__main__':
    unittest.main()
