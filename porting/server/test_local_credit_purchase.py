"""The explicitly free local credit pack changes only the local wallet."""
import tempfile
import unittest
from unittest.mock import patch

from api_common import ApiError
from backend import Backend
from catalog import LOCAL_CREDIT_OFFER, get_offer

BODY = {"offerId": LOCAL_CREDIT_OFFER, "purchaseQuantity": 1, "currency": "MtxCurrency",
        "currencySubType": "", "expectedTotalPrice": 0}


class LocalCreditPurchaseTests(unittest.TestCase):
    def test_repeatable_local_credits_persist(self):
        with tempfile.TemporaryDirectory() as directory:
            service = Backend(state_dir=directory).profiles
            for expected in (1000, 2000):
                service.run("account", "common_core", "PurchaseCatalogEntry", BODY)
                core = service.profile("account", "common_core")
                self.assertEqual(next(iter(core["items"].values()))["quantity"], expected)
            self.assertEqual(core, Backend(state_dir=directory).profiles.profile("account", "common_core"))

    def test_bad_price_quantity_or_grant_does_not_credit(self):
        service = Backend().profiles
        before = service.profile("account", "common_core")
        for update in ({"expectedTotalPrice": 1}, {"purchaseQuantity": 2}, {"currency": "RealMoney"}):
            with self.subTest(update=update), self.assertRaises(ApiError):
                service.run("account", "common_core", "PurchaseCatalogEntry", {**BODY, **update})
            self.assertEqual(before, service.profile("account", "common_core"))
        offer = get_offer(LOCAL_CREDIT_OFFER)
        offer["itemGrants"][0]["quantity"] = 9999
        with patch("profile_service.get_offer", return_value=offer), self.assertRaises(ApiError):
            service.run("account", "common_core", "PurchaseCatalogEntry", BODY)
        self.assertEqual(before, service.profile("account", "common_core"))


if __name__ == "__main__":
    unittest.main()
