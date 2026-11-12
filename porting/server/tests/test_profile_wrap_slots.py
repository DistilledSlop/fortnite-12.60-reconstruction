"""Observed ItemWrap slot indices and apply-to-all requests."""

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


class WrapSlotTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.service = Backend(state_dir=self.directory.name).profiles
        self.account = "wrap-account"
        self.locker = self.profile()["stats"]["attributes"]["last_applied_loadout"]
        self.template = "AthenaItemWrap:wrap_test"
        self.item = self.service.grant_item(self.account, "athena", self.template, {
            "variants": [{"channel": "Color", "active": "Red", "owned": ["Red", "Blue"]}]})
        self.update = {"channel": "Color", "active": "Blue", "owned": ["Blue", "Forged"]}

    def profile(self):
        return self.service.profile(self.account, "athena")

    def slot(self):
        return self.profile()["items"][self.locker]["attributes"]["locker_slots_data"]["slots"]["ItemWrap"]

    def equip(self, **changes):
        body = {"lockerItem": self.locker, "category": "ItemWrap", "itemToSlot": self.item,
                "slotIndex": -1, "variantUpdates": [self.update]}
        body.update(changes)
        return self.service.run(self.account, "athena", "SetCosmeticLockerSlot", body)

    def test_apply_all_response_noop_persistence_and_clear(self):
        before = self.profile()
        response = self.equip()
        variants = [{"channel": "Color", "active": "Blue", "owned": ["Red", "Blue"]}]
        self.assertEqual(self.slot(), {"items": [self.template] * 7,
                                      "activeVariants": [{"variants": variants}] * 7})
        after = self.profile()
        self.assertEqual(after["rvn"], before["rvn"] + 1)
        self.assertEqual({c["itemId"] for c in response["profileChanges"]}, {self.item, self.locker})
        self.assertEqual(after, Backend(state_dir=self.directory.name).profiles.profile(self.account, "athena"))
        self.assertEqual(self.equip()["profileChanges"], [])
        self.assertEqual(self.profile()["rvn"], after["rvn"])
        self.equip(itemToSlot="", variantUpdates=[])
        self.assertEqual(self.slot(), {"items": [""] * 7,
                                      "activeVariants": [{"variants": []}] * 7})

    def test_index_six_and_other_supported_commands(self):
        self.equip(slotIndex=6, variantUpdates=[])
        self.assertEqual(self.slot()["items"], [""] * 6 + [self.template])
        self.assertEqual(len(self.slot()["items"]), len(self.slot()["activeVariants"]))
        self.service.run(self.account, "athena", "SetCosmeticLockerSlots", {
            "lockerItem": self.locker, "loadoutData": [
                {"slotName": "ItemWrap", "itemToSlot": self.item, "indexWithinSlot": 1}]})
        self.service.run(self.account, "athena", "EquipBattleRoyaleCustomization", {
            "slotName": "ItemWrap", "itemToSlot": self.item, "indexWithinSlot": 6})
        self.assertEqual(self.slot()["items"][1], self.template)

    def test_invalid_requests_are_atomic_and_do_not_trust_client(self):
        invalid = [
            {"slotIndex": -2}, {"slotIndex": 7}, {"slotIndex": True},
            {"category": "Character", "slotIndex": -1},
            {"category": "Character", "slotIndex": 6},
            {"category": "Unknown"},
            {"itemToSlot": "AthenaItemWrap:not_owned"},
            {"itemToSlot": "AthenaCharacter:cid_001_athena_commando_f_default"},
            {"variantUpdates": [{"channel": "Color", "active": "Forged", "owned": ["Forged"]}]},
            {"itemToSlot": "", "variantUpdates": [self.update]},
        ]
        for changes in invalid:
            with self.subTest(changes=changes):
                before = self.profile()
                with self.assertRaises(ApiError):
                    self.equip(**changes)
                self.assertEqual(self.profile(), before)
                self.assertEqual(Backend(state_dir=self.directory.name).profiles.profile(self.account, "athena"), before)

    def test_batch_failure_rolls_back_wrap_changes(self):
        before = self.profile()
        with self.assertRaises(ApiError):
            self.service.run(self.account, "athena", "SetCosmeticLockerSlots", {
                "lockerItem": self.locker, "loadoutData": [
                    {"slotName": "ItemWrap", "itemToSlot": self.item, "indexWithinSlot": 6},
                    {"slotName": "ItemWrap", "itemToSlot": "AthenaItemWrap:missing", "indexWithinSlot": 1}]})
        self.assertEqual(self.profile(), before)
        self.assertEqual(Backend(state_dir=self.directory.name).profiles.profile(self.account, "athena"), before)


if __name__ == "__main__":
    unittest.main()
