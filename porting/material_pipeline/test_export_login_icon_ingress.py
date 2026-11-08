#!/usr/bin/env python3
"""Consistency checks for the login-icon family bundles and their native pins."""
import hashlib
import json
from pathlib import Path
import re
import unittest

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent / 'generated/material_pipeline'
CPP = HERE.parent / 'native/Source/FortniteGame/Private/LegacyMaterialShaderMapProbe.cpp'
FACEBOOK = 'facebook-ingress-1/bundle.json'
INSTANCES = {n: 'login-icon-instances-1/M_TempIcon_Login_%s.bundle.json' % n for n in ('PS4', 'XBox', 'Epic')}


def sha1(path):
    return hashlib.sha1((ROOT / path).read_bytes()).hexdigest()


@unittest.skipUnless((ROOT / FACEBOOK).exists(), 'generated bundles absent')
class LoginIconBundles(unittest.TestCase):
    def test_native_pins_match_generated_bundles(self):
        text = CPP.read_text()
        facebook = json.loads((ROOT / FACEBOOK).read_bytes())
        resource = facebook['provenance']['resource_sha1']
        for path in [FACEBOOK] + list(INSTANCES.values()):
            self.assertIn(sha1(path), text)
        self.assertEqual(text.count(resource), 4)

    def test_facebook_is_complete_base_material(self):
        data = json.loads((ROOT / FACEBOOK).read_bytes())
        self.assertEqual((data['owner_type'], data['parent_path']), ('UMaterial', ''))
        self.assertEqual(data['decoded']['shader_record_count'], 19)
        self.assertEqual(len(data['runtime_fixture']['shaders']), 7)
        mips = data['runtime_fixture']['texture_contract']['source_platform_data']['mip_receipts']
        self.assertEqual(len(mips), 10)

    def test_instances_inherit_facebook_resource(self):
        facebook = json.loads((ROOT / FACEBOOK).read_bytes())
        for name, path in INSTANCES.items():
            data = json.loads((ROOT / path).read_bytes())
            self.assertEqual(data['owner_type'], 'UMaterialInstanceConstant')
            self.assertEqual(data['resource_owner_path'], facebook['owner_path'])
            self.assertEqual(data['resource_raw_hex'], facebook['resource_raw_hex'])
            fixture = data['runtime_fixture']
            self.assertEqual(len(fixture['owner_texture_expectations']), 1)
            self.assertEqual({e['parameter']['name']['name'] for e in fixture['owner_parameter_expectations']},
                             {'IconFilter', 'RefractionDepthBias'})


if __name__ == '__main__':
    unittest.main()
