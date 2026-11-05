#!/usr/bin/env python3
"""Checks for Epic Logo's original texture closure and shader bindings."""
import copy
import importlib.util
import json
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('epic_logo', Path(__file__).with_name('export_epic_logo_ingress.py'))
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)
I = m.GENERATED / 'frontend-probes/frontend-material-inventory-textures'
STEM = m.PACKAGE.replace('/', '_')
BASE = m.GENERATED / 'reconstruction-visual-2026-11-05'
F = BASE / 'signin-debug-epic-functions-2'
T = BASE / 'signin-debug-epic-texture/texture-report.json'
S = BASE / 'signin-debug-epic-shaders/reflection.json'

@unittest.skipUnless((I / (STEM + '.decoded.json')).exists() and F.exists() and T.exists() and S.exists(), 'original evidence unavailable')
class OriginalEvidence(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        load = lambda p: json.loads(p.read_text())
        cls.parsed = load(I / (STEM + '.decoded.json'))
        cls.owner = load(I / (STEM + '.json'))
        cls.exports = load(I / (STEM + '.export-map.json'))
        cls.functions = [load(p) for p in F.glob('*.json')]
        cls.meta = load(T)
        cls.source = m.EXTRACTED / (m.TEXTURE_PACKAGE + '.uexp')
        cls.header = cls.source.with_suffix('.uasset').stat().st_size

    def test_source_reference_and_function_closure(self):
        proof = m.prove_context(self.owner, self.exports, self.functions)
        self.assertEqual(proof['ordered_texture_references'], [m.TEXTURE_PATH])
        with self.assertRaises(ValueError):
            m.prove_context(self.owner, self.exports, self.functions[:-1])
        owner = copy.deepcopy(self.owner)
        expression = next(e for e in owner['expressionExports'] if e['exportType'] == 'MaterialExpressionTextureSample')
        next(p for p in expression['properties'] if p['name'] == 'Texture')['value'] = 'wrong'
        with self.assertRaises(ValueError):
            m.prove_context(owner, self.exports, self.functions)

    def test_source_texture_and_sampler_are_bound_to_original(self):
        self.assertEqual(m.validate_texture(self.meta, self.source.read_bytes(), self.header)['rgba_sha256'], self.meta['rgbaSha256'])
        m.validate_arrays(self.parsed)
        modified = copy.deepcopy(self.parsed)
        modified['uniform_expression_arrays']['texture_2d']['expressions'][0]['texture']['sampler_source'] = 2
        with self.assertRaises(ValueError):
            m.validate_arrays(modified)
        changed = copy.deepcopy(self.meta)
        changed['srgb'] = False
        with self.assertRaises(ValueError):
            m.validate_texture(changed, self.source.read_bytes(), self.header)

    def test_all_original_slate_shader_bindings(self):
        table = {row['shader_index']: row for row in json.loads(S.read_text())['records']}
        self.assertEqual(set(table), {2480, 2481, 111465, 111466, 111467, 111468, 111469})
        for record in self.parsed['material_shaders']['records']:
            contract = m.typed_contract(record, table[record['library_association']['index']])
            if record['frequency'] == 3:
                self.assertIn('TextureAssetSampler', contract['roles'])
                sampler = next(r for r in contract['material_resources'] if r['d3d_register']['kind'] == 'sampler')
                self.assertEqual(sampler['resource_index'], 1)

if __name__ == '__main__':
    unittest.main()
