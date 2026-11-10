"""Local catalog purchases keep wallet and cosmetics in one transaction."""
import copy
import sqlite3
import tempfile
import unittest
from unittest.mock import patch

from api_common import ApiError
from backend import Backend
from profile_service import ProfileTx


OFFER = {
    "prices": [{"currencyType": "MtxCurrency", "currencySubType": "", "finalPrice": 500}],
    "itemGrants": [{"templateId": "AthenaDance:eid_test", "quantity": 1},
                   {"templateId": "AthenaBackpack:bid_test", "quantity": 1}],
}
BODY = {"offerId": "test", "purchaseQuantity": 1, "currency": "MtxCurrency",
        "currencySubType": "", "expectedTotalPrice": 500}


class PurchaseTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.service = Backend(state_dir=self.directory.name).profiles
        core = self.service.profile("account", "common_core")
        self.wallet = next(iter(core["items"]))
        tx = ProfileTx(self.service.ctx, core)
        tx.set_item_quantity(self.wallet, 700)
        self.service._store("account", {"common_core": tx})
        self.service.profile("account", "athena")
        self.lookup = patch("profile_service.get_offer", side_effect=lambda key: copy.deepcopy(OFFER) if key == "test" else None)
        self.lookup.start()
        self.addCleanup(self.lookup.stop)

    def purchase(self, body=BODY):
        return self.service.run("account", "common_core", "PurchaseCatalogEntry", body)

    def snapshots(self):
        return [self.service.profile("account", profile) for profile in ("common_core", "athena")]

    def test_purchase_and_restart_preserve_wallet_and_grants(self):
        before = self.snapshots()
        result = self.purchase()
        after = self.snapshots()
        self.assertEqual(after[0]["items"][self.wallet]["quantity"], 200)
        self.assertEqual(after[0]["rvn"], before[0]["rvn"] + 1)
        self.assertEqual(after[1]["rvn"], before[1]["rvn"] + 1)
        self.assertEqual(result["profileChanges"][0]["changeType"], "itemQuantityChanged")
        self.assertTrue(result["notifications"][0]["primary"])
        loot = result["notifications"][0]["lootResult"]["items"]
        self.assertEqual([item["itemType"] for item in loot], [g["templateId"] for g in OFFER["itemGrants"]])
        self.assertEqual(result["multiUpdate"][0]["profileId"], "athena")
        self.assertEqual([c["changeType"] for c in result["multiUpdate"][0]["profileChanges"]],
                         ["itemAdded", "itemAdded"])
        templates = {item["templateId"] for item in after[1]["items"].values()}
        self.assertTrue({grant["templateId"] for grant in OFFER["itemGrants"]} <= templates)
        restored = Backend(state_dir=self.directory.name).profiles
        self.assertEqual(after, [restored.profile("account", p) for p in ("common_core", "athena")])
        with self.assertRaises(ApiError):
            self.purchase()
        self.assertEqual(after, self.snapshots())

    def test_published_offer_can_be_purchased(self):
        from catalog import storefronts
        offer = storefronts()[0]["catalogEntries"][0]
        self.lookup.stop()
        body = dict(BODY, offerId=offer["offerId"], expectedTotalPrice=offer["prices"][0]["finalPrice"])
        self.purchase(body)
        core, athena = self.snapshots()
        self.assertEqual(core["items"][self.wallet]["quantity"], 700 - body["expectedTotalPrice"])
        self.assertIn(offer["itemGrants"][0]["templateId"],
                      {item["templateId"] for item in athena["items"].values()})

    def test_invalid_requests_do_not_change_either_profile(self):
        cases = [dict(BODY, offerId="unknown"), dict(BODY, expectedTotalPrice=0),
                 dict(BODY, expectedTotalPrice=True), dict(BODY, purchaseQuantity=2),
                 dict(BODY, currency="RealMoney"), dict(BODY, currencySubType="x")]
        before = self.snapshots()
        for body in cases:
            with self.subTest(body=body), self.assertRaises(ApiError):
                self.purchase(body)
            self.assertEqual(before, self.snapshots())
        with patch("profile_service.get_offer", return_value={**OFFER, "prices": [
                {"currencyType": "MtxCurrency", "currencySubType": "", "finalPrice": 800}]}):
            with self.assertRaises(ApiError):
                self.purchase(dict(BODY, expectedTotalPrice=800))
        self.assertEqual(before, self.snapshots())

    def test_partly_owned_bundle_and_invalid_grant_are_atomic(self):
        self.service.grant_item("account", "athena", "ATHENADANCE:EID_TEST")
        before = self.snapshots()
        with self.assertRaises(ApiError):
            self.purchase()
        self.assertEqual(before, self.snapshots())
        bad = {**OFFER, "itemGrants": [OFFER["itemGrants"][1], {"templateId": "Currency:MtxGiveaway", "quantity": 1}]}
        with patch("profile_service.get_offer", return_value=bad), self.assertRaises(ApiError):
            self.purchase()
        self.assertEqual(before, self.snapshots())

    def test_database_failure_rolls_back_both_profiles_and_cache(self):
        before = self.snapshots()
        with sqlite3.connect(self.service._database) as db:
            db.execute("CREATE TRIGGER fail_athena BEFORE INSERT ON profiles WHEN NEW.profile_id = 'athena' BEGIN SELECT RAISE(ABORT, 'test failure'); END")
        with self.assertRaises(sqlite3.IntegrityError):
            self.purchase()
        self.assertEqual(before, self.snapshots())
        restored = Backend(state_dir=self.directory.name).profiles
        self.assertEqual(before, [restored.profile("account", p) for p in ("common_core", "athena")])


if __name__ == "__main__":
    unittest.main()
