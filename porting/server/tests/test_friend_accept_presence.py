"""Presence replay when friendship is accepted after clients are online."""
import json
import re
import select
import unittest
import xml.etree.ElementTree as ET
from types import SimpleNamespace
from unittest.mock import patch

try:
    from . import test_xmpp_service as xmpp_tests
except ImportError:
    import test_xmpp_service as xmpp_tests


class FriendAcceptPresenceTests(unittest.TestCase):
    setUp = xmpp_tests.XmppTests.setUp
    tearDown = xmpp_tests.XmppTests.tearDown
    peer = xmpp_tests.XmppTests.peer

    def sync(self, peer):
        peer.send('<iq xmlns="jabber:client" type="get" id="sync"><ping xmlns="urn:xmpp:ping"/></iq>')
        self.assertIn('id="sync"', peer.recv())

    def request(self, account, other):
        match = re.match(r'([^/]+)/([^/]+)', account + '/' + other)
        with patch.object(self.friends.oauth, 'account_session', return_value=SimpleNamespace(account_id=account)):
            self.assertEqual(self.friends.request_friend(None, match)[0], 204)

    def assert_event(self, peer, status):
        stanza = ET.fromstring(peer.recv())
        self.assertEqual(stanza.tag, '{jabber:client}message')
        self.assertEqual(json.loads(stanza.find('{jabber:client}body').text)['status'], status)

    def assert_presence(self, peer, source, resource, status):
        stanza = ET.fromstring(peer.recv())
        self.assertEqual(stanza.tag, '{jabber:client}presence')
        self.assertEqual(stanza.get('from'), source + '@example.test/' + resource)
        self.assertEqual(stanza.find('{jabber:client}status').text, status)

    def assert_no_data(self, peer):
        self.assertFalse(select.select([peer.sock], [], [], 0.15)[0])

    def test_accept_replays_only_available_peer_resources_after_event(self):
        self.friends.notify = self.service.notify
        a, b, b2 = self.peer(), self.peer(), self.peer()
        a.login(self.a, self.at, 'a')
        b.login(self.b, self.bt, 'b')
        b2.login(self.b, self.bt, 'b2')
        a.send('<presence xmlns="jabber:client"><status>A &amp; ready</status></presence>')
        self.sync(a)
        b2.send('<presence xmlns="jabber:client"><status>B ready</status></presence>')
        self.sync(b2)
        # Same-account resources receive normal presence regardless of friendship.
        self.assert_presence(b, self.b, 'b2', 'B ready')
        self.request(self.a, self.b)
        for peer in (a, b, b2):
            self.assert_event(peer, 'PENDING')
        self.request(self.b, self.a)
        for peer in (a, b, b2):
            self.assert_event(peer, 'ACCEPTED')
        self.assert_presence(a, self.b, 'b2', 'B ready')
        self.assert_presence(b, self.a, 'a', 'A & ready')
        self.assert_presence(b2, self.a, 'a', 'A & ready')
        self.assert_no_data(a)

    def test_forged_or_blocked_accept_does_not_replay(self):
        a, b = self.peer(), self.peer()
        a.login(self.a, self.at)
        b.login(self.b, self.bt)
        b.send('<presence xmlns="jabber:client"/>')
        self.sync(b)
        event = {'type': 'com.epicgames.friends.core.apiobjects.Friend', 'accountId': self.b, 'status': 'ACCEPTED'}
        self.service.notify(self.a, event)
        self.assert_event(a, 'ACCEPTED')
        self.assert_no_data(a)
        x, y = sorted((self.a, self.b))
        with self.friends._lock, self.friends._db:
            self.friends._db.execute('INSERT INTO friends VALUES (?,?,?)', (x, y, 'now'))
            self.friends._db.execute('INSERT INTO blocks VALUES (?,?,?)', (self.a, self.b, 'now'))
        self.service.notify(self.a, event)
        self.assert_event(a, 'ACCEPTED')
        self.assert_no_data(a)


if __name__ == '__main__':
    unittest.main()
