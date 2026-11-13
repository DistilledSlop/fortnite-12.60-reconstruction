"""Durable trusted-result replay, atomic rewards and recovered XP thresholds."""

# Isolated mode omits the script directory from sys.path.
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import sqlite3
import tempfile
import unittest
from contextlib import closing

from account_service import AccountService
from api_common import ApiError, Context, LOCAL_ACCOUNT_ID
from profile_service import ProfileService, ProfileTx
from progression_service import ProgressionService
from season12_progression_data import XP_TO_NEXT


class ProgressionTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.ctx = Context(clock=lambda: 1700000000)
        self.accounts = AccountService(self.ctx, state_dir=self.directory.name)
        self.profiles, self.service = self.restart()
        self.payload = {"accountId": LOCAL_ACCOUNT_ID, "kills": 3, "placement": 1,
                        "xp": 30000, "endedAt": "2023-11-14T22:13:20Z"}
        self.snapshots()

    def restart(self):
        profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.directory.name)
        return profiles, ProgressionService(self.ctx, self.accounts, profiles,
                                           state_dir=self.directory.name)

    def snapshots(self):
        return [self.profiles.profile(LOCAL_ACCOUNT_ID, pid) for pid in ("athena", "common_core")]

    def test_exactly_once_survives_restart_and_response_mutation(self):
        before = self.snapshots()
        receipt = self.service.record_result("match-1", self.payload)
        after = self.snapshots()
        attrs = after[0]["stats"]["attributes"]
        self.assertEqual((attrs["level"], attrs["xp"], attrs["book_level"], attrs["book_xp"]), (3, 0, 3, 0))
        self.assertEqual(attrs["accountLevel"], 3)
        self.assertEqual(after[0]["rvn"], before[0]["rvn"] + 1)
        self.assertEqual(after[0]["commandRevision"], before[0]["commandRevision"])
        self.assertEqual(receipt, self.service.record_result("match-1", self.payload))
        self.assertEqual(after, self.snapshots())
        self.profiles, self.service = self.restart()
        self.assertTrue(self.service.has_result("match-1", LOCAL_ACCOUNT_ID))
        self.assertEqual(receipt, self.service.replay_result("match-1", self.payload))
        receipt["profileUpdate"]["profileChanges"].clear()
        self.assertTrue(self.service.record_result("match-1", self.payload)["profileUpdate"]["profileChanges"])
        self.assertEqual(after, self.snapshots())
        counters = self.service.stats(LOCAL_ACCOUNT_ID)["stats"]
        self.assertEqual(counters["br_kills_keyboardmouse_m0_playlist_defaultsolo"], 3)
        self.assertEqual(counters["br_matchesplayed_keyboardmouse_m0_playlist_defaultsolo"], 1)

    def test_conflict_invalid_and_unknown_account_do_not_mutate(self):
        self.service.record_result("match-1", self.payload)
        before = self.snapshots()
        with self.assertRaises(ApiError) as caught:
            self.service.record_result("match-1", dict(self.payload, kills=4))
        self.assertEqual(caught.exception.status, 409)
        invalid = [{"placement": 0}, {"placement": True}, {"kills": -1}, {"kills": 100},
                   {"xp": -1}, {"xp": True}, {"bookXp": "100"}, {"extra": 1},
                   {"endedAt": "2023-02-30T00:00:00Z"}, {"playlist": "bad/route"},
                   {"accountId": "missing"}, {"accolades": {"MadeUp": 1}},
                   {"accolades": {"Elimination": 3}, "xp": 1}]
        for change in invalid:
            with self.subTest(change=change), self.assertRaises(ApiError):
                self.service.record_result("invalid", dict(self.payload, **change))
            self.assertFalse(self.service.has_result("invalid", LOCAL_ACCOUNT_ID))
            self.assertEqual(before, self.snapshots())

    def test_rewards_include_currency_atomically_and_paid_schedule(self):
        athena, core = self.snapshots()
        tx = ProfileTx(self.ctx, athena)
        tx.set_stat("book_purchased", True)
        self.profiles._store(LOCAL_ACCOUNT_ID, {"athena": tx})
        before = self.snapshots()
        receipt = self.service.record_result("rewards", dict(self.payload, xp=110000))
        after = self.snapshots()
        templates = {item["templateId"] for item in after[0]["items"].values()}
        self.assertIn("AthenaDance:EID_CallMe", templates)
        self.assertIn("AthenaLoadingScreen:LSID_213_SkullDude", templates)
        wallet = next(i for i in core["items"] if core["items"][i]["templateId"] == "Currency:MtxGiveaway")
        self.assertEqual(after[1]["items"][wallet]["quantity"], core["items"][wallet]["quantity"] + 100)
        self.assertEqual(after[1]["rvn"], before[1]["rvn"] + 1)
        self.assertEqual(after[1]["commandRevision"], before[1]["commandRevision"])
        self.assertEqual(receipt["profileUpdate"]["multiUpdate"][0]["profileId"], "common_core")
        self.profiles, self.service = self.restart()
        self.assertEqual(after, self.snapshots())
        self.assertEqual(receipt, self.service.record_result("rewards", dict(self.payload, xp=110000)))

    def test_match_crossing_banner_level_converts_and_replays(self):
        athena, _ = self.snapshots()
        tx = ProfileTx(self.ctx, athena)
        tx.set_stat('book_level', 32)
        tx.set_stat('book_xp', 0)
        self.profiles._store(LOCAL_ACCOUNT_ID, {'athena': tx})
        payload = dict(self.payload, xp=0, bookXp=XP_TO_NEXT[31])
        receipt = self.service.record_result('banner', payload)
        athena, core = self.snapshots()
        self.assertFalse(any(v['templateId'].casefold() == 'bannertoken:bannertoken_029_s12_cowboy'
                             for v in athena['items'].values()))
        icon = 'homebasebannericon:brs12_cowboy'
        self.assertEqual(sum(v['templateId'].casefold() == icon for v in core['items'].values()), 1)
        update = next(u for u in receipt['profileUpdate']['multiUpdate'] if u['profileId'] == 'common_core')
        self.assertTrue(any(c['changeType'] == 'itemAdded' and c['item']['templateId'].casefold() == icon
                            for c in update['profileChanges']))
        self.profiles, self.service = self.restart()
        self.assertEqual(receipt, self.service.record_result('banner', payload))
        self.assertEqual([athena, core], self.snapshots())

    def test_receipt_failure_rolls_back_all_profiles_and_cache(self):
        before = self.snapshots()
        with closing(sqlite3.connect(self.profiles._database)) as db, db:
            db.execute("CREATE TRIGGER reject_result BEFORE INSERT ON match_results "
                       "BEGIN SELECT RAISE(ABORT,'test result failure'); END")
        with self.assertRaises(sqlite3.IntegrityError):
            self.service.record_result("failed", dict(self.payload, xp=110000))
        self.assertEqual(before, self.snapshots())
        self.assertFalse(self.service.has_result("failed", LOCAL_ACCOUNT_ID))
        self.profiles, self.service = self.restart()
        self.assertEqual(before, self.snapshots())

    def test_latest_database_profiles_used_and_other_player_same_session(self):
        stale_profiles, stale_service = self.restart()
        self.service.record_result("one", self.payload)
        stale_service.record_result("two", self.payload)
        self.profiles, self.service = self.restart()
        self.assertEqual(self.snapshots()[0]["stats"]["attributes"]["matchesplayed"], 2)
        other = self.accounts.get_or_create_username("Other")
        self.service.record_result("one", dict(self.payload, accountId=other["id"]))
        self.assertEqual(self.service.stats(other["id"])["stats"]["br_matchesplayed_keyboardmouse_m0_playlist_defaultsolo"], 1)

    def test_accolades_original_amounts_and_stats_time_bounds(self):
        payload = dict(self.payload)
        del payload["xp"]
        payload["accolades"] = {"Elimination": 3}
        receipt = self.service.record_result("accolades", payload)
        self.assertEqual(receipt["xpAwarded"], 450)
        self.assertEqual(self.service.stats(LOCAL_ACCOUNT_ID, 1700000001)["stats"], {})
        self.assertTrue(self.service.stats(LOCAL_ACCOUNT_ID, 1700000000, 1700000000)["stats"])
        with self.assertRaises(ApiError):
            self.service.stats(LOCAL_ACCOUNT_ID, 10, 1)

    def test_extended_levels_book_purchases_and_free_only_grants(self):
        athena = self.snapshots()[0]
        tx = ProfileTx(self.ctx, athena)
        tx.set_stat("level", 100)
        tx.set_stat("accountLevel", 200)
        tx.set_stat("book_level", 4)
        self.profiles._store(LOCAL_ACCOUNT_ID, {"athena": tx})
        self.assertEqual(XP_TO_NEXT[99], 50000)
        self.service.record_result("extended", dict(self.payload, xp=50000, bookXp=40000))
        athena = self.snapshots()[0]
        attrs = athena["stats"]["attributes"]
        self.assertEqual((attrs["level"], attrs["accountLevel"], attrs["book_level"]), (101, 201, 5))
        templates = {item["templateId"] for item in athena["items"].values()}
        self.assertNotIn("AthenaLoadingScreen:LSID_213_SkullDude", templates)


if __name__ == "__main__":
    unittest.main()
