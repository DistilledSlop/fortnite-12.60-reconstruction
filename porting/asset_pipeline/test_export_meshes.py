#!/usr/bin/env python3
"""Exercise checkpoint selection and output corruption detection without UE."""
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('export_meshes', Path(__file__).with_name('export_meshes.py'))
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


def glb():
    chunk = json.dumps({'asset': {'version': '2.0'}, 'meshes': [{'primitives': [{'attributes': {'POSITION': 0}}]}]}).encode()
    chunk += b' ' * (-len(chunk) % 4)
    return struct.pack('<4sIIII', b'glTF', 2, 20 + len(chunk), len(chunk), 0x4e4f534a) + chunk


class CheckpointTests(unittest.TestCase):
    def test_resume_corruption_and_failed_retry(self):
        with tempfile.TemporaryDirectory() as d:
            base = Path(d)
            root, out = base / 'inputs', base / 'outputs'
            name = 'FortniteGame/Content/A/SM_Same'
            other = 'FortniteGame/Content/B/SM_Same'
            for n in (name, other):
                p = root / (n + '.uasset')
                p.parent.mkdir(parents=True, exist_ok=True)
                p.write_bytes(b'asset')
            target = out / 'glb' / name / 'SM_Same.glb'
            target.parent.mkdir(parents=True)
            target.write_bytes(glb())
            tool = {'Cue4Inspect.dll':'hash'}
            records = {name: {'status':'ok', 'input':m.input_fingerprint(root, name), 'tool':tool,
                              'files':[{'name':target.name, 'sha256':m.digest(target)}]},
                       other:{'status':'failed'}}
            self.assertEqual(m.select_candidates([name, other], records, root, out, tool, False, 10), [])
            self.assertEqual(m.select_candidates([name, other], records, root, out, tool, True, 10), [other])
            target.write_bytes(b'corrupt')
            self.assertEqual(m.select_candidates([name, other], records, root, out, tool, False, 10), [name])
            target.write_bytes(glb())
            (root / (name + '.uexp')).write_bytes(b'new sidecar')
            self.assertFalse(m.verified(records[name], root, out, tool, name))

    def test_optional_bulk_sidecars_invalidate_checkpoint(self):
        with tempfile.TemporaryDirectory() as d:
            base = Path(d)
            root, out = base / 'inputs', base / 'outputs'
            name = 'FortniteGame/Content/A/SM_Test'
            asset = root / (name + '.uasset')
            asset.parent.mkdir(parents=True)
            asset.write_bytes(b'asset')
            target = out / 'glb' / name / 'SM_Test.glb'
            target.parent.mkdir(parents=True)
            target.write_bytes(glb())
            tool = {'Cue4Inspect.dll': 'hash'}
            record = {'status': 'ok', 'input': m.input_fingerprint(root, name), 'tool': tool,
                      'files': [{'name': target.name, 'sha256': m.digest(target)}]}
            for suffix in ('.uptnl', '.m.ubulk'):
                sidecar = root / (name + suffix)
                sidecar.write_bytes(b'optional')
                self.assertFalse(m.verified(record, root, out, tool, name))
                record['input'] = m.input_fingerprint(root, name)
                self.assertTrue(m.verified(record, root, out, tool, name))
                sidecar.write_bytes(b'changed!')
                self.assertFalse(m.verified(record, root, out, tool, name))
                sidecar.unlink()
                record['input'] = m.input_fingerprint(root, name)

    def test_bad_glb_and_unsafe_package(self):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / 'bad.glb'
            path.write_bytes(glb()[:-1])
            with self.assertRaises(ValueError):
                m.validate_glb(path)
        for name in ('../SM_A', '/FortniteGame/Content/SM_A', 'FortniteGame/Content/../SM_A'):
            with self.assertRaises(ValueError):
                m.package_path(name)

if __name__ == '__main__':
    unittest.main()
