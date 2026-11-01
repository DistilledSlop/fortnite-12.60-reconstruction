#!/usr/bin/env python3
"""Exercise receipt handling without a UE client or generated runtime artifacts."""
import contextlib
import importlib.util
import io
import json
import hashlib
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('recorder', Path(__file__).with_name('record_frontend_materials.py'))
recorder = importlib.util.module_from_spec(spec)
spec.loader.exec_module(recorder)


class ReceiptTests(unittest.TestCase):
    def test_frozen_native_source_uses_commit_not_modified_worktree(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / 'porting/native/code.cpp'
            source.parent.mkdir(parents=True)
            source.write_bytes(b'new worktree bytes')
            manifest = root / 'manifest.json'
            manifest.write_bytes(b'manifest')
            inputs = root / 'inputs.json'
            inputs.write_text(json.dumps([
                {'path': str(source), 'sha256': hashlib.sha256(b'committed bytes').hexdigest()},
                {'path': str(manifest), 'sha256': recorder.digest(manifest)}]))
            old = Path.cwd()
            try:
                os.chdir(root)
                with patch.object(recorder.subprocess, 'check_output', return_value=b'committed bytes') as git:
                    recorder.verify_frozen_inputs(inputs, manifest, recorder.digest(manifest), 'abc', 'def')
                    self.assertIn('abc:porting/native/code.cpp', git.call_args.args[0])
                with patch.object(recorder.subprocess, 'check_output', return_value=b'wrong committed bytes'):
                    with self.assertRaisesRegex(ValueError, 'frozen build input changed'):
                        recorder.verify_frozen_inputs(inputs, manifest, recorder.digest(manifest), 'abc', 'def')
            finally:
                os.chdir(old)

    def test_checkpoint_rejects_mismatched_staged_binary(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest, receipt, build = (root / name for name in ('manifest', 'receipt', 'build'))
            manifest.write_text('{}')
            receipt.write_text('[]')
            build.write_text(json.dumps({'exit_status': 0}))
            binaries = []
            for kind in ('executable', 'symbols', 'debug'):
                snapshot = root / kind
                snapshot.write_bytes(kind.encode())
                for role in ('source', 'stage', 'snapshot'):
                    binaries.append({'target': 'FortniteLinux', 'artifact_kind': kind, 'role': role,
                                     'path': str(snapshot), 'size': snapshot.stat().st_size,
                                     'sha256': recorder.digest(snapshot)})
            checkpoint = {'schema': 'fortnite.material-runtime-checkpoint.v1', 'native_commit': 'native',
                          'engine_commit': 'engine', 'manifest': {'path': str(manifest), 'sha256': recorder.digest(manifest)},
                          'manifest_sha256': recorder.digest(manifest),
                          'owner_run_receipt': {'path': str(receipt), 'sha256': recorder.digest(receipt)},
                          'build_receipt': {'path': str(build), 'sha256': recorder.digest(build)},
                          'probes': [], 'builds': [{'exit_status': 0}], 'source_files': [], 'binaries': binaries}
            checkpoint_path = root / 'checkpoint'
            checkpoint_path.write_text(json.dumps(checkpoint))
            with patch.object(recorder, 'git_commit', side_effect=lambda repo, rev: rev):
                args = (checkpoint_path, recorder.digest(checkpoint_path), receipt, manifest,
                        recorder.digest(manifest), 'native', 'engine')
                recorder.verify_checkpoint(*args)
                binaries[1]['sha256'] = 'wrong'
                checkpoint_path.write_text(json.dumps(checkpoint))
                args = (checkpoint_path, recorder.digest(checkpoint_path), receipt, manifest,
                        recorder.digest(manifest), 'native', 'engine')
                with self.assertRaisesRegex(ValueError, 'hashes differ'):
                    recorder.verify_checkpoint(*args)

    def test_nonzero_exit_remains_pending_even_if_manifest_lists_owner(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            owner = '/Game/UI/Test.Test'
            ident = recorder.owner_id(owner)
            manifest = root / 'manifest.json'
            manifest.write_text(json.dumps({'entries': [{'owner_path': owner}]}))
            receipt = root / 'receipt.json'
            receipt.write_text(json.dumps([{'owner': owner, 'exit_status': 1}]))
            inventory = root / 'inventory.json'
            inventory.write_text('{}')
            inputs = root / 'inputs.json'
            inputs.write_text(json.dumps([{'path': str(manifest), 'sha256': recorder.digest(manifest)}]))
            components = root / 'components.json'
            components.write_text(json.dumps({'tree': [{'id': 'ui.material_frontend_closure', 'measure': {'checks': [ident]}}]}))
            evidence = root / 'evidence.json'
            evidence.write_text(json.dumps({'checks': {}, 'unrelated': 'preserved'}))
            with patch.multiple(recorder, COMPONENTS=components, EVIDENCE=evidence), \
                 patch.object(recorder, 'git_commit', return_value='frozen-commit'), \
                 patch.object(recorder, 'verify_checkpoint', return_value=({'artifacts': [], 'source_files': []}, 'binary')), \
                 patch.object(recorder, 'inventory_owners', return_value=[owner]), \
                 patch.object(recorder, 'report_for', side_effect=AssertionError('must not verify a failed run')), \
                 patch.object(sys, 'argv', ['recorder', '--manifest', str(manifest), '--receipt', str(receipt), '--inventory', str(inventory), '--inputs', str(inputs), '--manifest-sha256', recorder.digest(manifest), '--source-native-commit', 'native', '--engine-commit', 'engine', '--checkpoint', 'checkpoint.json', '--checkpoint-sha256', 'digest']), \
                 contextlib.redirect_stdout(io.StringIO()):
                recorder.main()
            result = json.loads(evidence.read_text())
            self.assertEqual(result['checks'][ident]['result'], 'pending')
            self.assertEqual(result['checks'][ident]['owner_path'], owner)
            self.assertEqual(result['unrelated'], 'preserved')


if __name__ == '__main__':
    unittest.main()
