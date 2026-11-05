"""Process-level ownership of persistent probe-server state."""

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


SERVER = Path(__file__).with_name("probe_server.py")


class StateLockTests(unittest.TestCase):
    def test_second_server_is_rejected_and_lock_is_released(self):
        with tempfile.TemporaryDirectory() as directory:
            command = [sys.executable, "-I", str(SERVER), "--stub", "--port", "0",
                       "--state-dir", directory, "--record", str(Path(directory) / "requests")]
            first = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            try:
                self.assertIn("Listening", first.stdout.readline())
                second = subprocess.run(command, capture_output=True, text=True, timeout=10)
                self.assertNotEqual(second.returncode, 0)
                self.assertIn("state directory is already in use", second.stderr)
            finally:
                first.terminate()
                first.communicate(timeout=10)
            third = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            try:
                self.assertIn("Listening", third.stdout.readline())
            finally:
                third.terminate()
                third.communicate(timeout=10)


if __name__ == "__main__":
    unittest.main()
