#!/usr/bin/env python3
"""Guard receipt pins, source-only claims and preserved effort allocation."""
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

folder = Path(__file__).parent
spec = importlib.util.spec_from_file_location('stages', folder / 'record_frontend_source_stages.py')
stages = importlib.util.module_from_spec(spec)
spec.loader.exec_module(stages)

class SourceStageTests(unittest.TestCase):
    def test_frozen_bytes_cannot_be_missing_or_changed(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            path = root / 'source.cpp'
            path.write_text('original')
            rows = [{'path': path.name, 'sha256': hashlib.sha256(path.read_bytes()).hexdigest(), 'size': path.stat().st_size}]
            self.assertEqual(stages.verify_files(root, rows), 1)
            path.write_text('modified')
            with self.assertRaises(ValueError):
                stages.verify_files(root, rows)
            path.unlink()
            with self.assertRaises(FileNotFoundError):
                stages.verify_files(root, rows)

    def test_receipt_hash_and_tampering(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'receipt.json'
            path.write_text('{}')
            sha = hashlib.sha256(path.read_bytes()).hexdigest()
            self.assertEqual(stages.read_receipt(path, sha), {})
            path.write_text('{"changed": true}')
            with self.assertRaises(ValueError):
                stages.read_receipt(path, sha)

    def test_context_runtime_cannot_be_claimed(self):
        receipt = {'passed': True, 'root_portable_core_test': {'passed': True},
                   'generator_policy_tests': 8, 'source_bindings_verified': 30984,
                   'source_review': {'derived_context_behavior_claim': False},
                   'fresh_uht': False, 'ue_build': False, 'ue_runtime': False}
        stages.validate('frontend_context_source_stage', receipt)
        receipt['ue_runtime'] = True
        with self.assertRaises(AssertionError):
            stages.validate('frontend_context_source_stage', receipt)

    def test_stage_allocations_and_unfinished_work(self):
        model = json.loads((folder / 'components.json').read_text())
        ui = next(group for group in model['tree'] if group['id'] == 'ui')
        leaves = {leaf['id']: leaf for leaf in ui['children']}
        for name, check in [('context', 'frontend_context_source_stage'), ('panel', 'frontend_panel_base_source_stage'), ('input', 'frontend_input_source_stage')]:
            leaf = leaves['ui.' + name + '_source_stage']
            self.assertEqual(leaf['weight'], 1)
            self.assertEqual(leaf['measure']['checks'], [check])
        self.assertEqual(leaves['ui.lifecycle']['weight'] + leaves['ui.context_source_stage']['weight'] + leaves['ui.context_runtime_stage']['weight'] + leaves['ui.startup_selection_runtime']['weight'], 14)
        self.assertEqual(leaves['ui.commonui_umg.implementation']['weight'] + leaves['ui.commonui_activation_runtime']['weight'] + leaves['ui.commonui_button_internal_runtime']['weight'] + leaves['ui.commonui_panel_intro_runtime']['weight'] + leaves['ui.commonui_panel_outro_runtime']['weight'] + leaves['ui.panel_source_stage']['weight'] + leaves['ui.input_source_stage']['weight'] + leaves['ui.commonui_button_foundation_runtime']['weight'] + leaves['ui.commonui_input_foundation_runtime']['weight'] + leaves['ui.commonui_binding_data_runtime']['weight'], 38)
        self.assertTrue(all(not gate['done'] for gate in model['milestones']))

if __name__ == '__main__':
    unittest.main()
