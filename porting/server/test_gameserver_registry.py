"""Local game-server registration and ticket integration checks."""
import base64
import json
import threading
import time
import unittest
from api_common import ApiError, Context, Request
from gameserver_service import GameServerService, HEARTBEAT_SECONDS
from matchmaking import MatchmakingService

class Accounts:
    def display_name(self, account): return account

class Clock:
    now = 1000
    def __call__(self): return self.now

class Connection:
    def __init__(self):
        self.messages = []
        self.sock = self
    def settimeout(self, timeout): pass
    def send_json(self, value): self.messages.append(value)
    def recv_message(self): raise OSError('closed')
    def close(self, *args): pass

class RegistryTests(unittest.TestCase):
    def setUp(self):
        self.clock = Clock()
        self.ctx = Context(clock=self.clock)
        self.accounts = Accounts()
        self.registry = GameServerService(self.ctx, self.accounts, None, None, shared_secret='test-secret')
        self.mm = MatchmakingService(self.ctx, self.accounts, state_step_sec=0, gameservers=self.registry)
    def req(self, body, token='test-secret'):
        return Request('POST', '/', body=json.dumps(body).encode(), headers={'Authorization': 'Bearer ' + token})
    def register(self, **updates):
        body = dict(address='127.0.0.1', port=7777, playlists=['Playlist_DefaultSolo'], maxPlayers=2, buildUniqueId='12896485')
        body.update(updates)
        return self.registry.register_server(self.req(body))[1]
    def state(self, sid, **updates):
        body = dict(phase='warmup', numPlayers=0, joinable=True)
        body.update(updates)
        return self.registry.update_state(self.req(body), sid)
    def test_auth_and_validation(self):
        for token in ('bad', ''):
            with self.assertRaises(ApiError) as error: self.registry.register_server(self.req({}, token))
            self.assertEqual(error.exception.status, 401)
        disabled = GameServerService(self.ctx, self.accounts, None, None, shared_secret='')
        with self.assertRaises(ApiError) as error: disabled.register_server(self.req({}))
        self.assertEqual(error.exception.status, 503)
        for args in (dict(port=True), dict(playlists=[]), dict(maxPlayers=0), dict(buildUniqueId='')):
            with self.assertRaises(ApiError) as error: self.register(**args)
            self.assertEqual(error.exception.status, 400)
    def test_capacity_expiry_and_keys(self):
        first, second = self.register(), self.register(port=7779)
        self.assertNotEqual(first['sessionId'], second['sessionId'])
        self.assertNotEqual(first['encryptionKey'], second['encryptionKey'])
        self.assertEqual(len(base64.b64decode(first['encryptionKey'])), 32)
        sid = first['sessionId']
        self.state(sid)
        self.assertIsNone(self.registry.reserve('other', ['x']))
        self.assertEqual(self.registry.reserve('playlist_defaultsolo', ['a', 'b']), sid)
        self.assertIsNone(self.registry.reserve('Playlist_DefaultSolo', ['c']))
        self.assertEqual(self.mm.session_key('a', sid)[1]['key'], first['encryptionKey'])
        self.assertIsNone(self.mm.session_key('c', sid))
        self.assertEqual(self.mm.join(sid, 'b'), (204, None))
        self.assertIsNone(self.mm.join(sid, 'c'))
        self.clock.now += HEARTBEAT_SECONDS
        self.assertIsNone(self.mm.session(sid))
        self.assertIsNone(self.mm.session_key('a', sid))
    def test_restart_ids_and_expired_metadata(self):
        sid = self.register()['sessionId']
        other = GameServerService(Context(clock=self.clock), self.accounts, None, None, shared_secret='test-secret')
        other_sid = other.register_server(self.req(dict(address='127.0.0.1', port=7777,
            playlists=['Playlist_DefaultSolo'], maxPlayers=2, buildUniqueId='12896485')))[1]['sessionId']
        self.assertNotEqual(sid, other_sid)
        self.clock.now += HEARTBEAT_SECONDS
        self.assertIsNone(self.registry.get_session(sid))
        self.assertNotIn('key', self.registry.get_session(sid, include_expired=True))

    def test_connected_capacity_and_departures(self):
        sid = self.register()['sessionId']
        self.state(sid, connectedPlayerIds=[])
        self.assertEqual(self.registry.reserve('Playlist_DefaultSolo', ['a', 'b']), sid)
        self.assertEqual(self.registry.reserve('Playlist_DefaultSolo', ['b', 'a']), sid)
        self.state(sid, numPlayers=2, connectedPlayerIds=['a', 'b'])
        self.assertEqual(self.mm.session(sid)[1]['openPublicPlayers'], 0)
        self.state(sid, numPlayers=1, connectedPlayerIds=['b'])
        self.assertIsNone(self.registry.join(sid, 'a'))
        self.assertEqual(self.registry.reserve('Playlist_DefaultSolo', ['c']), sid)
        self.state(sid, phase='inprogress', numPlayers=1, connectedPlayerIds=['b'])
        self.assertTrue(self.mm.session(sid)[1]['started'])
        for ids in (['b', 'b'], [''], ['b', 'c']):
            with self.assertRaises(ApiError): self.state(sid, numPlayers=1, connectedPlayerIds=ids)

    def test_build_mismatch_stays_queued(self):
        sid = self.register()['sessionId']
        self.state(sid)
        self.assertIsNone(self.registry.reserve('Playlist_DefaultSolo', ['a'], 'wrong'))
        self.assertEqual(self.registry.reserve('Playlist_DefaultSolo', ['a'], '12896485'), sid)
        _, ticket = self.mm.ticket('b', {'bucketId': 'wrong:0:NAE:Playlist_DefaultSolo'})
        conn = Connection()
        def close_when_queued(value):
            conn.messages.append(value)
            if value['payload'].get('state') == 'Queued' and len(conn.messages) > 3: raise OSError('closed')
        conn.send_json = close_when_queued
        self.mm.websocket_route(ticket['serviceUrl'].split('.com')[-1], {})(conn)
        self.assertFalse(any(m['name'] == 'Play' for m in conn.messages))

    def test_ticket_waits_then_assigns_registered_session(self):
        registered = self.register()
        sid = registered['sessionId']
        _, ticket = self.mm.ticket('a', {'bucketId': '12896485:0:NAE:Playlist_DefaultSolo', 'partyPlayerIds': 'a,b'})
        self.assertFalse(self.mm._sessions)
        conn = Connection()
        worker = threading.Thread(target=self.mm.websocket_route(ticket['serviceUrl'].split('.com')[-1], {}), args=(conn,), daemon=True)
        worker.start()
        deadline = time.monotonic() + 2
        while not any(m['payload']['state'] == 'Queued' for m in conn.messages if m['name'] == 'StatusUpdate'):
            self.assertLess(time.monotonic(), deadline)
            time.sleep(.01)
        self.assertFalse(any(m['name'] == 'Play' for m in conn.messages))
        self.state(sid)
        worker.join(2)
        self.assertFalse(worker.is_alive())
        play = [m for m in conn.messages if m['name'] == 'Play'][0]
        self.assertEqual(play['payload']['sessionId'], sid)
        self.assertEqual(self.mm.session(sid)[1]['publicPlayers'], ['a', 'b'])
        self.assertEqual(self.mm.session_key('b', sid)[1]['key'], registered['encryptionKey'])

if __name__ == '__main__': unittest.main()
