"""Reservation history permits trusted first results without reviving sessions."""
if __package__:
    from . import _bootstrap
else:
    import sys
    from pathlib import Path
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    import _bootstrap

import json
import re
import sqlite3
import tempfile
import unittest

from api_common import ApiError, LOCAL_ACCOUNT_ID, Request
from backend import Backend


class ParticipantTests(unittest.TestCase):
    def req(self, body, token='test'):
        return Request('POST', '/', body=json.dumps(body).encode(),
                       headers={'Authorization': 'Bearer ' + token})

    def setup_match(self, backend):
        registry = backend.gameservers
        sid = registry.register_server(self.req(dict(address='127.0.0.1', port=7777,
            playlists=['Playlist_DefaultSolo', 'Playlist_Other'], maxPlayers=4,
            buildUniqueId='build')))[1]['sessionId']
        registry.update_state(self.req(dict(phase='warmup', numPlayers=0, joinable=True,
                                            connectedPlayerIds=[])), sid)
        return sid

    def result(self, backend, sid, payload, token='test'):
        return backend.gameplay.result(self.req(payload, token), re.match(r'(.*)', sid))[1]

    def test_departure_restart_first_result_and_replay(self):
        with tempfile.TemporaryDirectory() as directory:
            backend = Backend(state_dir=directory, gameserver_secret='test')
            outsider = backend.accounts.create('Outsider')['id']
            sid = self.setup_match(backend)
            self.assertEqual(backend.gameservers.reserve('Playlist_DefaultSolo', [LOCAL_ACCOUNT_ID]), sid)
            backend.gameservers.update_state(self.req(dict(phase='inprogress', numPlayers=2,
                joinable=False, connectedPlayerIds=[LOCAL_ACCOUNT_ID, outsider])), sid)
            backend.gameservers.update_state(self.req(dict(phase='inprogress', numPlayers=1,
                joinable=False, connectedPlayerIds=[outsider])), sid)
            self.assertIsNone(backend.gameservers.join(sid, LOCAL_ACCOUNT_ID))
            backend = Backend(state_dir=directory, gameserver_secret='test')
            self.assertIsNone(backend.gameservers.session(sid))
            self.assertIsNone(backend.gameservers.session_key(LOCAL_ACCOUNT_ID, sid))
            self.assertIsNone(backend.gameservers.join(sid, LOCAL_ACCOUNT_ID))
            payload = dict(accountId=LOCAL_ACCOUNT_ID, playlist='playlist_defaultsolo',
                           placement=1, kills=3, xp=30000)
            before = backend.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')
            for match_id, candidate in [(sid, dict(payload, accountId=outsider)),
                                        ('wrong-session', payload),
                                        (sid, dict(payload, playlist='Playlist_Other'))]:
                with self.assertRaises(ApiError) as caught:
                    self.result(backend, match_id, candidate)
                self.assertEqual(caught.exception.status, 403)
            self.assertEqual(before, backend.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
            receipt = self.result(backend, sid, payload)
            after = backend.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')
            self.assertEqual(after['rvn'], before['rvn'] + 1)
            backend = Backend(state_dir=directory, gameserver_secret='test')
            self.assertEqual(receipt, self.result(backend, sid, payload))
            self.assertEqual(after, backend.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
            with self.assertRaises(ApiError) as caught:
                self.result(backend, sid, dict(payload, kills=4))
            self.assertEqual(caught.exception.status, 409)
            with self.assertRaises(ApiError) as caught:
                self.result(backend, sid, payload, token='wrong')
            self.assertEqual(caught.exception.status, 401)

    def test_failed_ledger_write_rolls_back_entire_reservation(self):
        with tempfile.TemporaryDirectory() as directory:
            backend = Backend(state_dir=directory, gameserver_secret='test')
            sid = self.setup_match(backend)
            with sqlite3.connect(backend.profiles._database) as db:
                db.execute("CREATE TRIGGER fail_participant BEFORE INSERT ON match_participants "
                           "WHEN NEW.account_id='fail' BEGIN SELECT RAISE(ABORT, 'test'); END")
            with self.assertRaises(sqlite3.IntegrityError):
                backend.gameservers.reserve('Playlist_DefaultSolo', [LOCAL_ACCOUNT_ID, 'fail'])
            self.assertEqual(backend.gameservers.get_session(sid)['players'], [])
            self.assertFalse(backend.gameservers.result_participant(sid, LOCAL_ACCOUNT_ID, 'Playlist_DefaultSolo'))

    def test_in_memory_departure(self):
        backend = Backend(gameserver_secret='test')
        sid = self.setup_match(backend)
        backend.gameservers.reserve('Playlist_DefaultSolo', [LOCAL_ACCOUNT_ID])
        backend.gameservers.update_state(self.req(dict(phase='inprogress', numPlayers=1,
            joinable=False, connectedPlayerIds=[LOCAL_ACCOUNT_ID])), sid)
        backend.gameservers.update_state(self.req(dict(phase='inprogress', numPlayers=0,
            joinable=False, connectedPlayerIds=[])), sid)
        payload = dict(accountId=LOCAL_ACCOUNT_ID, playlist='Playlist_DefaultSolo',
                       placement=5, kills=0, xp=100)
        self.assertEqual(self.result(backend, sid, payload), self.result(backend, sid, payload))


if __name__ == '__main__':
    unittest.main()
