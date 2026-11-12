"""Local Season 12 pass purchase; offer mapping and price are assumptions."""
import sqlite3
from contextlib import closing
import tempfile
import unittest
from unittest.mock import patch

from api_common import ApiError
from backend import Backend
from catalog import SEASON12_LEVEL_OFFER, SEASON12_PASS_OFFER, storefronts
from season12_rewards import PAID
from profile_service import ProfileTx

BODY = {"offerId": SEASON12_PASS_OFFER, "purchaseQuantity": 1, "currency": "MtxCurrency",
        "currencySubType": "", "expectedTotalPrice": 950}


class BattlePassPurchaseTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.service = Backend(state_dir=self.directory.name).profiles
        core = self.service.profile("account", "common_core")
        self.wallet = next(iter(core["items"]))
        tx = ProfileTx(self.service.ctx, core)
        tx.set_item_quantity(self.wallet, 1200)
        self.service._store("account", {"common_core": tx})
        self.service.profile("account", "athena")

    def snapshots(self, service=None):
        service = service or self.service
        return [service.profile("account", p) for p in ("common_core", "athena")]

    def purchase(self, body=BODY):
        return self.service.run("account", "common_core", "PurchaseCatalogEntry", body)

    def test_pass_purchase_persists_and_cannot_be_repeated(self):
        offers = next(s for s in storefronts() if s["name"] == "BRSeason12")["catalogEntries"]
        self.assertEqual(offers[0]["offerId"], BODY["offerId"])
        before = self.snapshots()
        result = self.purchase()
        after = self.snapshots()
        self.assertEqual(after[0]["items"][self.wallet]["quantity"], 250)
        self.assertTrue(after[1]["stats"]["attributes"]["book_purchased"])
        self.assertEqual(after[1]["stats"]["attributes"]["book_level"], before[1]["stats"]["attributes"]["book_level"])
        templates = {item["templateId"] for item in after[1]["items"].values()}
        self.assertTrue({template for template, _ in PAID[1]} <= templates)
        self.assertEqual(len(result["notifications"][0]["lootResult"]["items"]), 5)
        self.assertEqual(after, self.snapshots(Backend(state_dir=self.directory.name).profiles))
        with self.assertRaises(ApiError):
            self.purchase()
        self.assertEqual(after, self.snapshots())

    def test_insufficient_funds_wrong_price_and_unimplemented_bundle_do_not_mutate(self):
        before = self.snapshots()
        for body in (dict(BODY, expectedTotalPrice=0),
                     dict(BODY, offerId="E72002954292DCD8B8013B9E59A89921", expectedTotalPrice=2800)):
            with self.subTest(body=body), self.assertRaises(ApiError):
                self.purchase(body)
            self.assertEqual(before, self.snapshots())
        tx = ProfileTx(self.service.ctx, before[0])
        tx.set_item_quantity(self.wallet, 949)
        self.service._store("account", {"common_core": tx})
        before = self.snapshots()
        with self.assertRaises(ApiError):
            self.purchase()
        self.assertEqual(before, self.snapshots())

    def test_level_purchase_grants_paid_and_free_rewards_and_currency(self):
        self.purchase()
        core = self.service.profile("account", "common_core")
        tx = ProfileTx(self.service.ctx, core)
        tx.set_item_quantity(self.wallet, 1000)
        self.service._store("account", {"common_core": tx})
        self.purchase(dict(BODY, offerId=SEASON12_LEVEL_OFFER, purchaseQuantity=4, expectedTotalPrice=600))
        core, athena = self.snapshots()
        self.assertEqual(core["items"][self.wallet]["quantity"], 500)
        self.assertEqual(athena["stats"]["attributes"]["book_level"], 5)
        templates = {item["templateId"] for item in athena["items"].values()}
        self.assertIn("AthenaDance:EID_CallMe", templates)
        self.assertIn("AthenaLoadingScreen:LSID_213_SkullDude", templates)

    def test_free_levels_then_pass_catches_up_paid_rewards(self):
        self.purchase(dict(BODY, offerId=SEASON12_LEVEL_OFFER, purchaseQuantity=2, expectedTotalPrice=300))
        core = self.service.profile("account", "common_core")
        tx = ProfileTx(self.service.ctx, core)
        tx.set_item_quantity(self.wallet, 1000)
        self.service._store("account", {"common_core": tx})
        self.purchase()
        templates = {item["templateId"] for item in self.snapshots()[1]["items"].values()}
        self.assertIn("AthenaLoadingScreen:LSID_213_SkullDude", templates)
        self.assertIn("AthenaDance:EID_CallMe", templates)

    def test_repair_is_idempotent_and_preserves_owned_styles(self):
        tx = ProfileTx(self.service.ctx, self.snapshots()[1])
        tx.set_stat("book_purchased", True)
        self.service._store("account", {"athena": tx})
        original = self.service.grant_item("account", "athena", PAID[1][0][0], {"variants": [{"channel": "test"}]})
        before = self.service.profile("account", "athena")
        # Four missing level-one rewards also activate their two immediate
        # challenge bundles and initial quests in the same repair transaction.
        self.assertEqual(self.service.repair_season12_level_one("account"), 8)
        after = self.service.profile("account", "athena")
        self.assertEqual(after["items"][original], before["items"][original])
        templates = {item["templateId"] for item in after["items"].values()}
        self.assertIn("Quest:Quest_S12_Donut_W1_A", templates)
        self.assertIn("Quest:quest_s12_styles1_q01_accolade_ar", templates)
        self.assertEqual(after["rvn"], before["rvn"] + 1)
        self.assertEqual(self.service.repair_season12_level_one("account"), 0)
        self.assertEqual(after, self.service.profile("account", "athena"))
        self.assertEqual(after, Backend(state_dir=self.directory.name).profiles.profile("account", "athena"))

    def test_level_limit_and_invalid_quantity_are_atomic(self):
        before = self.snapshots()
        for q in (0, 100, True):
            with self.subTest(q=q), self.assertRaises(ApiError):
                self.purchase(dict(BODY, offerId=SEASON12_LEVEL_OFFER, purchaseQuantity=q, expectedTotalPrice=150 * q))
            self.assertEqual(before, self.snapshots())

    def test_all_levels_and_unsupported_reward_are_atomic(self):
        self.purchase()
        core = self.service.profile("account", "common_core")
        tx = ProfileTx(self.service.ctx, core)
        tx.set_item_quantity(self.wallet, 20000)
        self.service._store("account", {"common_core": tx})
        before = self.snapshots()
        body = dict(BODY, offerId=SEASON12_LEVEL_OFFER, purchaseQuantity=99, expectedTotalPrice=14850)
        with patch.dict("profile_service.SEASON12_PAID", {2: [("Unsupported:example", 1)]}):
            with self.assertRaises(ApiError):
                self.purchase(body)
        self.assertEqual(before, self.snapshots())
        self.purchase(body)
        core, athena = self.snapshots()
        self.assertEqual(athena["stats"]["attributes"]["book_level"], 100)
        self.assertEqual(athena["stats"]["attributes"]["level"], 100)
        self.assertEqual(core["items"][self.wallet]["quantity"], 6650)
        before = self.snapshots()
        with self.assertRaises(ApiError):
            self.purchase(dict(body, purchaseQuantity=1, expectedTotalPrice=150))
        self.assertEqual(before, self.snapshots())

    def test_database_failure_rolls_back_entitlement_and_debit(self):
        before = self.snapshots()
        with closing(sqlite3.connect(self.service._database)) as db, db:
            db.execute("CREATE TRIGGER fail_athena BEFORE INSERT ON profiles WHEN NEW.profile_id = 'athena' BEGIN SELECT RAISE(ABORT, 'test failure'); END")
        with self.assertRaises(sqlite3.IntegrityError):
            self.purchase()
        self.assertEqual(before, self.snapshots())
        self.assertEqual(before, self.snapshots(Backend(state_dir=self.directory.name).profiles))


if __name__ == "__main__":
    unittest.main()
