#!/usr/bin/env python3
"""Focused transactional and provenance checks using isolated synthetic fixtures."""
import importlib.util
import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

HERE = Path(__file__).resolve().parent
SPEC = importlib.util.spec_from_file_location('stage_gameplay_tags', HERE / 'stage_gameplay_tags.py')
STAGE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(STAGE)


class StagingTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(dir=HERE)
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.src = self.root / 'source-game'
        self.native = self.root / 'native'
        self.dst = self.root / 'stage'
        self.manifest = self.root / 'manifest.json'
        self.src.mkdir()
        (self.native / 'Config/Tags').mkdir(parents=True)
        files = []
        for relative, data in [('Config/DefaultGameplayTags.ini', b'[tags]'), ('Content/First.uasset', b'header'), ('Content/Last.uexp', b'rows')]:
            target = (self.native if relative.startswith('Config/') else self.src) / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
            files.append({'path': relative, 'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()})
        event = 'generator_policies/native_gameplay_tags_runtime/constructor_events.json'
        target = self.root / event
        target.parent.mkdir(parents=True)
        target.write_bytes(b'original native events')
        manifest = {'files': files, 'native_constructor_events': {'path': event, 'sha256': STAGE.sha256(target)}, 'runtime_parity': 'UNVERIFIED'}
        self.manifest.write_text(json.dumps(manifest))
        self.manifest_patch = patch.object(STAGE, 'MANIFEST_SHA256', STAGE.sha256(self.manifest))
        self.manifest_patch.start()
        self.addCleanup(self.manifest_patch.stop)

    def run_stage(self, check_only=False):
        return STAGE.stage(self.src, self.dst, self.native, self.manifest, check_only)

    def test_check_only_does_not_create_destination(self):
        result = self.run_stage(True)
        self.assertEqual(result['validated_files'], 3)
        self.assertEqual(result['copied_files'], 0)
        self.assertFalse(self.dst.exists())

    def test_stage_missing_and_idempotent(self):
        self.assertEqual(self.run_stage()['copied_files'], 3)
        self.assertEqual(self.run_stage()['copied_files'], 0)
        for entry in STAGE.load_manifest(self.manifest)['files']:
            STAGE.checked_file(self.dst / entry['path'], entry)

    def test_late_source_corruption_prevents_every_copy(self):
        entry = STAGE.load_manifest(self.manifest)['files'][-1]
        (self.src / entry['path']).write_bytes(b'bad table')
        with self.assertRaises(ValueError):
            self.run_stage()
        self.assertFalse(self.dst.exists())

    def test_existing_stage_conflict_preserved_before_any_copy(self):
        entry = STAGE.load_manifest(self.manifest)['files'][-1]
        target = self.dst / entry['path']
        target.parent.mkdir(parents=True)
        target.write_bytes(b'existing divergent table')
        with self.assertRaises(ValueError):
            self.run_stage()
        self.assertEqual([p for p in self.dst.rglob('*') if p.is_file()], [target])
        self.assertEqual(target.read_bytes(), b'existing divergent table')

    def test_unexpected_native_tag_config_rejected(self):
        (self.native / 'Config/Tags/Extra.ini').write_text('[GameplayTags]\n')
        with self.assertRaises(ValueError):
            self.run_stage()
        self.assertFalse(self.dst.exists())

    def test_native_event_change_rejected(self):
        event = STAGE.load_manifest(self.manifest)['native_constructor_events']['path']
        (self.root / event).write_text('[]')
        with self.assertRaises(ValueError):
            self.run_stage()
        self.assertFalse(self.dst.exists())

    def test_destination_parent_file_rejected_before_copy(self):
        self.dst.mkdir()
        (self.dst / 'Content').write_text('blocking file')
        with self.assertRaises(ValueError):
            self.run_stage()
        self.assertFalse((self.dst / 'Config').exists())

    def test_manifest_tampering_rejected(self):
        self.manifest.write_text('{}')
        with self.assertRaises(ValueError):
            self.run_stage()
        self.assertFalse(self.dst.exists())

    def test_unexpected_stage_tag_ini_rejected(self):
        tags = self.dst / 'Config/Tags'
        tags.mkdir(parents=True)
        (tags / 'Extra.ini').write_text('[GameplayTags]')
        with self.assertRaises(ValueError):
            self.run_stage()
        self.assertFalse((self.dst / 'Content').exists())

    def test_symlinked_tag_directory_rejected(self):
        (self.native / 'Config/Tags/Hidden').symlink_to(self.src, target_is_directory=True)
        with self.assertRaises(ValueError):
            self.run_stage()
        self.assertFalse(self.dst.exists())


if __name__ == '__main__':
    unittest.main()
