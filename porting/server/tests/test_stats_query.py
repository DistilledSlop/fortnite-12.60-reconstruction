"""HTTP coverage for the recovered multi-user StatsV2 query contract."""
if __package__:
    from . import _bootstrap
    from .test_services import GENERATED, ServiceTestCase, jcall
else:
    import sys
    from pathlib import Path
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    import _bootstrap
    from test_services import GENERATED, ServiceTestCase, jcall

import copy
import tempfile
from backend import Backend

PATH = '/statsproxy/api/statsv2/query'
KILLS = 'br_kills_keyboardmouse_m0_playlist_defaultsolo'
MATCHES = 'br_matchesplayed_keyboardmouse_m0_playlist_defaultsolo'
SECRET = 'stats-test-server-secret'


class StatsQueryTests(ServiceTestCase):
    def setUp(self):
        super().setUp()
        self.backend.close()
        self.state = tempfile.TemporaryDirectory(dir=GENERATED)
        self.addCleanup(self.state.cleanup)
        self.restart()
        self.alice = self.sign_in('local-username:Stats Alice')
        self.bob = self.sign_in('local-username:Stats Bob')
        self.request = {'appId': 'Fortnite', 'startDate': 0, 'endDate': 2**63 - 1,
                        'owners': [self.alice['account_id'], self.bob['account_id']],
                        'stats': [KILLS, MATCHES]}

    def restart(self):
        if hasattr(self, 'backend'):
            self.backend.close()
        self.start_backend(Backend(clock=self.clock, state_step_sec=0, state_dir=self.state.name,
                                   gameserver_secret=SECRET))
        self.addCleanup(self.backend.close)

    def call_query(self, body, token=None):
        return jcall(self.base, 'POST', PATH, body=body, token=token)

    def record(self, account, kills, ended):
        registration = {'address': '127.0.0.1', 'port': 7777, 'playlists': ['Playlist_DefaultSolo'],
                        'maxPlayers': 2, 'buildUniqueId': '12896485'}
        status, registered, _ = jcall(self.base, 'POST', '/local/gameserver/register',
                                      token=SECRET, body=registration)
        self.assertEqual(status, 200)
        sid = registered['sessionId']
        root = '/local/gameserver/' + sid
        status, _, _ = jcall(self.base, 'POST', root + '/state', token=SECRET,
                             body={'phase': 'warmup', 'numPlayers': 0, 'joinable': True})
        self.assertEqual(status, 204)
        self.assertIsNotNone(self.backend.gameservers.reserve('Playlist_DefaultSolo', [account], '12896485'))
        payload = {'accountId': account, 'kills': kills, 'placement': 1, 'xp': 0, 'endedAt': ended}
        result = jcall(self.base, 'POST', root + '/result', token=SECRET, body=payload)
        self.assertEqual(result[0], 200, result[1])
        replay = jcall(self.base, 'POST', root + '/result', token=SECRET, body=payload)
        self.assertEqual(replay[:2], result[:2])
        status, _, _ = jcall(self.base, 'POST', root + '/state', token=SECRET,
                             body={'phase': 'inprogress', 'numPlayers': 0, 'joinable': False})
        self.assertEqual(status, 204)

    def test_results_filter_bounds_retry_and_restart(self):
        a, b = self.alice['account_id'], self.bob['account_id']
        self.record(a, 3, '2026-01-01T00:00:00Z')
        self.record(a, 2, '2026-01-02T00:00:00Z')
        self.record(b, 7, '2026-01-01T00:00:00Z')
        token = self.alice['access_token']
        body = dict(self.request, owners=[b, a, b], stats=[KILLS, 'missing'])
        snapshot = copy.deepcopy(body)
        status, results, _ = self.call_query(body, token)
        self.assertEqual(status, 200, results)
        self.assertEqual(body, snapshot)
        self.assertEqual([x['accountId'] for x in results], [b, a])
        self.assertEqual([x['stats'] for x in results], [{KILLS: 7}, {KILLS: 5}])
        self.assertEqual(self.call_query(body, token)[:2], (status, results))
        bounded = dict(body, startDate=1767225600, endDate=1767225600)
        status, rows, _ = self.call_query(bounded, token)
        self.assertEqual(status, 200, rows)
        self.assertEqual([x['stats'] for x in rows], [{KILLS: 7}, {KILLS: 3}])
        self.assertTrue(all((x['startTime'], x['endTime']) == (1767225600, 1767225600) for x in rows))
        self.restart()
        token = self.sign_in('local-username:Stats Alice')['access_token']
        self.assertEqual(self.call_query(body, token)[:2], (200, results))

    def test_authorization_and_invalid_batches_are_read_only(self):
        token = self.alice['access_token']
        self.assertEqual(self.call_query(self.request)[0], 401)
        self.assertEqual(self.call_query(self.request, 'invalid-token')[0], 401)
        bad = [dict(self.request, appId=''), dict(self.request, appId=1),
               dict(self.request, owners=[]), dict(self.request, owners=['x'] * 101),
               dict(self.request, owners=[None]), dict(self.request, stats=[]),
               dict(self.request, stats=['x'] * 101), dict(self.request, stats=[1]),
               dict(self.request, startDate=True), dict(self.request, endDate=2**63),
               dict(self.request, startDate=-1), dict(self.request, startDate=2, endDate=1),
               dict(self.request, endDate='1')]
        for body in bad:
            with self.subTest(body=body):
                self.assertEqual(self.call_query(body, token)[0], 400)
        missing = 'deadbeef' * 4
        self.assertIsNone(self.backend.accounts.get(missing))
        query = dict(self.request, owners=[self.alice['account_id'], missing])
        self.assertEqual(self.call_query(query, token)[0], 404)
        self.assertIsNone(self.backend.accounts.get(missing))
        self.assertEqual(self.call_query(self.request, token)[0], 200)


if __name__ == '__main__':
    import unittest
    unittest.main()
