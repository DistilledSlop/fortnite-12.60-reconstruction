"""Exercise the client's Party-prefixed room over XMPP WebSocket frames."""

import re
import select
from types import SimpleNamespace
import unittest
from urllib.parse import quote
import xml.etree.ElementTree as ET

try:
    from . import test_xmpp_service as framework
except ImportError:
    import test_xmpp_service as framework


class PartyMucPrefixTests(unittest.TestCase):
    setUp = framework.XmppTests.setUp
    tearDown = framework.XmppTests.tearDown
    peer = framework.XmppTests.peer

    def test_named_room_relay_authorization_and_member_cleanup(self):
        party = self.backend.party
        def req(token):
            return SimpleNamespace(bearer_token=lambda: token, json_object=lambda: {})
        _, created = party.create(req(self.at), None)
        pid = created['id']
        match = re.match(r'([^/]+)/([^/]+)', pid + '/' + self.b)
        party.invite(req(self.at), match)
        party.join(req(self.bt), match)
        outsider_id = self.backend.accounts.create('Outsider')['id']
        outsider_token = self.backend.oauth.local_sign_in(outsider_id)['access_token']
        a, b, outsider = self.peer(), self.peer(), self.peer()
        a.login(self.a, self.at)
        b.login(self.b, self.bt)
        outsider.login(outsider_id, outsider_token)
        room = 'Party-' + pid + '@muc.example.test'
        alice_nick = quote('É' * 24, safe='-_.~') + ':' + self.a + ':client-resource'
        self.assertGreater(len(alice_nick), 128)
        def join(peer, address, nick):
            peer.send('<presence to="' + address + '/' + nick + '"><x xmlns="http://jabber.org/protocol/muc"><history maxstanzas="50"/></x></presence>')
        join(outsider, room, 'outsider')
        self.assertIn('forbidden', outsider.recv())
        join(a, 'party-' + pid + '@muc.example.test', 'alice')
        self.assertIn('forbidden', a.recv())
        join(a, room, alice_nick)
        self.assertEqual(ET.fromstring(a.recv()).get('from'), room + '/' + alice_nick)
        join(b, room, 'bob')
        self.assertEqual(ET.fromstring(b.recv()).get('from'), room + '/' + alice_nick)
        self.assertEqual(ET.fromstring(b.recv()).get('from'), room + '/bob')
        self.assertEqual(ET.fromstring(a.recv()).get('from'), room + '/bob')
        def message(body):
            a.send('<message type="groupchat" to="' + room + '"><body>' + body + '</body></message>')
        message('hello')
        for peer in (a, b):
            stanza = ET.fromstring(peer.recv())
            self.assertEqual(stanza.get('from'), room + '/' + alice_nick)
            self.assertEqual(stanza.find('{jabber:client}body').text, 'hello')
        a.send('<presence type="unavailable" to="' + room + '/' + alice_nick + '"/>')
        for peer in (a, b):
            stanza = ET.fromstring(peer.recv())
            self.assertEqual(stanza.get('type'), 'unavailable')
            self.assertEqual(stanza.get('from'), room + '/' + alice_nick)
        join(a, room, alice_nick)
        self.assertEqual(ET.fromstring(a.recv()).get('from'), room + '/bob')
        self.assertEqual(ET.fromstring(a.recv()).get('from'), room + '/' + alice_nick)
        self.assertEqual(ET.fromstring(b.recv()).get('from'), room + '/' + alice_nick)
        with self.friends._lock, self.friends._db:
            self.friends._db.execute('INSERT INTO blocks VALUES (?,?,?)', (self.a, self.b, 'now'))
        message('private')
        self.assertIn('private', a.recv())
        self.assertFalse(select.select([b.sock], [], [], 0.15)[0])
        with self.friends._lock, self.friends._db:
            self.friends._db.execute('DELETE FROM blocks')
        party.leave_or_kick(req(self.at), match)
        self.service._prune_rooms()
        self.assertEqual(ET.fromstring(b.recv()).get('type'), 'unavailable')
        self.assertEqual(ET.fromstring(a.recv()).get('from'), room + '/bob')
        join(b, room, 'bob')
        self.assertIn('forbidden', b.recv())
        captain = re.match(r'([^/]+)/([^/]+)', pid + '/' + self.a)
        party.leave_or_kick(req(self.at), captain)
        self.service._prune_rooms()
        self.assertEqual(ET.fromstring(a.recv()).get('from'), room + '/' + alice_nick)
        join(a, room, alice_nick)
        self.assertIn('forbidden', a.recv())


if __name__ == '__main__':
    unittest.main()
