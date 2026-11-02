import json
from pathlib import Path
import subprocess
import tempfile
import unittest

from stage_native_config import digest, stage

CONFIG = b"""[/Script/EngineSettings.GameMapsSettings]
GameInstanceClass=/Script/FortniteGame.FortGameInstance
[/Script/Engine.Engine]
GameViewportClientClassName=/Script/FortniteGame.FortGameViewportClient
LocalPlayerClassName=/Script/FortniteGame.FortLocalPlayer
"""
# UE's actual section is EngineSettings, not Engine.


class StageTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.repo = self.root / "repo"
        self.repo.mkdir()
        self.git("init", "-q")
        self.git("config", "user.email", "test@example.invalid")
        self.git("config", "user.name", "Test")
        self.source = self.repo / "Config" / "DefaultEngine.ini"
        self.source.parent.mkdir()
        self.source.write_bytes(CONFIG)
        self.git("add", "Config/DefaultEngine.ini")
        self.git("commit", "-qm", "config")
        self.commit = self.git("rev-parse", "HEAD").decode().strip()
        self.target = self.root / "stage" / "DefaultEngine.ini"
        self.target.parent.mkdir()
        self.target.write_bytes(b"old staged bytes\n")
        self.original = self.root / "original.ini"
        self.original.write_bytes(CONFIG + b"; private unrelated config omitted from receipt\n")
        self.out = self.root / "evidence"

    def git(self, *args):
        return subprocess.check_output(["git", "-C", str(self.repo), *args])

    def run_stage(self):
        return stage(self.source, self.target, self.out, self.commit, self.original)

    def test_copy_and_receipt(self):
        receipt = self.run_stage()
        self.assertEqual(self.target.read_bytes(), CONFIG)
        self.assertEqual((self.out / "DefaultEngine.ini.before").read_bytes(), b"old staged bytes\n")
        self.assertEqual(receipt, json.loads((self.out / "receipt.json").read_text()))
        self.assertEqual(receipt["source_commit"], self.commit)
        self.assertEqual(receipt["before_sha256"], receipt["backup_sha256"])
        self.assertEqual(receipt["after_sha256"], digest(CONFIG))
        self.assertEqual(receipt["source_sha256"], receipt["committed_source_sha256"])
        self.assertTrue(receipt["original_settings_validated"])
        self.assertNotIn("private unrelated", json.dumps(receipt))

    def test_rejections_leave_target_intact(self):
        for case in ("dirty", "bad_original", "duplicate_original", "existing_out", "missing_target", "same_target"):
            with self.subTest(case=case):
                self.source.write_bytes(CONFIG + (b"; dirty\n" if case == "dirty" else b""))
                self.original.write_bytes(CONFIG.replace(b"FortLocalPlayer", b"Wrong") if case == "bad_original" else CONFIG + (b"LocalPlayerClassName=/Script/FortniteGame.FortLocalPlayer\n" if case == "duplicate_original" else b""))
                if case == "existing_out":
                    self.out.mkdir()
                if case == "missing_target":
                    self.target.unlink()
                with self.assertRaises((ValueError, OSError)):
                    stage(self.source, self.source if case == "same_target" else self.target, self.out, self.commit, self.original)
                if case != "missing_target":
                    self.assertEqual(self.target.read_bytes(), b"old staged bytes\n")
                else:
                    self.target.write_bytes(b"old staged bytes\n")
                if self.out.exists():
                    self.out.rmdir()

    def test_bad_committed_settings_rejected(self):
        self.source.write_bytes(CONFIG.replace(b"FortGameInstance", b"Wrong"))
        self.git("add", "Config/DefaultEngine.ini")
        self.git("commit", "-qm", "bad config")
        self.commit = self.git("rev-parse", "HEAD").decode().strip()
        with self.assertRaises(ValueError):
            self.run_stage()
        self.assertFalse(self.out.exists())
        self.assertEqual(self.target.read_bytes(), b"old staged bytes\n")


if __name__ == "__main__":
    unittest.main()
