"""Real HTTP/WebSocket coverage of the trusted dedicated-server bridge."""

# Isolated mode omits the script directory from sys.path.
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import json
import socket
import sys
from pathlib import Path
import tempfile
import urllib.parse
import unittest


import smoke_matchmaking as ws
from backend import Backend
if __package__:
    from .test_services import GENERATED, ServiceTestCase, jcall, mcp
else:
    from test_services import GENERATED, ServiceTestCase, jcall, mcp
if __package__:
    from .test_xmpp_service import Peer
else:
    from test_xmpp_service import Peer


class BackendGameFlowTests(ServiceTestCase):
    def test_xmpp_http_upgrade_and_oauth_bind(self):
        self.addCleanup(self.backend.close)
        auth = self.sign_in()

        def upgrade(protocol):
            sock = socket.create_connection(('127.0.0.1', self.server.port), timeout=2)
            sock.settimeout(2)
            offer = '' if protocol is None else f'Sec-WebSocket-Protocol: {protocol}\r\n'
            sock.sendall((f'GET /xmpp HTTP/1.1\r\nHost: 127.0.0.1:{self.server.port}\r\n'
                          'Upgrade: websocket\r\nConnection: Upgrade\r\n'
                          'Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n'
                          'Sec-WebSocket-Version: 13\r\n' + offer + '\r\n').encode('ascii'))
            reader = sock.makefile('rb')
            status = reader.readline().decode('ascii').strip()
            headers = {}
            while True:
                line = reader.readline().decode('ascii').strip()
                if not line:
                    break
                name, _, value = line.partition(':')
                headers[name.lower()] = value.strip()
            return sock, reader, status, headers

        for protocol in (None, 'other'):
            with self.subTest(protocol=protocol):
                sock, reader, status, headers = upgrade(protocol)
                try:
                    self.assertTrue(status.startswith('HTTP/1.1 400'), status)
                    self.assertNotIn('sec-websocket-protocol', headers)
                finally:
                    reader.close()
                    sock.close()

        sock, reader, status, headers = upgrade('other, xmpp')
        try:
            self.assertTrue(status.startswith('HTTP/1.1 101'), status)
            self.assertEqual(headers.get('sec-websocket-protocol'), 'xmpp')
            self.assertEqual(headers.get('sec-websocket-accept'), 's3pPLMBiTxaQ9kYGzzhZRbK+xOo=')
            peer = Peer.__new__(Peer)
            peer.sock, peer.reader = sock, reader
            bound = peer.login(auth['account_id'], auth['access_token'], resource='integration')
            self.assertIn(auth['account_id'] + '@example.test/integration', bound)
            peer.send('<iq xmlns="jabber:client" type="get" id="ping"><ping xmlns="urn:xmpp:ping"/></iq>')
            self.assertIn('type="result"', peer.recv())
        finally:
            sock.shutdown(socket.SHUT_RDWR)
            reader.close()
            sock.close()

    def test_registered_match_to_durable_result(self):
        state = tempfile.TemporaryDirectory(dir=GENERATED)
        self.addCleanup(state.cleanup)
        secret = 'integration-server-secret'

        def restart():
            if hasattr(self, 'backend'):
                self.backend.close()
            self.start_backend(Backend(clock=self.clock, state_step_sec=0, state_dir=state.name,
                                       gameserver_secret=secret))
            self.addCleanup(self.backend.close)

        def login(name):
            status, auth, _ = jcall(self.base, 'POST', '/account/api/oauth/token',
                                    form={'grant_type': 'local_username', 'username': name})
            self.assertEqual(status, 200, auth)
            return auth

        def request(method, path, **kwargs):
            return jcall(self.base, method, path, **kwargs)[:2]

        restart()
        alice, outsider = login('Integration Alice'), login('Integration Outsider')
        account, token = alice['account_id'], alice['access_token']
        registration = {'address': '127.0.0.1', 'port': 7777, 'playlists': ['Playlist_DefaultSolo'],
                        'maxPlayers': 2, 'buildUniqueId': '12896485'}
        self.assertEqual(request('POST', '/local/gameserver/register', body=registration)[0], 401)
        status, registered = request('POST', '/local/gameserver/register', token=secret, body=registration)
        self.assertEqual(status, 200, registered)
        sid = registered['sessionId']
        root = f'/local/gameserver/{sid}'
        self.assertEqual(request('POST', root + '/state', token=secret,
                                 body={'phase': 'warmup', 'numPlayers': 0, 'joinable': True})[0], 204)
        status, ticket = request('GET', f'/fortnite/api/game/v2/matchmakingservice/ticket/player/{account}'
                                 '?bucketId=12896485:0:NAE:Playlist_DefaultSolo')
        self.assertEqual(status, 200, ticket)
        sock, stream, status_line, accepted = ws.ws_open(self.server.port,
                                                        urllib.parse.urlsplit(ticket['serviceUrl']).path, None)
        try:
            self.assertTrue(status_line.startswith('HTTP/1.1 101') and accepted)
            while True:
                opcode, payload = ws.ws_recv(stream)
                self.assertEqual(opcode, 1)
                message = json.loads(payload)
                if message['name'] == 'Play':
                    self.assertEqual(message['payload']['sessionId'], sid)
                    break
        finally:
            stream.close()
            sock.close()
        status, session = request('GET', f'/fortnite/api/matchmaking/session/{sid}')
        self.assertEqual(status, 200, session)
        self.assertEqual(session['publicPlayers'], [account])
        self.assertEqual((session['serverAddress'], session['serverPort']), ('127.0.0.1', 7777))
        self.assertEqual(request('POST', f'/fortnite/api/matchmaking/session/{sid}/join?accountId={account}')[0], 204)
        status, key = request('GET', f'/fortnite/api/game/v2/matchmaking/account/{account}/session/{sid}')
        self.assertEqual(status, 200)
        self.assertEqual(key['key'], registered['encryptionKey'])
        self.assertEqual(request('GET', f'/fortnite/api/game/v2/matchmaking/account/{outsider["account_id"]}/session/{sid}')[0], 404)

        player_path = root + '/player/' + account
        self.assertEqual(request('GET', player_path, token=token)[0], 401)
        self.assertEqual(request('GET', root + '/player/' + outsider['account_id'], token=secret)[0], 403)
        status, player = request('GET', player_path, token=secret)
        self.assertEqual(status, 200, player)
        status, envelope, _ = mcp(self.base, token, account, 'QueryProfile')
        self.assertEqual(status, 200, envelope)
        profile = envelope['profileChanges'][0]['profile']
        locker_id = profile['stats']['attributes']['last_applied_loadout']
        slots = profile['items'][locker_id]['attributes']['locker_slots_data']['slots']
        self.assertEqual(player['loadoutId'], locker_id)
        self.assertEqual(player['profileRevision'], profile['rvn'])
        self.assertEqual(player['cosmetics'], {name: slot['items'] for name, slot in slots.items()})

        result = {'accountId': account, 'kills': 3, 'placement': 1, 'xp': 30000,
                  'endedAt': '2026-01-01T00:00:00Z'}
        self.assertEqual(request('POST', root + '/result', token=token, body=result)[0], 401)
        self.assertEqual(request('POST', root + '/result', token=secret,
                                 body=dict(result, accountId=outsider['account_id']))[0], 403)
        status, receipt = request('POST', root + '/result', token=secret, body=result)
        self.assertEqual(status, 200, receipt)
        stats_path = f'/statsproxy/api/statsv2/account/{account}'
        self.assertEqual(request('GET', stats_path)[0], 401)
        status, stats = request('GET', stats_path, token=token)
        self.assertEqual(status, 200, stats)
        self.assertEqual(stats['stats']['br_kills_keyboardmouse_m0_playlist_defaultsolo'], 3)
        self.assertEqual(stats['stats']['br_matchesplayed_keyboardmouse_m0_playlist_defaultsolo'], 1)
        status, updated, _ = mcp(self.base, token, account, 'QueryProfile')
        self.assertEqual(status, 200, updated)
        updated_profile = updated['profileChanges'][0]['profile']
        self.assertEqual(updated_profile['stats']['attributes']['matchesplayed'], 1)

        restart()
        alice_again = login('integration alice')
        self.assertEqual(alice_again['account_id'], account)
        self.assertEqual(request('GET', player_path, token=secret)[0], 404)
        self.assertEqual(request('POST', root + '/result', token=secret, body=result), (200, receipt))
        self.assertEqual(request('POST', root + '/result', token=secret, body=dict(result, kills=4))[0], 409)
        self.assertEqual(request('GET', stats_path, token=alice_again['access_token']), (200, stats))
        status, replayed, _ = mcp(self.base, alice_again['access_token'], account, 'QueryProfile')
        self.assertEqual(status, 200, replayed)
        self.assertEqual(replayed['profileChanges'][0]['profile'], updated_profile)


if __name__ == '__main__':
    unittest.main()
