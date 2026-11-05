"""Unsupported style changes must never be acknowledged or partly applied."""
import tempfile
import unittest

from api_common import ApiError
from backend import Backend


class ProfileVariantUpdateTests(unittest.TestCase):
    def test_nonempty_updates_fail_atomically_for_single_and_batch(self):
        with tempfile.TemporaryDirectory() as directory:
            service = Backend(state_dir=directory).profiles
            account = "account"
            before = service.query(account, "athena")["profileChanges"][0]["profile"]
            locker = before["stats"]["attributes"]["last_applied_loadout"]
            dance = next(key for key, item in before["items"].items()
                         if item["templateId"].startswith("AthenaDance:"))
            updates = [{"channel": "Material", "active": "Blue", "owned": ["Blue"]}]
            commands = [
                ("SetCosmeticLockerSlot", {"lockerItem": locker, "category": "Dance",
                                          "itemToSlot": dance, "slotIndex": 1,
                                          "variantUpdates": updates}),
                ("SetCosmeticLockerSlots", {"lockerItem": locker, "loadoutData": [
                    {"slotName": "Dance", "itemToSlot": dance, "indexWithinSlot": 1},
                    {"slotName": "Dance", "itemToSlot": dance, "indexWithinSlot": 2,
                     "variantUpdates": updates},
                ]}),
            ]
            for command, body in commands:
                with self.subTest(command=command), self.assertRaises(ApiError):
                    service.run(account, "athena", command, body)
                self.assertEqual(before, service.query(account, "athena")["profileChanges"][0]["profile"])
                restored = Backend(state_dir=directory).profiles.query(account, "athena")
                self.assertEqual(before, restored["profileChanges"][0]["profile"])


if __name__ == "__main__":
    unittest.main()
