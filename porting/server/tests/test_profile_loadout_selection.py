"""Loadout selection and deletion keep profile references usable and persistent."""

# Isolated mode omits the script directory from sys.path.
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import tempfile
import unittest

from api_common import ApiError
from backend import Backend
from profile_service import ProfileTx


class LoadoutSelectionTests(unittest.TestCase):
    def test_selection_deletion_and_persistence(self):
        with tempfile.TemporaryDirectory() as directory:
            service = Backend(state_dir=directory).profiles
            account = "account"
            for target in (1, 2):
                service.run(account, "athena", "CopyCosmeticLoadout", {
                    "sourceIndex": 0, "targetIndex": target,
                })
            ids = service.profile(account, "athena")["stats"]["attributes"]["loadouts"]
            response = service.run(account, "athena", "SetActiveLoadoutIndex", {"index": 2})
            self.assertIn({"changeType": "statModified", "name": "last_applied_loadout", "value": ids[2]},
                          response["profileChanges"])
            service.run(account, "athena", "DeleteCosmeticLoadout", {"index": 0, "fallbackLoadoutIndex": 1})
            profile = service.profile(account, "athena")
            self.assertEqual(profile["stats"]["attributes"]["active_loadout_index"], 1)
            self.assertEqual(profile["stats"]["attributes"]["last_applied_loadout"], ids[2])
            response = service.run(account, "athena", "DeleteCosmeticLoadout", {
                "index": 1, "fallbackLoadoutIndex": 0,
            })
            self.assertIn({"changeType": "itemRemoved", "itemId": ids[2]}, response["profileChanges"])
            restored = Backend(state_dir=directory).profiles.profile(account, "athena")
            self.assertEqual(restored["stats"]["attributes"]["loadouts"], [ids[1]])
            self.assertEqual(restored["stats"]["attributes"]["last_applied_loadout"], ids[1])
            self.assertEqual(restored["stats"]["attributes"]["active_loadout_index"], 0)
            self.assertNotIn(ids[2], restored["items"])

    def test_invalid_requests_are_atomic_and_duplicates_are_preserved(self):
        service = Backend().profiles
        account = "account"
        profile = service.profile(account, "athena")
        locker = profile["stats"]["attributes"]["last_applied_loadout"]
        tx = ProfileTx(service.ctx, profile)
        tx.set_stat("loadouts", [locker, locker])
        service._store(account, {"athena": tx})
        for name, body in (("SetActiveLoadoutIndex", {"index": 2}),
                           ("SetActiveLoadoutIndex", {"index": True}),
                           ("DeleteCosmeticLoadout", {"index": 0, "fallbackLoadoutIndex": 0}),
                           ("DeleteCosmeticLoadout", {"index": 0, "fallbackLoadoutIndex": 2})):
            before = service.profile(account, "athena")
            with self.subTest(name=name, body=body), self.assertRaises(ApiError):
                service.run(account, "athena", name, body)
            self.assertEqual(service.profile(account, "athena"), before)
        service.run(account, "athena", "DeleteCosmeticLoadout", {"index": 0, "fallbackLoadoutIndex": 1})
        profile = service.profile(account, "athena")
        self.assertIn(locker, profile["items"])
        self.assertEqual(profile["stats"]["attributes"]["last_applied_loadout"], locker)


if __name__ == "__main__":
    unittest.main()
