"""Username selection and authenticated locker loading over the HTTP API."""
import tempfile

from api_common import Err
from backend import Backend
from test_services import GENERATED, ServiceTestCase, jcall, mcp


class UsernameLockerFlowTests(ServiceTestCase):
    def test_named_accounts_load_distinct_persistent_lockers(self):
        state = tempfile.TemporaryDirectory(dir=GENERATED)
        self.addCleanup(state.cleanup)
        self.start_backend(Backend(clock=self.clock, state_step_sec=0, state_dir=state.name))

        def login(name):
            status, auth, _ = jcall(self.base, "POST", "/account/api/oauth/token",
                                    form={"grant_type": "local_username", "username": name})
            self.assertEqual(status, 200, auth)
            return self.remember(auth)

        def query(auth):
            account = auth["account_id"]
            status, envelope, _ = mcp(self.base, auth["access_token"], account, "QueryProfile")
            self.assertEqual(status, 200, envelope)
            self.assertEqual(envelope["profileId"], "athena")
            self.assertEqual(len(envelope["profileChanges"]), 1)
            change = envelope["profileChanges"][0]
            self.assertEqual(change["changeType"], "fullProfileUpdate")
            profile = change["profile"]
            self.assertEqual(profile["accountId"], account)
            self.assertEqual(profile["profileId"], "athena")
            self.assertEqual(envelope["profileRevision"], profile["rvn"])
            self.assertEqual(envelope["profileCommandRevision"], profile["commandRevision"])
            self.assertEqual(envelope["profileChangesBaseRevision"], profile["rvn"])
            locker = profile["stats"]["attributes"]["last_applied_loadout"]
            self.assertEqual(profile["items"][locker]["templateId"], "CosmeticLocker:cosmeticlocker_athena")
            return profile, locker

        alice = login("  Alice Flow  ")
        bob = login("  Bob Flow  ")
        self.assertEqual(alice["displayName"], "Alice Flow")
        self.assertEqual(bob["displayName"], "Bob Flow")
        self.assertNotEqual(alice["account_id"], bob["account_id"])
        alice_profile, alice_locker = query(alice)
        bob_profile, bob_locker = query(bob)
        status, result, _ = mcp(self.base, alice["access_token"], alice["account_id"],
                                 "SetCosmeticLockerName", body={"lockerItem": alice_locker, "name": "Alice's locker"})
        self.assertEqual(status, 200, result)
        alice_profile, _ = query(alice)
        bob_profile, _ = query(bob)
        self.assertEqual(alice_profile["items"][alice_locker]["attributes"]["locker_name"], "Alice's locker")
        self.assertNotEqual(bob_profile["items"][bob_locker]["attributes"]["locker_name"], "Alice's locker")
        self.assertError(mcp(self.base, bob["access_token"], alice["account_id"], "QueryProfile"),
                         403, Err.MISSING_PERMISSION)
        self.assertError(mcp(self.base, "invalid-token", alice["account_id"], "QueryProfile"),
                         401, Err.TOKEN_INVALID)
        self.clock.now += alice["expires_in"]
        self.assertError(mcp(self.base, alice["access_token"], alice["account_id"], "QueryProfile"),
                         401, Err.TOKEN_INVALID)

        self.start_backend(Backend(clock=self.clock, state_step_sec=0, state_dir=state.name))
        alice_again = login(" alice flow ")
        bob_again = login(" BOB FLOW ")
        self.assertEqual(alice_again["account_id"], alice["account_id"])
        self.assertEqual(bob_again["account_id"], bob["account_id"])
        self.assertEqual(query(alice_again)[0], alice_profile)
        self.assertEqual(query(bob_again)[0], bob_profile)
