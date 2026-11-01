"""Focused fail-closed checks for the frozen description runtime importer."""
import copy
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('record_description', Path(__file__).with_name('record_frontend_confirmation_description_runtime.py'))
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


class ReceiptTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.checkpoint = json.loads(m.read_pinned(m.CHECKPOINT, m.CHECKPOINT_SHA))
        cls.release = json.loads(m.read_pinned(m.RELEASE, m.RELEASE_SHA))

    def test_real_evidence(self):
        m.validate(self.checkpoint, self.release)

    def test_wrong_pin_or_missing_file_fails(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'pin'
            path.write_text('tampered')
            with self.assertRaises(ValueError):
                m.read_pinned(path, m.CHECKPOINT_SHA)
            with self.assertRaises(FileNotFoundError):
                m.read_pinned(path.with_name('missing'), m.CHECKPOINT_SHA)

    def test_release_and_build_failure_rejected(self):
        release = copy.deepcopy(self.release)
        release['checkpoint_sha256'] = '0' * 64
        with self.assertRaises(ValueError):
            m.validate(self.checkpoint, release)
        checkpoint = copy.deepcopy(self.checkpoint)
        checkpoint['builds'][0]['source_tree_unchanged'] = False
        with self.assertRaises(ValueError):
            m.validate(checkpoint, self.release)

    def test_source_tamper_rejected(self):
        source = self.checkpoint['source_files'][0]['archive']
        original = m.read_pinned
        def altered(path, digest):
            if path == source:
                raise ValueError('hash mismatch')
            return original(path, digest)
        with patch.object(m, 'read_pinned', side_effect=altered), self.assertRaises(ValueError):
            m.validate(self.checkpoint, self.release)

    def test_committed_source_tamper_rejected(self):
        with patch.object(m.subprocess, 'check_output', return_value=b'wrong committed source'), self.assertRaises(ValueError):
            m.validate(self.checkpoint, self.release)

    def test_false_process_receipt_rejected(self):
        row = next(x for x in self.checkpoint['artifacts'] if x['path'].endswith('/confirmation-description/receipt.json'))
        original = m.read_pinned
        def altered(path, digest):
            data = original(path, digest)
            if path == row['archive']:
                doc = json.loads(data)
                doc['proc_exe_inode_verified'] = False
                return json.dumps(doc).encode()
            return data
        with patch.object(m, 'read_pinned', side_effect=altered), self.assertRaises(ValueError):
            m.validate(self.checkpoint, self.release)

    def test_bad_transcript_rejected_even_if_receipts_claim_pass(self):
        row = next(x for x in self.checkpoint['artifacts'] if x['path'].endswith('/confirmation-description/runtime.log'))
        original = m.read_pinned
        def altered(path, digest):
            data = original(path, digest)
            if path == row['archive']:
                return data.replace(b'row=layout_values size=280', b'row=layout_values size=281')
            return data
        with patch.object(m, 'read_pinned', side_effect=altered), self.assertRaises(ValueError):
            m.validate(self.checkpoint, self.release)


if __name__ == '__main__':
    unittest.main()
