#!/usr/bin/env python3
"""Verify destination identity and rejection of changed export evidence."""
import json
from pathlib import Path
import struct
import tempfile
import unittest
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
import export_meshes as ex
import import_manifest as manifest


def sample_glb(meshes=1):
    data = json.dumps({'asset': {'version':'2.0'},
                       'meshes':[{'primitives':[{'attributes':{'POSITION':0}}]} for _ in range(meshes)]}).encode()
    data += b' ' * (-len(data) % 4)
    return struct.pack('<4sIIII', b'glTF', 2, len(data) + 20, len(data), 0x4e4f534a) + data


class ImportManifestTests(unittest.TestCase):
    def test_same_basename_distinct_destinations_and_tamper(self):
        with tempfile.TemporaryDirectory() as d:
            root, output = Path(d) / 'input', Path(d) / 'output'
            names = ['FortniteGame/Content/A/SM_One', 'FortniteGame/Content/B/SM_One']
            tool = {'tool':'hash'}
            records = {}
            for n in names:
                asset = root / (n + '.uasset')
                asset.parent.mkdir(parents=True, exist_ok=True)
                asset.write_bytes(b'input')
                glb = output / 'glb' / n / 'SM_One.glb'
                glb.parent.mkdir(parents=True, exist_ok=True)
                glb.write_bytes(sample_glb())
                records[n] = {'status':'ok', 'input':ex.input_fingerprint(root, n), 'tool':tool,
                              'files':[{'name':glb.name, 'sha256':ex.digest(glb)}]}
            state = {'version':1, 'input_root':str(root), 'records':records}
            result = manifest.build_manifest(names, state, root, output, tool)
            self.assertEqual([x['DestinationPath'] for x in result['import_settings']['ImportGroups']], ['/Game/A', '/Game/B'])
            self.assertNotEqual(result['packages'][0]['glb'], result['packages'][1]['glb'])
            with self.assertRaisesRegex(ValueError, 'unverified checkpoint'):
                manifest.build_manifest(names, state, root, output, {'tool':'changed'})
            (root / (names[0] + '.uasset')).write_bytes(b'changed')
            with self.assertRaisesRegex(ValueError, 'unverified checkpoint'):
                manifest.build_manifest(names, state, root, output, tool)
            (root / (names[0] + '.uasset')).write_bytes(b'input')
            (output / 'glb' / names[0] / 'SM_One.glb').write_bytes(b'bad')
            with self.assertRaises(ValueError):
                manifest.build_manifest(names, state, root, output, tool)

    def test_selection_validation(self):
        self.assertEqual(manifest.selection_names({'sources':[{'packages':['FortniteGame/Content/A/SM_One']}]}),
                         ['FortniteGame/Content/A/SM_One'])
        with self.assertRaises(ValueError):
            manifest.selection_names({'sources':[{'packages':['../SM_One']}]})
        with self.assertRaises(ValueError):
            manifest.selection_names({'sources':[]})

    def test_nonmatching_export_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            root, output = Path(d) / 'input', Path(d) / 'output'
            name = 'FortniteGame/Content/A/SM_Test'
            asset = root / (name + '.uasset')
            asset.parent.mkdir(parents=True)
            asset.write_bytes(b'a')
            glb = output / 'glb' / name / 'SM_Test_LOD0.glb'
            glb.parent.mkdir(parents=True)
            glb.write_bytes(sample_glb())
            record = {'status':'ok', 'input':ex.input_fingerprint(root, name), 'tool':{},
                      'files':[{'name':glb.name, 'sha256':ex.digest(glb)}]}
            with self.assertRaisesRegex(ValueError, 'expected exactly'):
                manifest.build_manifest([name], {'version':1, 'input_root':str(root), 'records':{name:record}}, root, output, {})

    def test_multiple_meshes_rejected_even_with_matching_filename(self):
        with tempfile.TemporaryDirectory() as d:
            root, output = Path(d) / 'input', Path(d) / 'output'
            name = 'FortniteGame/Content/A/SM_Test'
            asset = root / (name + '.uasset')
            asset.parent.mkdir(parents=True)
            asset.write_bytes(b'a')
            glb = output / 'glb' / name / 'SM_Test.glb'
            glb.parent.mkdir(parents=True)
            glb.write_bytes(sample_glb(meshes=2))
            record = {'status':'ok', 'input':ex.input_fingerprint(root, name), 'tool':{},
                      'files':[{'name':glb.name, 'sha256':ex.digest(glb)}]}
            with self.assertRaisesRegex(ValueError, 'exactly one glTF mesh'):
                manifest.build_manifest([name], {'version':1, 'input_root':str(root), 'records':{name:record}}, root, output, {})

if __name__ == '__main__':
    unittest.main()
