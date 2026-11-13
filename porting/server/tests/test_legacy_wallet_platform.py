"""Legacy wallet migration preserves balances and survives a restart."""

if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import json
import sqlite3
import tempfile
import unittest
from contextlib import closing

from backend import Backend


class LegacyWalletPlatformTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        service = Backend(state_dir=self.directory.name).profiles
        # Query once to establish any other profile repairs before making a legacy fixture.
        core = service.query("account", "common_core")["profileChanges"][0]["profile"]
        self.wallet = next(k for k, v in core["items"].items()
                           if v["templateId"] == "Currency:MtxGiveaway")
        self.database = service._database
        self.core = core

    def seed(self, platform):
        core = self.core
        wallet = core["items"][self.wallet]
        wallet["quantity"] = 14000
        wallet["attributes"]["legacy_marker"] = {"preserved": True}
        if platform == "missing":
            wallet["attributes"].pop("platform", None)
        else:
            wallet["attributes"]["platform"] = platform
        core["rvn"] = 16
        core["commandRevision"] = 15
        core["updated"] = "2000-01-01T00:00:00.000Z"
        with closing(sqlite3.connect(self.database)) as db, db:
            db.execute("UPDATE profiles SET data = ? WHERE account_id = ? AND profile_id = ?",
                       (json.dumps(core), "account", "common_core"))

    def test_missing_or_null_platform_is_repaired_once_and_purchase_persists(self):
        for platform in ("missing", None):
            with self.subTest(platform=platform):
                self.seed(platform)
                service = Backend(state_dir=self.directory.name).profiles
                response = service.query("account", "common_core")
                core = response["profileChanges"][0]["profile"]
                self.assertEqual(response["profileRevision"], 17)
                self.assertEqual(response["profileCommandRevision"], 15)
                self.assertEqual(core["rvn"], 17)
                self.assertNotEqual(core["updated"], "2000-01-01T00:00:00.000Z")
                self.assertEqual(core["items"][self.wallet], {
                    "templateId": "Currency:MtxGiveaway", "quantity": 14000,
                    "attributes": {"platform": "EpicPC", "legacy_marker": {"preserved": True}}})
                purchase = {"offerId": "local-free-credits", "purchaseQuantity": 1,
                            "currency": "MtxCurrency", "currencySubType": "", "expectedTotalPrice": 0}
                result = service.run("account", "common_core", "PurchaseCatalogEntry", purchase, "-1")
                bought = result["profileChanges"][0]["profile"]
                self.assertEqual((result["profileRevision"], result["profileCommandRevision"]), (18, 16))
                self.assertEqual(bought["items"][self.wallet]["quantity"], 15000)
                self.assertEqual(bought["items"][self.wallet]["attributes"]["platform"], "EpicPC")
                restored = Backend(state_dir=self.directory.name).profiles
                self.assertEqual(restored.query("account", "common_core")["profileChanges"][0]["profile"], bought)
                self.assertEqual(restored.query("account", "common_core")["profileRevision"], 18)

    def test_explicit_platform_is_preserved(self):
        self.seed("PSN")
        service = Backend(state_dir=self.directory.name).profiles
        core = service.query("account", "common_core")["profileChanges"][0]["profile"]
        self.assertEqual(core["items"][self.wallet]["attributes"]["platform"], "PSN")
        self.assertEqual((core["rvn"], core["commandRevision"]), (16, 15))
        self.assertEqual(core["updated"], "2000-01-01T00:00:00.000Z")


if __name__ == "__main__":
    unittest.main()
