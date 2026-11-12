"""Party pings should not point to parties that have been deleted."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import test_services as support


class PartyPingLifecycleTests(support.ServiceTestCase):
    def test_disband_removes_only_pings_for_deleted_party(self):
        captain = self.sign_in()["access_token"]
        guest_id = self.new_account("Guest")
        guest = self.sign_in_account(guest_id)["access_token"]
        other_id = self.new_account("Other")
        other = self.sign_in_account(other_id)["access_token"]
        first = support.create_party(self.base, captain)["id"]
        second = support.create_party(self.base, other)["id"]
        for sender_id, token in ((support.LOCAL, captain), (other_id, other)):
            path = f"{support.PARTY}/user/{guest_id}/pings/{sender_id}/parties"
            self.assertEqual(support.call(self.base, "POST", path, token=token, body={})[0], 204)

        state = f"{support.PARTY}/user/{guest_id}"
        pings = support.jcall(self.base, "GET", state, token=guest)[1]["pings"]
        self.assertEqual({ping["party_id"] for ping in pings}, {first, second})
        leave = f"{support.PARTY}/parties/{first}/members/{support.LOCAL}"
        self.assertEqual(support.call(self.base, "DELETE", leave, token=captain)[0], 204)
        pings = support.jcall(self.base, "GET", state, token=guest)[1]["pings"]
        self.assertEqual([ping["party_id"] for ping in pings], [second])


if __name__ == "__main__":
    unittest.main()
