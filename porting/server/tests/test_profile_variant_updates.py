"""Locker styles are validated against owned variants and committed atomically."""

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


class ProfileVariantUpdateTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.service = Backend(state_dir=self.directory.name).profiles
        self.account = "account"
        profile = self.profile()
        self.locker = profile["stats"]["attributes"]["last_applied_loadout"]
        self.item = self.service.grant_item(self.account, "athena", "AthenaCharacter:style_test", {
            "variants": [{"channel": "Progressive", "active": "Stage0", "owned": ["Stage0", "Stage1"]}]})
        self.update = {"channel": "Progressive", "active": "Stage1", "owned": []}

    def profile(self):
        return self.service.profile(self.account, "athena")

    def slot(self, profile):
        return profile["items"][self.locker]["attributes"]["locker_slots_data"]["slots"]["Character"]

    def body(self, **changes):
        body = {"lockerItem": self.locker, "category": "Character", "itemToSlot": self.item,
                "slotIndex": 0, "variantUpdates": [self.update]}
        body.update(changes)
        return body

    def test_save_response_noop_and_restart(self):
        before = self.profile()
        response = self.service.run(self.account, "athena", "SetCosmeticLockerSlot", self.body())
        profile = self.profile()
        variants = [{"channel": "Progressive", "active": "Stage1", "owned": ["Stage0", "Stage1"]}]
        self.assertEqual(profile["items"][self.item]["attributes"]["variants"], variants)
        self.assertEqual(self.slot(profile)["activeVariants"][0], {"variants": variants})
        self.assertEqual(profile["rvn"], before["rvn"] + 1)
        self.assertEqual({change["itemId"] for change in response["profileChanges"]}, {self.item, self.locker})
        self.assertEqual(profile, Backend(state_dir=self.directory.name).profiles.profile(self.account, "athena"))
        again = self.service.run(self.account, "athena", "SetCosmeticLockerSlot", self.body())
        self.assertEqual(again["profileChanges"], [])
        self.assertEqual(self.profile()["rvn"], profile["rvn"])

    def test_slot_alignment_and_clear(self):
        self.service.run(self.account, "athena", "SetCosmeticLockerSlot", self.body(slotIndex=3))
        slot = self.slot(self.profile())
        self.assertEqual(len(slot["items"]), len(slot["activeVariants"]))
        self.assertEqual(slot["activeVariants"][2], {"variants": []})
        self.service.run(self.account, "athena", "SetCosmeticLockerSlot", self.body(
            itemToSlot="", slotIndex=3, variantUpdates=[]))
        self.assertEqual(self.slot(self.profile())["activeVariants"][3], {"variants": []})

    def test_invalid_updates_and_client_claimed_ownership_are_rejected(self):
        bad_updates = [
            [{"channel": "Progressive", "active": "Stage2", "owned": ["Stage2"]}],
            [{"channel": "Missing", "active": "Stage1", "owned": []}],
            [self.update, self.update], [None], "invalid",
        ]
        for updates in bad_updates:
            before = self.profile()
            with self.subTest(updates=updates), self.assertRaises(ApiError):
                self.service.run(self.account, "athena", "SetCosmeticLockerSlot", self.body(variantUpdates=updates))
            self.assertEqual(self.profile(), before)
            self.assertEqual(Backend(state_dir=self.directory.name).profiles.profile(self.account, "athena"), before)

    def test_batch_rolls_back_prior_valid_style(self):
        before = self.profile()
        entries = [
            {"slotName": "Character", "itemToSlot": self.item, "indexWithinSlot": 0,
             "variantUpdates": [self.update]},
            {"slotName": "Character", "itemToSlot": self.item, "indexWithinSlot": 1,
             "variantUpdates": [{"channel": "Progressive", "active": "Bad", "owned": ["Bad"]}]},
        ]
        with self.assertRaises(ApiError):
            self.service.run(self.account, "athena", "SetCosmeticLockerSlots",
                             {"lockerItem": self.locker, "loadoutData": entries})
        self.assertEqual(self.profile(), before)
        self.assertEqual(Backend(state_dir=self.directory.name).profiles.profile(self.account, "athena"), before)


if __name__ == "__main__":
    unittest.main()
