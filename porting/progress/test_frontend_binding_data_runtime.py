import copy
import importlib.util
from pathlib import Path
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('binding_data', Path(__file__).with_name('record_frontend_binding_data_runtime.py'))
f = importlib.util.module_from_spec(spec)
spec.loader.exec_module(f)

class BindingDataEvidenceTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls): cls.checkpoint = f.load_release()

 def test_frozen_evidence_passes(self): f.validate(self.checkpoint)

 def test_missing_artifact_fails(self):
  c = copy.deepcopy(self.checkpoint); c['artifacts'].pop()
  with self.assertRaises((ValueError, KeyError, FileNotFoundError)): f.validate(c)

 def test_tampered_process_receipt_fails(self):
  c = copy.deepcopy(self.checkpoint); c['binding_data']['proc_exe_inode_verified'] = False
  with self.assertRaises(ValueError): f.validate(c)

 def test_tampered_artifact_fails_hash(self):
  row = self.checkpoint['artifacts'][0]; original = Path.read_bytes
  def corrupt(path):
   data = original(path)
   return data + b'tamper' if str(path) == row['archive'] else data
  with patch.object(Path, 'read_bytes', corrupt), self.assertRaisesRegex(ValueError, 'hash mismatch'): f.validate(self.checkpoint)

 def test_wrong_historical_source_fails(self):
  original = f.committed
  with patch.object(f, 'committed', side_effect=lambda path: original(path) + b'tamper'), self.assertRaisesRegex(ValueError, 'committed source mismatch'): f.load_release()

 def test_verifier_rejects_missing_rows(self):
  with self.assertRaises(ValueError): f.verifier('binding_data_verifier.py', '')

if __name__ == '__main__': unittest.main()
