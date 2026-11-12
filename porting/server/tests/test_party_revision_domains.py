"""Exercise party and member revision domains through the HTTP service."""
import unittest

if __package__:
    from .test_services import ServiceTestCase, PARTY, LOCAL, call, create_party, jcall
else:
    import sys
    from pathlib import Path
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    from test_services import ServiceTestCase, PARTY, LOCAL, call, create_party, jcall

from api_common import Err


class PartyRevisionDomainsTests(ServiceTestCase):
    def test_member_change_does_not_silently_stale_party_revision(self):
        token = self.sign_in()["access_token"]
        party = create_party(self.base, token, meta={"party_keep": "original"})
        path = f"{PARTY}/parties/{party['id']}"
        member_path = f"{path}/members/{LOCAL}/meta"
        party_revision = party["revision"]
        member_revision = party["members"][0]["revision"]
        events = []
        self.backend.party.notify = lambda recipient, event: events.append((recipient, event))
        self.clock.now += 10

        self.assertEqual(call(self.base, "PATCH", member_path, token=token,
                              body={"revision": member_revision,
                                    "update": {"member_key": "member_value"}})[0], 204)
        self.assertError(jcall(self.base, "PATCH", member_path, token=token,
                               body={"revision": member_revision, "update": {"bad": "value"}}),
                         409, Err.STALE_REVISION)
        after_member = jcall(self.base, "GET", path, token=token)[1]
        self.assertEqual(after_member["revision"], party_revision)
        self.assertEqual(after_member["members"][0]["revision"], member_revision + 1)
        self.assertNotEqual(after_member["members"][0]["updated_at"], party["members"][0]["updated_at"])

        payload = {"revision": party_revision, "meta": {"update": {"party_key": "party_value"}}}
        self.assertEqual(call(self.base, "PATCH", path, token=token, body=payload)[0], 204)
        self.assertError(jcall(self.base, "PATCH", path, token=token, body=payload),
                         409, Err.STALE_REVISION)
        state = jcall(self.base, "GET", path, token=token)[1]
        self.assertEqual(state["revision"], party_revision + 1)
        self.assertEqual(state["meta"], {"party_keep": "original", "party_key": "party_value"})
        self.assertEqual(state["members"][0]["meta"], {"member_key": "member_value"})
        self.assertEqual(len(events), 2)
        member_event, party_event = (event for _, event in events)
        self.assertTrue(member_event["type"].endswith("MEMBER_STATE_UPDATED"))
        self.assertEqual(member_event["revision"], member_revision + 1)
        self.assertEqual(member_event["member_state_updated"], {"member_key": "member_value"})
        self.assertTrue(party_event["type"].endswith("PARTY_UPDATED"))
        self.assertEqual(party_event["revision"], party_revision + 1)
        self.assertEqual(party_event["party_state_updated"], {"party_key": "party_value"})


if __name__ == "__main__":
    unittest.main()
