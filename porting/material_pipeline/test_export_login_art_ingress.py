#!/usr/bin/env python3
"""Rejection, source-evidence and native-pin checks for the login-art regular-Texture ingress (ExternalLink, BackArrow)."""
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import unittest

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('login_art', HERE / 'export_login_art_ingress.py')
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)
CPP = HERE.parent / 'native/Source/FortniteGame/Private/LegacyMaterialShaderMapProbe.cpp'
INVENTORY = m.GENERATED / 'frontend-probes/frontend-material-inventory-textures'
KEYS = sorted(m.PROFILES)


def load(path):
    return json.loads(path.read_text())


def arrays(expression):
    result = {n: dict(count=0, expressions=[]) for n in m.bundle.decoder.ARRAYS}
    result['texture_2d'] = dict(count=1, expressions=[expression])
    return dict(uniform_expression_arrays=result, virtual_texture_stacks=dict(count=0), parameter_collections=dict(count=0))


class Portable(unittest.TestCase):
    def test_unknown_material_fails_closed(self):
        with self.assertRaises(KeyError):
            m.configure('Nope')

    def test_plain_texture_identity_and_sampler(self):
        for key in KEYS:
            m.configure(key)
            offset, end = m.PROFILE['texture_expression']
            good = dict(offset=offset, end=end, type='FMaterialUniformExpressionTexture', texture=copy.deepcopy(m.BASE_TEXTURE))
            for field, value in [('type', 'FMaterialUniformExpressionTextureParameter'), ('parameter', dict(name='invented')), ('offset', offset + 1)]:
                with self.subTest(key=key, field=field), self.assertRaises(ValueError):
                    bad = copy.deepcopy(good); bad[field] = value; m.validate_arrays(arrays(bad))
            for field, value in [('texture_index', 1), ('sampler_source', 0), ('virtual_texture', True), ('layer_index', 0)]:
                with self.subTest(key=key, field=field), self.assertRaises(ValueError):
                    bad = copy.deepcopy(good); bad['texture'][field] = value; m.validate_arrays(arrays(bad))

    def test_material_sampler_must_be_world_clamp_slot_three(self):
        m.configure('ExternalLink')
        record = dict(frequency=3, material_uniform_buffer=dict(base_index=1, bound=True), uniform_parameters=[])
        desc = dict(set=0, binding=0, d3d_register=dict(kind='sampler', space=0, index=0), vulkan_descriptor_type=0,
                    resource_table_entries=[dict(uniform_buffer=1, resource=1, bind=0, table='sampler')])
        with self.assertRaisesRegex(ValueError, 'resource-table'):
            m.typed_contract(record, dict(descriptors=[desc]))

    def test_profile_hashes_are_original_when_staged(self):
        for key in KEYS:
            m.configure(key)
            for relative, expected in m.SOURCE_HASHES.items():
                path = m.EXTRACTED / relative
                if not path.exists():
                    self.skipTest('original extracted tree unavailable')
                self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), expected, relative)


def native_rows():
    text = CPP.read_text()
    table = {}
    for row in re.finditer(r'\{TEXT\("(Media|ExternalLink|BackArrow)"\),\w+,\w+,TEXT\("(\w+)"\),(\d+),(\d+),(\d+),TEXT\("(\w+)"\),TEXT\("(\w+)"\),TEXT\("(\w+)"\)\}', text):
        table[row[1]] = row.groups()[1:]
    return text, table


class Native(unittest.TestCase):
    def test_native_source_table_matches_profiles(self):
        _, table = native_rows()
        for key, label in (('ExternalLink', 'ExternalLink'), ('BackArrow', 'BackArrow')):
            m.configure(key)
            lod, header, exports, stored, header_sha, exports_sha, raw_sha = table[label]
            hashes = {k.rsplit('/', 1)[1]: v for k, v in m.SOURCE_HASHES.items()}
            self.assertEqual(lod, 'TEXTUREGROUP_UI')
            self.assertEqual(header_sha, hashes[m.TEXTURE_NAME + '.uasset'])
            self.assertEqual(exports_sha, hashes[m.TEXTURE_NAME + '.uexp'])
            self.assertEqual(raw_sha, m.PROFILE['raw_bgra_sha256'])
            self.assertTrue(int(header) < int(stored) < int(header) + int(exports))  # one inline mip inside the .uexp
            if (m.EXTRACTED / (m.TEXTURE_PACKAGE + '.uasset')).exists():
                self.assertEqual(int(header), (m.EXTRACTED / (m.TEXTURE_PACKAGE + '.uasset')).stat().st_size)
                self.assertEqual(int(exports), (m.EXTRACTED / (m.TEXTURE_PACKAGE + '.uexp')).stat().st_size)
        self.assertEqual(set(table), {'Media', 'ExternalLink', 'BackArrow'})


class Generated(unittest.TestCase):
    def bundle(self, key):
        m.configure(key)
        path = m.ROOT / (m.PROFILE['slug'] + '-ingress-1/bundle.json')
        if not path.exists():
            self.skipTest('generated bundle absent')
        return path, json.loads(path.read_bytes())

    def test_native_pins_match_generated_bundles(self):
        text = CPP.read_text()
        for key in KEYS:
            path, data = self.bundle(key)
            self.assertIn(hashlib.sha1(path.read_bytes()).hexdigest(), text)
            self.assertIn(data['provenance']['resource_sha1'], text)

    def test_bundle_is_complete_original_base_material(self):
        for key in KEYS:
            _, data = self.bundle(key)
            fixture = data['runtime_fixture']
            self.assertEqual((data['owner_type'], data['parent_path']), ('UMaterial', ''))
            self.assertEqual(len(data['original_shader_code']), 19)
            self.assertEqual(len(fixture['shaders']), 7)
            self.assertEqual(fixture['texture_2d_parameters'][0]['sampler_source'], 2)
            self.assertIsNone(fixture['texture_contract']['parameter'])
            self.assertFalse(fixture['texture_contract']['named_overrides_allowed'])
            self.assertEqual(fixture['numeric_comparison_bytes'], 16 * len(fixture['vector_roots']) + 4 * len(fixture['scalar_roots']))
            self.assertEqual(fixture['expected_layout'], m.PROFILE['layout'])
            cases = {c['name']: c for c in fixture['uniform_cases']}
            for name in ('unmatched_association', 'unmatched_index', 'unmatched_number'):
                self.assertEqual(cases[name]['numeric_sha256'], cases['defaults']['numeric_sha256'])
            self.assertNotEqual(cases['combined']['numeric_sha256'], cases['defaults']['numeric_sha256'])

    def test_function_closure_and_reference_order(self):
        for key in KEYS:
            _, data = self.bundle(key)
            context = data['runtime_fixture']['texture_context_proof']
            self.assertEqual(context['ordered_texture_references'], [m.TEXTURE_PATH])
            self.assertEqual(sum('texture_object' in x for x in context['trace']), m.PROFILE['texture_samples'])
            self.assertEqual(sum('function_order' in x for x in context['trace']), m.PROFILE['function_calls'])
            self.assertTrue(all(x['texture_appends'] == [] for x in context['trace'] if 'function_order' in x))


if __name__ == '__main__':
    unittest.main()
