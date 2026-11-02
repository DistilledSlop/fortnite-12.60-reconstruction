import copy
import importlib.util
import json
from pathlib import Path
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('foundation', Path(__file__).with_name('record_frontend_foundation_runtime.py'))
f = importlib.util.module_from_spec(spec)
spec.loader.exec_module(f)

class FoundationEvidenceTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.checkpoint = f.load_release('common-input-current-type-runtime-1')

 def test_frozen_evidence_passes(self):
  f.check()

 def test_missing_artifact_fails(self):
  c = copy.deepcopy(self.checkpoint)
  c['artifacts'].pop()
  with self.assertRaises((ValueError, KeyError, FileNotFoundError)):
   f.validate(c)

 def test_tampered_process_receipt_fails(self):
  c = copy.deepcopy(self.checkpoint)
  c['current_input_type']['proc_exe_inode_verified'] = False
  with self.assertRaises(ValueError):
   f.validate(c)

 def test_tampered_frozen_artifact_fails_hash(self):
  row = self.checkpoint['artifacts'][0]
  original = Path.read_bytes
  def corrupt(path):
   data = original(path)
   return data + b'tamper' if str(path) == row['archive'] else data
  with patch.object(Path, 'read_bytes', corrupt), self.assertRaisesRegex(ValueError, 'hash mismatch'):
   f.validate(self.checkpoint)

 def test_wrong_historical_source_fails(self):
  original = f.committed
  with patch.object(f, 'committed', side_effect=lambda commit, path: original(commit, path) + b'tamper'), self.assertRaisesRegex(ValueError, 'committed source mismatch'):
   f.load_release('main-button-defaults-runtime-1')

 def test_diagnostic_verifier_rejects_incomplete_log(self):
  with self.assertRaises(ValueError):
   f.verifier('porting/frontend/button_internal_runtime_probe/current_input_type_verifier.py', '')

if __name__ == '__main__': unittest.main()
