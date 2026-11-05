"""Party invite and ping lifetimes against the local HTTP service."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import test_services as support


class PartyExpiryTests(support.ServiceTestCase):
    def setUp(self):
        super().setUp()
        self.captain = self.sign_in()["access_token"]
        self.guest_id = self.new_account("Guest")
        self.guest = self.sign_in_account(self.guest_id)["access_token"]

    def test_expired_invite_cannot_join_closed_party_and_can_be_renewed(self):
        party = support.create_party(self.base, self.captain,
                                     config={"joinability": "CLOSED", "invite_ttl_seconds": 10})
        pid = party["id"]
        invite = f"{support.PARTY}/parties/{pid}/invites/{self.guest_id}"
        join = f"{support.PARTY}/parties/{pid}/members/{self.guest_id}/join"
        state = f"{support.PARTY}/user/{self.guest_id}"
        self.assertEqual(support.call(self.base, "POST", invite, token=self.captain,
                                      body={"sendPing": True})[0], 204)
        self.clock.now += 9
        self.assertEqual(len(support.jcall(self.base, "GET", state, token=self.guest)[1]["invites"]), 1)
        self.clock.now += 1
        self.assertError(support.jcall(self.base, "POST", join, token=self.guest, body={}),
                         403, support.Err.PARTY_JOIN_FORBIDDEN)
        self.assertEqual(support.jcall(self.base, "GET", state, token=self.guest)[1]["invites"], [])
        self.assertEqual(support.call(self.base, "POST", invite, token=self.captain, body={})[0], 204)
        self.assertEqual(support.jcall(self.base, "POST", join, token=self.guest, body={})[0], 200)
        self.clock.now += 50
        self.assertEqual(support.jcall(self.base, "GET", state, token=self.guest)[1]["pings"], [])

    def test_invalid_configured_invite_ttl_is_rejected(self):
        for ttl in (0, -1, True, "10"):
            with self.subTest(ttl=ttl):
                self.assertError(support.jcall(self.base, "POST", f"{support.PARTY}/parties",
                                               token=self.captain,
                                               body={"config": {"invite_ttl_seconds": ttl}}),
                                 400, support.Err.INVALID_PAYLOAD)


if __name__ == "__main__":
    unittest.main()
