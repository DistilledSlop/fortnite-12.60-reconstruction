"""Offline local currency grants survive restart and respect the server lock."""
import fcntl
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from backend import Backend


class LocalCurrencyTests(unittest.TestCase):
    def test_credit_persists_and_refuses_active_state(self):
        with tempfile.TemporaryDirectory() as directory:
            cmd = [sys.executable, str(Path(__file__).with_name('local_currency.py')),
                   '--state-dir', directory, '--username', 'Buyer', '--amount', '1000']
            first = subprocess.run(cmd, capture_output=True, text=True)
            self.assertEqual(first.returncode, 0, first.stderr)
            backend = Backend(state_dir=directory)
            account = backend.accounts.get_or_create_username('Buyer')['id']
            core = backend.profiles.profile(account, 'common_core')
            self.assertEqual(next(iter(core['items'].values()))['quantity'], 1000)
            self.assertEqual(next(iter(core['items'].values()))['attributes']['platform'], 'EpicPC')
            with (Path(directory) / '.backend.lock').open('a') as lock:
                fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
                second = subprocess.run(cmd, capture_output=True, text=True)
            self.assertNotEqual(second.returncode, 0)
            self.assertIn('already in use', second.stderr)
            restored = Backend(state_dir=directory).profiles.profile(account, 'common_core')
            self.assertEqual(core, restored)


if __name__ == '__main__':
    unittest.main()
