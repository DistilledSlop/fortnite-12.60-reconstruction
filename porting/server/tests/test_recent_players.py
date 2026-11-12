"""Recent co-play is derived from committed match results only."""
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import sqlite3
from contextlib import closing
import tempfile
import unittest
from types import SimpleNamespace
from unittest.mock import patch

from account_service import AccountService
from api_common import ApiError, Context, LOCAL_ACCOUNT_ID, Router
from friends_service import FriendsService
from profile_service import ProfileService
from progression_service import ProgressionService


class RecentPlayersTests(unittest.TestCase):
    def setup_services(self, state_dir):
        ctx = Context(clock=lambda: 1700000000)
        accounts = AccountService(ctx, state_dir=state_dir)
        profiles = ProfileService(ctx, accounts, None, state_dir=state_dir)
        progression = ProgressionService(ctx, accounts, profiles, state_dir=state_dir)
        oauth = SimpleNamespace(account_session=lambda req: SimpleNamespace(account_id=req))
        friends = FriendsService(ctx, accounts, oauth, state_dir=state_dir)
        self.addCleanup(friends.close)
        friends.progression = progression
        router = Router()
        friends.register(router)
        return accounts, progression, friends, router

    @staticmethod
    def record(service, session, account, date='2023-11-14T22:13:20Z'):
        return service.record_result(session, {'accountId': account, 'placement': 4,
                                               'kills': 0, 'xp': 0, 'endedAt': date})

    @staticmethod
    def call(router, caller, account, namespace='fortnite'):
        route, match, _ = router.resolve('GET', f'/friends/api/v1/{account}/recent/{namespace}')
        return route.handler(caller, match)[1]

    def exercise(self, state_dir):
        accounts, progression, friends, router = self.setup_services(state_dir)
        a = LOCAL_ACCOUNT_ID
        b, c, outsider = [accounts.create(name)['id'] for name in ('Bee', 'Cee', 'Outsider')]
        self.record(progression, 'one', a)
        self.record(progression, 'other', outsider)
        self.assertEqual(self.call(router, a, a), [])
        self.record(progression, 'one', b)
        self.assertEqual(self.call(router, a, a), [{'accountId': b, 'lastPlayed': '2023-11-14T22:13:20Z'}])
        self.assertEqual(self.call(router, b, b)[0]['accountId'], a)
        self.record(progression, 'two', a, '2023-11-15T00:00:00Z')
        self.record(progression, 'two', c, '2023-11-15T00:00:01Z')
        self.record(progression, 'three', a, '2023-11-15T00:00:00Z')
        self.record(progression, 'three', b, '2023-11-15T00:00:01Z')
        expected = [{'accountId': peer, 'lastPlayed': '2023-11-15T00:00:01Z'} for peer in sorted([b, c])]
        self.assertEqual(self.call(router, a, a, 'FoRtNiTe'), expected)
        self.assertEqual(self.call(router, a, a, 'other'), [])
        with self.assertRaises(ApiError) as error:
            self.call(router, b, a)
        self.assertEqual(error.exception.status, 403)
        self.record(progression, 'three', b, '2023-11-15T00:00:01Z')
        with self.assertRaises(ApiError):
            progression.record_result('three', {'accountId': b, 'placement': 1, 'kills': 0, 'xp': 0})
        self.assertEqual(self.call(router, a, a), expected)
        # Either directed block filters the peer for both accounts.
        route, match, _ = router.resolve('POST', f'/friends/api/v1/{b}/blocklist/{a}')
        route.handler(b, match)
        self.assertEqual(self.call(router, a, a), [entry for entry in expected if entry['accountId'] == c])
        self.assertEqual(self.call(router, b, b), [])
        friends.progression = None
        self.assertEqual(self.call(router, a, a), [])
        if state_dir:
            _, restarted, _, _ = self.setup_services(state_dir)
            self.assertEqual(restarted.recent_players(a), expected)

    def test_memory_and_durable(self):
        with self.subTest(storage='memory'):
            self.exercise(None)
        with tempfile.TemporaryDirectory() as directory, self.subTest(storage='sqlite'):
            self.exercise(directory)

    def test_failed_result_never_creates_encounter(self):
        for durable in (False, True):
            with self.subTest(durable=durable), tempfile.TemporaryDirectory() as directory:
                accounts, service, _, router = self.setup_services(directory if durable else None)
                peer = accounts.create('Peer')['id']
                self.record(service, 'failed', LOCAL_ACCOUNT_ID)
                if durable:
                    with closing(sqlite3.connect(service._database)) as db, db:
                        db.execute("CREATE TRIGGER reject_recent BEFORE INSERT ON match_results "
                                   "BEGIN SELECT RAISE(ABORT, 'fail'); END")
                    with self.assertRaises(sqlite3.IntegrityError):
                        self.record(service, 'failed', peer)
                else:
                    with patch.object(service, '_changes', side_effect=RuntimeError('fail')):
                        with self.assertRaises(RuntimeError):
                            self.record(service, 'failed', peer)
                self.assertEqual(self.call(router, LOCAL_ACCOUNT_ID, LOCAL_ACCOUNT_ID), [])
                self.assertFalse(service.has_result('failed', peer))


if __name__ == '__main__':
    unittest.main()
