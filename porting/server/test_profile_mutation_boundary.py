"""Profile snapshots and committed transactions cannot alter stored state."""
import tempfile
import unittest

from backend import Backend
from profile_service import ProfileTx


class ProfileMutationBoundaryTests(unittest.TestCase):
    def test_snapshots_and_committed_transactions_do_not_alias_stored_locker(self):
        with tempfile.TemporaryDirectory() as directory:
            service = Backend(state_dir=directory).profiles
            snapshot = service.profile("account", "athena")
            locker = snapshot["stats"]["attributes"]["last_applied_loadout"]
            tx = ProfileTx(service.ctx, snapshot)
            tx.set_item_attr(locker, "locker_name", "Saved")
            service._store("account", {"athena": tx})

            snapshot["items"][locker]["attributes"]["locker_name"] = "Snapshot edit"
            tx.profile["items"][locker]["attributes"]["locker_name"] = "Transaction edit"
            returned = service.profile("account", "athena")
            returned["items"][locker]["attributes"]["locker_name"] = "Returned edit"

            current = service.query("account", "athena")["profileChanges"][0]["profile"]
            restored = Backend(state_dir=directory).profiles.query("account", "athena")
            self.assertEqual(current, restored["profileChanges"][0]["profile"])
            self.assertEqual(current["items"][locker]["attributes"]["locker_name"], "Saved")


if __name__ == "__main__":
    unittest.main()
