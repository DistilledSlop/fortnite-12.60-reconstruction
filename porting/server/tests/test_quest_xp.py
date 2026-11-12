"""XP awards share the caller's profile transaction, including tier rewards."""
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
from profile_service import CommandContext, ProfileService, ProfileTx
from progression_service import award_profile_xp


class QuestXpTests(unittest.TestCase):
    def setUp(self):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.ctx = Context(clock=lambda: 1700000000)
        self.accounts = AccountService(self.ctx, state_dir=directory.name)
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=directory.name)
        self.directory = directory.name
        self.snapshots()

    def snapshots(self):
        return [self.profiles.profile(LOCAL_ACCOUNT_ID, pid) for pid in ("athena", "common_core")]

    def command(self):
        return CommandContext(self.profiles, LOCAL_ACCOUNT_ID, "athena", {})

    def set_stats(self, **stats):
        tx = ProfileTx(self.ctx, self.snapshots()[0])
        for name, value in stats.items():
            tx.set_stat(name, value)
        self.profiles._store(LOCAL_ACCOUNT_ID, {"athena": tx})

    def test_boundary_and_purchased_tier_free_rewards(self):
        self.set_stats(book_level=4, accountLevel=200)
        before = self.snapshots()
        command = self.command()
        award_profile_xp(command, 10000, 40000)
        attrs = command.tx("athena").profile["stats"]["attributes"]
        self.assertEqual((attrs["level"], attrs["xp"], attrs["accountLevel"]), (2, 0, 201))
        self.assertEqual((attrs["book_level"], attrs["book_xp"]), (5, 0))
        templates = {item["templateId"] for item in command.tx("athena").profile["items"].values()}
        self.assertNotIn("AthenaLoadingScreen:LSID_213_SkullDude", templates)
        self.assertEqual(before, self.snapshots())
        self.assertEqual(attrs.get("matchesplayed", 0), before[0]["stats"]["attributes"].get("matchesplayed", 0))

    def test_paid_and_free_rewards_share_transaction_and_rollback(self):
        self.set_stats(book_purchased=True)
        before = self.snapshots()
        command = self.command()
        award_profile_xp(command, 110000, 110000)
        templates = {item["templateId"] for item in command.tx("athena").profile["items"].values()}
        self.assertIn("AthenaDance:EID_CallMe", templates)
        self.assertIn("AthenaLoadingScreen:LSID_213_SkullDude", templates)
        self.assertTrue(command.tx("common_core").changed)
        with self.assertRaises(sqlite3.IntegrityError):
            with closing(sqlite3.connect(self.profiles._database)) as db, db:
                db.execute("BEGIN IMMEDIATE")
                self.profiles._store(LOCAL_ACCOUNT_ID, command.txs, database=db, publish=False)
                db.execute("CREATE TABLE failed (id INTEGER PRIMARY KEY)")
                db.execute("INSERT INTO failed VALUES (1)")
                db.execute("INSERT INTO failed VALUES (1)")
        self.assertEqual(before, self.snapshots())
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.directory)
        self.assertEqual(before, self.snapshots())

    def test_caps_and_invalid_awards(self):
        self.set_stats(level=1000, book_level=100, xp=9, book_xp=7)
        command = self.command()
        award_profile_xp(command, 4, 5)
        attrs = command.tx("athena").profile["stats"]["attributes"]
        self.assertEqual((attrs["level"], attrs["xp"], attrs["book_level"], attrs["book_xp"]), (1000, 13, 100, 12))
        for xp in (-1, True, "1", 2**31):
            with self.subTest(xp=xp), self.assertRaises(ApiError):
                award_profile_xp(self.command(), xp, 0)


if __name__ == "__main__":
    unittest.main()
