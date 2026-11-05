"""Copying over the selected loadout preserves the active locker reference."""
import tempfile
import unittest

from backend import Backend
from api_common import ApiError
from profile_service import ProfileTx


class CopyActiveLoadoutTests(unittest.TestCase):
    def test_duplicate_locker_ids_follow_selected_index(self):
        for active_index in (0, 1):
            with self.subTest(active_index=active_index):
                service = Backend().profiles
                account = "account"
                profile = service.profile(account, "athena")
                locker = profile["stats"]["attributes"]["last_applied_loadout"]
                tx = ProfileTx(service.ctx, profile)
                tx.set_stat("loadouts", [locker, locker])
                tx.set_stat("active_loadout_index", active_index)
                service._store(account, {"athena": tx})
                service.run(account, "athena", "CopyCosmeticLoadout", {
                    "sourceIndex": 1, "targetIndex": 0,
                })
                result = service.query(account, "athena")["profileChanges"][0]["profile"]
                stats = result["stats"]["attributes"]
                self.assertEqual(stats["last_applied_loadout"], stats["loadouts"][active_index])
                self.assertEqual(stats["loadouts"][1], locker)

    def test_overwriting_selected_loadout_keeps_subsequent_edits_on_it(self):
        with tempfile.TemporaryDirectory() as directory:
            service = Backend(state_dir=directory).profiles
            account = "account"
            service.run(account, "athena", "CopyCosmeticLoadout", {
                "sourceIndex": 0, "targetIndex": 1, "optNewNameForTarget": "Source",
            })
            before = service.query(account, "athena")["profileChanges"][0]["profile"]
            with self.assertRaises(ApiError):
                service.run(account, "athena", "CopyCosmeticLoadout", {
                    "sourceIndex": 99, "targetIndex": 0,
                })
            self.assertEqual(service.query(account, "athena")["profileChanges"][0]["profile"], before)
            response = service.run(account, "athena", "CopyCosmeticLoadout", {
                "sourceIndex": 1, "targetIndex": 0,
            })
            profile = service.query(account, "athena")["profileChanges"][0]["profile"]
            active = profile["stats"]["attributes"]["last_applied_loadout"]
            self.assertEqual(active, profile["stats"]["attributes"]["loadouts"][0])
            self.assertIn({"changeType": "statModified", "name": "last_applied_loadout", "value": active},
                          response["profileChanges"])
            dance = next(item_id for item_id, item in profile["items"].items()
                         if item["templateId"].startswith("AthenaDance:"))
            service.run(account, "athena", "EquipBattleRoyaleCustomization", {
                "slotName": "Dance", "itemToSlot": dance, "indexWithinSlot": 1,
            })
            restored = Backend(state_dir=directory).profiles.query(account, "athena")
            profile = restored["profileChanges"][0]["profile"]
            self.assertEqual(profile["stats"]["attributes"]["last_applied_loadout"], active)
            self.assertEqual(profile["items"][active]["attributes"]["locker_slots_data"]
                             ["slots"]["Dance"]["items"][1], profile["items"][dance]["templateId"])


if __name__ == "__main__":
    unittest.main()
