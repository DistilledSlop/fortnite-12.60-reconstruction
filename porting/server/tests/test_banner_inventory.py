"""Starter banners and persisted common-core repair."""

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
from banner_inventory import STARTER_BANNERS
from profile_service import ProfileTx


class BannerInventoryTests(unittest.TestCase):
    def test_new_profile_has_only_the_starter_set(self):
        service = Backend().profiles
        profile = service.query("new", "common_core")["profileChanges"][0]["profile"]
        banners = [item for item in profile["items"].values()
                   if item["templateId"].casefold().startswith("homebasebanner")]
        self.assertEqual({item["templateId"] for item in banners}, set(STARTER_BANNERS))
        self.assertEqual(len(banners), 75)
        self.assertTrue(all(item["quantity"] == 1 and item["attributes"]["item_seen"] is True
                            for item in banners))

    def test_repair_preserves_existing_items_and_is_durable_and_idempotent(self):
        with tempfile.TemporaryDirectory() as directory:
            service = Backend(state_dir=directory).profiles
            account = "existing"
            athena = service.profile(account, "athena")
            locker = athena["stats"]["attributes"]["last_applied_loadout"]
            service.run(account, "athena", "SetCosmeticLockerBanner", {
                "lockerItem": locker, "bannerIconTemplateName": "CustomBanner",
                "bannerColorTemplateName": "CustomColor",
            })
            selected = service.profile(account, "athena")
            core = service.profile(account, "common_core")
            tx = ProfileTx(service.ctx, core)
            for item_id, item in list(tx.profile["items"].items()):
                if item["templateId"] in STARTER_BANNERS:
                    tx.remove_item(item_id)
            tx.add_item("existing", "homebasebannericon:standardbanner1", {"item_seen": False, "custom": 4})
            tx.add_item("premium", "HomebaseBannerIcon:PremiumBanner", {"item_seen": False})
            tx.set_stat("custom_banner", "keep")
            service._store(account, {"common_core": tx})
            before_revision = tx.profile["rvn"]
            response = service.query(account, "common_core")
            repaired = response["profileChanges"][0]["profile"]
            self.assertEqual(repaired["rvn"], before_revision + 1)
            self.assertEqual(service.profile(account, "athena"), selected)
            self.assertEqual(repaired["items"]["existing"]["attributes"], {"item_seen": False, "custom": 4})
            self.assertEqual(repaired["items"]["premium"], tx.profile["items"]["premium"])
            self.assertEqual(repaired["stats"]["attributes"]["custom_banner"], "keep")
            templates = [item["templateId"].casefold() for item in repaired["items"].values()]
            for template in STARTER_BANNERS:
                self.assertEqual(templates.count(template.casefold()), 1)
            restarted = Backend(state_dir=directory).profiles
            self.assertEqual(restarted.query(account, "common_core")["profileChanges"][0]["profile"], repaired)
            self.assertEqual(restarted.query(account, "common_core")["profileChanges"][0]["profile"], repaired)


    def test_owned_selection_after_query_reports_locker_delta(self):
        with tempfile.TemporaryDirectory() as directory:
            service = Backend(state_dir=directory).profiles
            account = "picker"
            core = service.query(account, "common_core")["profileChanges"][0]["profile"]
            templates = {item["templateId"] for item in core["items"].values()}
            self.assertIn("HomebaseBannerIcon:StandardBanner15", templates)
            self.assertIn("HomebaseBannerColor:DefaultColor20", templates)
            athena = service.profile(account, "athena")
            locker = athena["stats"]["attributes"]["last_applied_loadout"]
            response = service.run(account, "athena", "SetCosmeticLockerBanner", {
                "lockerItem": locker, "bannerIconTemplateName": "StandardBanner15",
                "bannerColorTemplateName": "DefaultColor20",
            }, str(athena["rvn"]))
            self.assertEqual(response["profileChangesBaseRevision"], athena["rvn"])
            self.assertEqual(response["profileRevision"], athena["rvn"] + 1)
            self.assertEqual(response["multiUpdate"], [])
            self.assertEqual(response["profileChanges"], [
                {"changeType": "itemAttrChanged", "itemId": locker,
                 "attributeName": "banner_icon_template", "attributeValue": "StandardBanner15"},
                {"changeType": "itemAttrChanged", "itemId": locker,
                 "attributeName": "banner_color_template", "attributeValue": "DefaultColor20"},
            ])
            restored = Backend(state_dir=directory).profiles.profile(account, "athena")
            self.assertEqual(restored["items"][locker]["attributes"]["banner_icon_template"], "StandardBanner15")
            self.assertEqual(restored["items"][locker]["attributes"]["banner_color_template"], "DefaultColor20")
            self.assertEqual(service.profile(account, "common_core"), core)


if __name__ == "__main__":
    unittest.main()
