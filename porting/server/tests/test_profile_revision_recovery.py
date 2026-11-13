"""Exercise MCP revision recovery through the real HTTP handler."""

# Isolated mode omits the script directory from sys.path.
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import unittest
import sys
from pathlib import Path

if __package__:
    from . import test_services as support
else:
    import test_services as support


class ProfileRevisionRecoveryTests(support.ServiceTestCase):
    def test_stale_command_receives_resulting_snapshot_and_current_gets_delta(self):
        token = self.sign_in()["access_token"]
        path = f"/fortnite/api/game/v2/profile/{support.LOCAL}/client/"

        def command(name, rvn, body):
            status, result, _ = support.jcall(
                self.base, "POST", path + name + f"?profileId=athena&rvn={rvn}",
                token=token, body=body)
            self.assertEqual(status, 200, result)
            return result

        initial = command("QueryProfile", -1, {})["profileChanges"][0]["profile"]
        locker = initial["stats"]["attributes"]["last_applied_loadout"]
        current = command("SetCosmeticLockerName", initial["rvn"],
                          {"lockerItem": locker, "name": "First"})
        self.assertEqual(current["profileChanges"][0]["changeType"], "itemAttrChanged")
        self.assertEqual(current["profileChangesBaseRevision"], initial["rvn"])

        stale = command("SetCosmeticLockerName", initial["rvn"],
                        {"lockerItem": locker, "name": "Second"})
        self.assertEqual(len(stale["profileChanges"]), 1)
        change = stale["profileChanges"][0]
        self.assertEqual(change["changeType"], "fullProfileUpdate")
        self.assertEqual(change["profile"]["items"][locker]["attributes"]["locker_name"], "Second")
        self.assertEqual(change["profile"]["rvn"], stale["profileRevision"])
        self.assertEqual(stale["profileChangesBaseRevision"], stale["profileRevision"])

        # A stale no-op still needs a snapshot for the client to catch up.
        noop = command("SetCosmeticLockerName", -1, {"lockerItem": locker, "name": "Second"})
        self.assertEqual(noop["profileChanges"][0]["changeType"], "fullProfileUpdate")
        self.assertEqual(noop["profileRevision"], stale["profileRevision"])
        self.assertEqual(noop["profileCommandRevision"], stale["profileCommandRevision"] + 1)

    def test_wallet_purchase_uses_session_command_revision_and_safe_recovery(self):
        token = self.sign_in()["access_token"]
        path = f"/fortnite/api/game/v2/profile/{support.LOCAL}/client/"
        purchase = {"offerId": "local-free-credits", "purchaseQuantity": 1,
                    "currency": "MtxCurrency", "currencySubType": "", "expectedTotalPrice": 0}

        def command(name, revision, body):
            status, result, _ = support.jcall(
                self.base, "POST", path + name + f"?profileId=common_core&rvn={revision}",
                token=token, body=body)
            self.assertEqual(status, 200, result)
            return result

        initial = command("QueryProfile", -1, {})
        # A successful no-op advances only the command revision.
        noop = command("SetMtxPlatform", initial["profileCommandRevision"], {"newPlatform": "EpicPC"})
        self.assertEqual(noop["profileRevision"], initial["profileRevision"])
        self.assertNotEqual(noop["profileRevision"], noop["profileCommandRevision"])
        bought = command("PurchaseCatalogEntry", noop["profileCommandRevision"], purchase)
        self.assertEqual(bought["profileChangesBaseRevision"], noop["profileRevision"])
        self.assertEqual(bought["profileChanges"][0]["changeType"], "itemQuantityChanged")
        self.assertEqual(bought["profileChanges"][0]["quantity"], 1000)

        # A different session cannot borrow this session's known baseline.
        other_token = self.sign_in()["access_token"]
        status, other, _ = support.jcall(
            self.base, "POST", path + "SetMtxPlatform" +
            f"?profileId=common_core&rvn={bought['profileCommandRevision']}",
            token=other_token, body={"newPlatform": "EpicPC"})
        self.assertEqual(status, 200, other)
        self.assertEqual(other["profileChanges"][0]["changeType"], "fullProfileUpdate")
        # The separate no-op command also makes the first session's revision stale.
        caught_up = command("QueryProfile", -1, {})

        # An independently committed profile change does not advance its command
        # revision. The old session must receive a snapshot rather than a delta.
        self.backend.profiles.grant_item(support.LOCAL, "common_core", "Test:Independent")
        recovered = command("PurchaseCatalogEntry", caught_up["profileCommandRevision"], purchase)
        self.assertEqual(recovered["profileChanges"][0]["changeType"], "fullProfileUpdate")
        self.assertEqual(recovered["profileChangesBaseRevision"], recovered["profileRevision"])
        again = command("PurchaseCatalogEntry", recovered["profileCommandRevision"], purchase)
        self.assertEqual(again["profileChanges"][0]["changeType"], "itemQuantityChanged")
        self.assertEqual(again["profileChanges"][0]["quantity"], 3000)

    def test_same_command_revision_with_distinct_issued_snapshots_is_ambiguous(self):
        token = self.sign_in()["access_token"]
        path = f"/fortnite/api/game/v2/profile/{support.LOCAL}/client/"

        def command(name, revision, body):
            status, result, _ = support.jcall(
                self.base, "POST", path + name + f"?profileId=common_core&rvn={revision}",
                token=token, body=body)
            self.assertEqual(status, 200, result)
            return result

        first = command("QueryProfile", -1, {})
        self.backend.profiles.grant_item(support.LOCAL, "common_core", "Test:Independent")
        second = command("QueryProfile", -1, {})
        self.assertEqual(first["profileCommandRevision"], second["profileCommandRevision"])
        self.assertNotEqual(first["profileRevision"], second["profileRevision"])
        result = command("SetMtxPlatform", second["profileCommandRevision"], {"newPlatform": "PSN"})
        self.assertEqual(result["profileChanges"][0]["changeType"], "fullProfileUpdate")


if __name__ == "__main__":
    unittest.main()
