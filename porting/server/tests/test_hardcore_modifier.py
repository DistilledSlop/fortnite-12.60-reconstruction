"""Hardcore modifier updates are transactional and persist as profile item state."""
if __package__:
    from . import _bootstrap
else:
    import sys
    from pathlib import Path
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    import _bootstrap

import tempfile
import unittest

from api_common import ApiError
from backend import Backend

class HardcoreModifierTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.service = Backend(state_dir=self.directory.name).profiles
        self.account = "account"
        self.item = self.service.grant_item(self.account, "athena", "HardcoreModifier:test", {"is_enabled": False})
        self.other = self.service.grant_item(self.account, "athena", "AthenaCharacter:test", {})

    def profile(self):
        return self.service.profile(self.account, "athena")

    def run_update(self, updates, rvn=None):
        return self.service.run(self.account, "athena", "SetHardcoreModifier", {"updates": updates}, rvn)

    def test_empty_and_persistent_enable_disable(self):
        before = self.profile()
        self.assertEqual(self.run_update([])["profileChanges"], [])
        self.assertEqual(self.profile()["rvn"], before["rvn"])
        response = self.run_update([{"modifierId": self.item, "bEnabled": True}], str(before["rvn"]))
        self.assertEqual(response["profileChanges"], [{"changeType": "itemAttrChanged", "itemId": self.item,
                                                       "attributeName": "is_enabled", "attributeValue": True}])
        self.assertEqual(response["profileChangesBaseRevision"], before["rvn"])
        self.assertTrue(Backend(state_dir=self.directory.name).profiles.profile(self.account, "athena")
                        ["items"][self.item]["attributes"]["is_enabled"])
        self.assertEqual(self.run_update([{"modifierId": self.item, "bEnabled": True}])["profileChanges"], [])
        self.run_update([{"modifierId": self.item, "bEnabled": False}])
        self.assertFalse(self.profile()["items"][self.item]["attributes"]["is_enabled"])

    def test_invalid_batch_is_atomic(self):
        valid = {"modifierId": self.item, "bEnabled": True}
        for invalid in (None, {}, {"modifierId": self.item, "bEnabled": 1},
                        {"modifierId": "missing", "bEnabled": True},
                        {"modifierId": self.other, "bEnabled": True}, valid):
            before = self.profile()
            with self.subTest(invalid=invalid), self.assertRaises(ApiError):
                self.run_update([valid, invalid])
            self.assertEqual(self.profile(), before)
            self.assertEqual(Backend(state_dir=self.directory.name).profiles.profile(self.account, "athena"), before)
        for body in ({}, {"updates": None}, {"updates": {}}):
            with self.assertRaises(ApiError):
                self.service.run(self.account, "athena", "SetHardcoreModifier", body)


if __name__ == "__main__":
    unittest.main()
