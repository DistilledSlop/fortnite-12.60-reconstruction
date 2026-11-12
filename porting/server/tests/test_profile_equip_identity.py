"""Locker equipment accepts owned cosmetic instance or template IDs."""

# Isolated mode omits the script directory from sys.path.
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import unittest

from api_common import ApiError
from backend import Backend


class EquipIdentityTests(unittest.TestCase):
    def setUp(self):
        self.service = Backend().profiles
        self.account = "equip-account"
        profile = self.service.profile(self.account, "athena")
        self.locker = profile["stats"]["attributes"]["last_applied_loadout"]
        self.template = "AthenaBackpack:bid_001_bluesquire"
        self.instance = next(key for key, item in profile["items"].items()
                             if item["templateId"] == self.template)

    def equip(self, value, category="Backpack"):
        return self.service.run(self.account, "athena", "SetCosmeticLockerSlot", {
            "lockerItem": self.locker, "category": category,
            "itemToSlot": value, "slotIndex": 0,
        })

    def equipped(self):
        profile = self.service.profile(self.account, "athena")
        return profile["items"][self.locker]["attributes"]["locker_slots_data"]["slots"]["Backpack"]["items"][0]

    def test_template_and_instance_ids_resolve_to_owned_template(self):
        for value in (self.template, self.template.upper(), self.instance):
            with self.subTest(value=value):
                self.equip("")
                self.equip(value)
                self.assertEqual(self.equipped(), self.template)

    def test_unowned_and_wrong_category_are_rejected_atomically(self):
        for value, category in (("AthenaBackpack:not_owned", "Backpack"),
                                (self.template.lower(), "Character"),
                                (self.instance, "Character")):
            with self.subTest(value=value, category=category):
                before = self.service.profile(self.account, "athena")
                with self.assertRaises(ApiError):
                    self.equip(value, category)
                self.assertEqual(self.service.profile(self.account, "athena"), before)

    def test_batch_is_atomic_when_later_template_is_unowned(self):
        before = self.service.profile(self.account, "athena")
        with self.assertRaises(ApiError):
            self.service.run(self.account, "athena", "SetCosmeticLockerSlots", {
                "lockerItem": self.locker,
                "loadoutData": [
                    {"slotName": "Backpack", "itemToSlot": "", "indexWithinSlot": 0},
                    {"slotName": "Backpack", "itemToSlot": "AthenaBackpack:not_owned", "indexWithinSlot": 0},
                ],
            })
        self.assertEqual(self.service.profile(self.account, "athena"), before)


if __name__ == "__main__":
    unittest.main()
