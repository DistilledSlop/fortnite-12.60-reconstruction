"""Exercise XMPP over real RFC 6455 frames and real local OAuth sessions."""

# Isolated mode omits the script directory from sys.path.
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import base64
import json
import re
from types import SimpleNamespace
from unittest.mock import patch
import socket
import select
import struct
import threading
import time
import unittest
import xml.etree.ElementTree as ET

from backend import Backend
from friends_service import FriendsService
from local_ws import WebSocket
from xmpp_service import XmppService


class Peer:
    def __init__(self, service):
        self.sock, server = socket.socketpair()
        self.sock.settimeout(2)
        self.reader = self.sock.makefile('rb')
        ws = WebSocket(server.makefile('rb'), server.makefile('wb'), server)
        def serve():
            try:
                service.serve(ws)
            finally:
                for stream in (ws.rfile, ws.wfile):
                    try:
                        stream.close()
                    except OSError:
                        pass
                server.close()
        self.thread = threading.Thread(target=serve, daemon=True)
        self.thread.start()

    def send(self, text):
        payload = text.encode()
        mask = b'abcd'
        size = len(payload)
        header = bytes([0x81, 0x80 | size]) if size < 126 else bytes([0x81, 0xfe]) + struct.pack('!H', size)
        self.sock.sendall(header + mask + bytes(b ^ mask[i % 4] for i, b in enumerate(payload)))

    def recv(self):
        h = self.reader.read(2)
        if len(h) < 2:
            return ''
        length = h[1] & 127
        if length == 126:
            length = struct.unpack('!H', self.reader.read(2))[0]
        elif length == 127:
            length = struct.unpack('!Q', self.reader.read(8))[0]
        data = self.reader.read(length)
        return data.decode() if h[0] & 15 == 1 else ''

    def login(self, account, token, resource='client', unqualified=False):
        self.send('<open xmlns="urn:ietf:params:xml:ns:xmpp-framing" to="example.test"/>')
        self.recv(); self.recv()
        value = base64.b64encode(('\0' + account + '\0' + token).encode()).decode()
        self.send('<auth xmlns="urn:ietf:params:xml:ns:xmpp-sasl" mechanism="PLAIN">' + value + '</auth>')
        result = self.recv()
        if 'success' not in result:
            return result
        self.send('<open xmlns="urn:ietf:params:xml:ns:xmpp-framing" to="example.test"/>')
        self.recv(); self.recv()
        ns = '' if unqualified else ' xmlns="jabber:client"'
        self.send('<iq' + ns + ' type="set" id="bind"><bind xmlns="urn:ietf:params:xml:ns:xmpp-bind"><resource>' + resource + '</resource></bind></iq>')
        return self.recv()

    def close(self):
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass
        self.reader.close()
        self.sock.close()
        self.thread.join(timeout=2)


