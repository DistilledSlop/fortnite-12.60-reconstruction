#!/usr/bin/env python3
"""Exercise receipt handling without a UE client or generated runtime artifacts."""
import contextlib
import importlib.util
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('recorder', Path(__file__).with_name('record_frontend_materials.py'))
recorder = importlib.util.module_from_spec(spec)
spec.loader.exec_module(recorder)


class ReceiptTests(unittest.TestCase):
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
            inputs.write_text('[]')
            components = root / 'components.json'
            components.write_text(json.dumps({'tree': [{'id': 'ui.material_frontend_closure', 'measure': {'checks': [ident]}}]}))
            evidence = root / 'evidence.json'
            evidence.write_text(json.dumps({'checks': {}, 'unrelated': 'preserved'}))
            with patch.multiple(recorder, MANIFEST_SHA256=recorder.digest(manifest), INPUTS=inputs,
                                COMPONENTS=components, EVIDENCE=evidence), \
                 patch.object(recorder, 'inventory_owners', return_value=[owner]), \
                 patch.object(recorder, 'report_for', side_effect=AssertionError('must not verify a failed run')), \
                 patch.object(sys, 'argv', ['recorder', '--manifest', str(manifest), '--receipt', str(receipt), '--inventory', str(inventory)]), \
                 contextlib.redirect_stdout(io.StringIO()):
                recorder.main()
            result = json.loads(evidence.read_text())
            self.assertEqual(result['checks'][ident]['result'], 'pending')
            self.assertEqual(result['checks'][ident]['owner_path'], owner)
            self.assertEqual(result['unrelated'], 'preserved')


if __name__ == '__main__':
    unittest.main()
