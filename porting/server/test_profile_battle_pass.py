"""Local Season 12 pass purchase; offer mapping and price are assumptions."""
import sqlite3
import tempfile
import unittest

from api_common import ApiError
from backend import Backend
from catalog import SEASON12_PASS_OFFER, storefronts
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
        self.assertEqual(after[1]["items"], before[1]["items"])
        self.assertEqual(result["multiUpdate"][0]["profileChanges"], [
            {"changeType": "statModified", "name": "book_purchased", "value": True}])
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

    def test_database_failure_rolls_back_entitlement_and_debit(self):
        before = self.snapshots()
        with sqlite3.connect(self.service._database) as db:
            db.execute("CREATE TRIGGER fail_athena BEFORE INSERT ON profiles WHEN NEW.profile_id = 'athena' BEGIN SELECT RAISE(ABORT, 'test failure'); END")
        with self.assertRaises(sqlite3.IntegrityError):
            self.purchase()
        self.assertEqual(before, self.snapshots())
        self.assertEqual(before, self.snapshots(Backend(state_dir=self.directory.name).profiles))


if __name__ == "__main__":
    unittest.main()
