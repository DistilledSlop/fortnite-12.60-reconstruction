"""Profile response deltas must not share mutable state with stored profiles."""

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

from backend import Backend


class ProfileResponseOwnershipTests(unittest.TestCase):
    def test_mutated_deltas_cannot_change_cached_or_persisted_locker(self):
        with tempfile.TemporaryDirectory() as directory:
            service = Backend(state_dir=directory).profiles
            account = "account"
            profile = service.query(account, "athena")["profileChanges"][0]["profile"]
            locker = profile["stats"]["attributes"]["last_applied_loadout"]
            dance = next(item_id for item_id, item in profile["items"].items()
                         if item["templateId"].startswith("AthenaDance:"))
            response = service.run(account, "athena", "SetCosmeticLockerSlot", {
                "lockerItem": locker, "category": "Dance", "itemToSlot": dance, "slotIndex": 1,
            })
            slots = response["profileChanges"][0]["attributeValue"]["slots"]
            slots["Dance"]["items"][1] = "AthenaDance:tampered"

            copy_response = service.run(account, "athena", "CopyCosmeticLoadout", {
                "sourceIndex": 0, "targetIndex": 1,
            })
            loadouts = next(change["value"] for change in copy_response["profileChanges"]
                            if change["changeType"] == "statModified" and change["name"] == "loadouts")
            loadouts.clear()
            service.run(account, "athena", "SetCosmeticLockerName", {
                "lockerItem": locker, "name": "Saved",
            })

            cached = service.query(account, "athena")["profileChanges"][0]["profile"]
            restored = Backend(state_dir=directory).profiles.query(account, "athena")
            self.assertEqual(restored["profileChanges"][0]["profile"], cached)
            self.assertEqual(len(cached["stats"]["attributes"]["loadouts"]), 2)
            self.assertEqual(cached["items"][locker]["attributes"]["locker_slots_data"]
                             ["slots"]["Dance"]["items"][1], profile["items"][dance]["templateId"])


if __name__ == "__main__":
    unittest.main()