class XmppTests(unittest.TestCase):
    def setUp(self):
        self.backend = Backend(state_step_sec=0)
        self.a = '00000000000000000000000000000001'
        self.b = self.backend.accounts.create('Guest')['id']
        self.at = self.backend.oauth.local_sign_in(self.a)['access_token']
        self.bt = self.backend.oauth.local_sign_in(self.b)['access_token']
        self.friends = FriendsService(self.backend.ctx, self.backend.accounts, self.backend.oauth)
        self.service = XmppService(self.backend.ctx, self.backend.accounts, self.backend.oauth, self.friends, self.backend.party)
        self.peers = []

    def peer(self):
        p = Peer(self.service)
        self.peers.append(p)
        return p

    def tearDown(self):
        for p in self.peers:
            p.close()
        self.service.close()
        self.friends.close()

    def test_auth_bind_presence_push_and_revocation(self):
        bad = self.peer()
        self.assertIn('not-authorized', bad.login(self.b, self.at))
        a, b = self.peer(), self.peer()
        self.assertIn(self.a + '@example.test/client', a.login(self.a, self.at))
        self.assertIn(self.b + '@example.test/client', b.login(self.b, self.bt))
        # An accepted mutual friendship authorizes the presence exchange.
        with self.friends._lock, self.friends._db:
            x, y = sorted((self.a, self.b))
            self.friends._db.execute('INSERT INTO friends VALUES (?,?,?)', (x, y, 'now'))
        a.send('<presence xmlns="jabber:client"><status>Ready &amp; waiting</status></presence>')
        self.assertEqual(ET.fromstring(b.recv()).find('{jabber:client}status').text, 'Ready & waiting')
        b.send('<presence xmlns="jabber:client"/>')
        self.assertIn(self.b, a.recv())
        self.assertIn(self.a, b.recv())
        self.service.notify(self.b, {'type': 'test', 'value': '<&'})
        self.assertEqual(json.loads(ET.fromstring(b.recv()).find('{jabber:client}body').text)['value'], '<&')
        self.backend.oauth._drop(self.backend.oauth.lookup(self.bt))
        self.assertEqual(b.recv(), '')
        self.assertIn('unavailable', a.recv())

    def test_keepalive_roster_ping_and_unknown_iq(self):
        with self.friends._lock, self.friends._db:
            x, y = sorted((self.a, self.b))
            self.friends._db.execute('INSERT INTO friends VALUES (?,?,?)', (x, y, 'now'))
        peer = self.peer()
        peer.login(self.a, self.at)
        peer.send('  \n')
        peer.send('<iq xmlns="jabber:client" type="get" id="roster"><query xmlns="jabber:iq:roster"/></iq>')
        roster = ET.fromstring(peer.recv())
        item = roster.find('{jabber:iq:roster}query/{jabber:iq:roster}item')
        self.assertEqual(item.get('jid'), self.b + '@example.test')
        self.assertEqual(item.get('subscription'), 'both')
        peer.send('<iq xmlns="jabber:client" type="get" id="ping"><ping xmlns="urn:xmpp:ping"/></iq>')
        self.assertEqual(ET.fromstring(peer.recv()).get('type'), 'result')
        peer.send('<iq xmlns="jabber:client" type="get" id="unknown"><query xmlns="urn:unknown"/></iq>')
        self.assertEqual(ET.fromstring(peer.recv()).get('type'), 'error')

    def test_unqualified_bind_session_and_presence(self):
        with self.friends._lock, self.friends._db:
            x, y = sorted((self.a, self.b))
            self.friends._db.execute('INSERT INTO friends VALUES (?,?,?)', (x, y, 'now'))
        a, b = self.peer(), self.peer()
        self.assertIn(self.a + '@example.test/client', a.login(self.a, self.at, unqualified=True))
        b.login(self.b, self.bt)
        a.send('<iq type="set" id="session"><session xmlns="urn:ietf:params:xml:ns:xmpp-session"/></iq>')
        result = ET.fromstring(a.recv())
        self.assertEqual((result.get('id'), result.get('type')), ('session', 'result'))
        a.send('<presence><status>Ready &amp; waiting</status></presence>')
        self.assertEqual(ET.fromstring(b.recv()).find('{jabber:client}status').text, 'Ready & waiting')

    def test_unqualified_stanzas_still_require_phase_and_foreign_roots_rejected(self):
        early = self.peer()
        early.send('<iq type="set"><bind xmlns="urn:ietf:params:xml:ns:xmpp-bind"/></iq>')
        self.assertEqual(early.recv(), '')
        for name in ('iq', 'presence', 'message'):
            peer = self.peer()
            peer.login(self.a, self.at, resource=name)
            peer.send('<' + name + ' xmlns="urn:foreign" type="groupchat"/>')
            self.assertEqual(peer.recv(), '')

    def test_party_room_authorization_and_relay(self):
        party = self.backend.party
        def req(token):
            return SimpleNamespace(bearer_token=lambda: token, json_object=lambda: {})
        _, created = party.create(req(self.at), None)
        pid = created['id']
        match = re.match(r'([^/]+)/([^/]+)', pid + '/' + self.b)
        party.invite(req(self.at), match)
        party.join(req(self.bt), match)
        oid = self.backend.accounts.create('Outsider')['id']
        ot = self.backend.oauth.local_sign_in(oid)['access_token']
        a, b, outsider = self.peer(), self.peer(), self.peer()
        a.login(self.a, self.at); b.login(self.b, self.bt); outsider.login(oid, ot)
        room = pid + '@muc.example.test'
        def join(peer, nick, unqualified=False):
            ns = '' if unqualified else ' xmlns="jabber:client"'
            peer.send('<presence' + ns + ' to="' + room + '/' + nick + '"><x xmlns="http://jabber.org/protocol/muc"/></presence>')
        join(outsider, 'outsider')
        self.assertIn('forbidden', outsider.recv())
        join(a, 'alice', unqualified=True)
        self.assertIn('code="110"', a.recv())
        join(b, 'bob')
        self.assertIn('/alice', b.recv())
        self.assertIn('code="110"', b.recv())
        self.assertIn('/bob', a.recv())
        def message(body, unqualified=False):
            ns = '' if unqualified else ' xmlns="jabber:client"'
            a.send('<message' + ns + ' type="groupchat" to="' + room + '"><body>' + body + '</body></message>')
        message('hello &amp; hi')
        self.assertEqual(ET.fromstring(a.recv()).find('{jabber:client}body').text, 'hello & hi')
        self.assertEqual(ET.fromstring(b.recv()).find('{jabber:client}body').text, 'hello & hi')
        message('legacy', unqualified=True)
        self.assertIn('legacy', a.recv())
        self.assertIn('legacy', b.recv())
        with self.friends._lock, self.friends._db:
            self.friends._db.execute('INSERT INTO blocks VALUES (?,?,?)', (self.a, self.b, 'now'))
        message('private')
        self.assertIn('private', a.recv())
        self.assertFalse(select.select([b.sock], [], [], 0.15)[0])
        with self.friends._lock, self.friends._db:
            self.friends._db.execute('DELETE FROM blocks')
        party.leave_or_kick(req(self.at), match)
        self.service._prune_rooms()
        self.assertIn('unavailable', b.recv())
        self.assertIn('unavailable', a.recv())
        join(b, 'bob')
        self.assertIn('forbidden', b.recv())
        a.send('<iq xmlns="jabber:client" type="get" id="info" to="' + room + '"><query xmlns="http://jabber.org/protocol/disco#info"/></iq>')
        self.assertIn('category="conference"', a.recv())
        a.send('<iq xmlns="jabber:client" type="set" id="config" to="' + room + '"><query xmlns="http://jabber.org/protocol/muc#owner"><x xmlns="jabber:x:data" type="submit"/></query></iq>')
        self.assertEqual(ET.fromstring(a.recv()).get('type'), 'result')

    def test_preauth_timeout_and_invalid_xml_namespace(self):
        with patch('xmpp_service.PREAUTH_TIMEOUT', 0.1):
            peer = self.peer()
            self.assertEqual(peer.recv(), '')
        invalid = self.peer()
        invalid.send('<open xmlns="urn:wrong"/>')
        self.assertEqual(invalid.recv(), '')
        dtd = self.peer()
        dtd.send('<!DOCTYPE open><open xmlns="urn:ietf:params:xml:ns:xmpp-framing"/>')
        self.assertEqual(dtd.recv(), '')

    def test_slow_recipient_does_not_block_notify_or_close(self):
        peer = self.peer()
        peer.login(self.a, self.at)
        peer.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1024)
        start = time.monotonic()
        for i in range(400):
            self.service.notify(self.a, {'type': 'test', 'data': 'x' * 16000, 'n': i})
        self.assertLess(time.monotonic() - start, 2)
        start = time.monotonic()
        self.service.close()
        self.assertLess(time.monotonic() - start, 2)
        peer.thread.join(timeout=1)
        self.assertFalse(peer.thread.is_alive())
        self.assertEqual(len(self.service._connections), 0)

    def test_party_notifications_follow_successful_mutations(self):
        events = []
        party = self.backend.party
        party.notify = lambda recipient, event: events.append((recipient, event))
        def request(token, body):
            return SimpleNamespace(bearer_token=lambda: token, json_object=lambda: body)
        _, created = party.create(request(self.at, {}), None)
        pid = created['id']
        events.clear()
        match = re.match(r'([^/]+)/([^/]+)', pid + '/' + self.b)
        party.invite(request(self.at, {}), match)
        self.assertEqual([recipient for recipient, _ in events], [self.b])
        self.assertTrue(events[0][1]['type'].endswith('INITIAL_INVITE'))
        events.clear()
        party.join(request(self.bt, {}), match)
        self.assertEqual({recipient for recipient, _ in events}, {self.a, self.b})
        self.assertTrue(all(event['type'].endswith('MEMBER_JOINED') for _, event in events))
        events.clear()
        with self.assertRaises(Exception):
            party.join(request(self.bt, {}), match)
        self.assertEqual(events, [])


if __name__ == '__main__':
    unittest.main()
