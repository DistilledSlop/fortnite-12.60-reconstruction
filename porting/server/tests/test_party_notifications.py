"""Recovered party notification DTOs through the HTTP dispatcher."""
if __package__:
    from . import _bootstrap, test_services as support
else:
    import _bootstrap
    import test_services as support

import json
import unittest
from urllib.parse import parse_qsl, urlsplit


class PartyNotificationTests(support.ServiceTestCase):
    def setUp(self):
        super().setUp()
        self.captain = self.sign_in()['access_token']
        self.guest_id = self.new_account('Notification Guest')
        self.guest = self.sign_in_account(self.guest_id)['access_token']
        self.other_id = self.new_account('Notification Other')
        self.other = self.sign_in_account(self.other_id)['access_token']
        self.events = []
        self.backend.party.notify = lambda recipient, event: self.events.append((recipient, event))
        status, party = self.call('POST', support.PARTY + '/parties', self.captain,
                                  {'join_info': {'meta': {'captain-key': 'value'}}})
        self.assertEqual(status, 200)
        self.pid = party['id']
        self.events.clear()

    def call(self, method, path, token, body=None):
        url = urlsplit(path)
        return self.backend.respond(method, url.path, query=dict(parse_qsl(url.query)),
                                    headers={'Authorization': 'bearer ' + token},
                                    body=b'' if body is None else json.dumps(body).encode())

    def member_path(self, account, suffix=''):
        return f'{support.PARTY}/parties/{self.pid}/members/{account}{suffix}'

    def join(self, account, token, meta):
        self.assertEqual(self.call('POST', self.member_path(account, '/join'), token,
                                   {'meta': meta})[0], 200)

    def events_of(self, kind):
        return [(recipient, event) for recipient, event in self.events
                if event['type'].endswith('.' + kind)]

    def assert_member_dto(self, event, account, meta, revision):
        self.assertEqual((event['party_id'], event['ns'], event['account_id'], event['revision']),
                         (self.pid, 'Fortnite', account, revision))
        self.assertTrue(event['sent'])
        self.assertTrue(event['account_dn'])
        self.assertEqual(event['reason'], '')
        self.assertEqual(event['member_state_removed'], [])
        self.assertEqual(event['member_state_updated'], meta)
        self.assertNotIn('member', event)

    def test_invite_flat_payload_targets_invitee_and_copies_meta(self):
        meta = {'urn:epic:conn:platform': 'WIN', 'nested': {'key': 'value'},
                'meta': 'a flat metadata key'}
        path = f'{support.PARTY}/parties/{self.pid}/invites/{self.guest_id}'
        self.assertEqual(self.call('POST', path, self.captain, meta)[0], 204)
        events = self.events_of('INITIAL_INVITE')
        self.assertEqual(len(events), 1)
        recipient, event = events[0]
        invite = self.backend.party._parties[self.pid]['invites'][0]
        self.assertEqual(recipient, self.guest_id)
        self.assertEqual((event['party_id'], event['ns'], event['sent'], event['inviter_id'],
                          event['invitee_id'], event['expires_at']),
                         (self.pid, 'Fortnite', invite['sent_at'], support.LOCAL,
                          self.guest_id, invite['expires_at']))
        self.assertEqual(event['meta'], meta)
        self.assertTrue(event['party_id'] and event['ns'] and event['inviter_id'])
        self.assertTrue(event['meta']['urn:epic:conn:platform'])
        self.assertNotIn('invite', event)
        invite['meta']['nested']['key'] = 'changed'
        self.assertEqual(event['meta'], meta)
        other_path = f'{support.PARTY}/parties/{self.pid}/invites/{self.other_id}'
        self.assertEqual(self.call('POST', other_path, self.captain, {})[0], 204)
        recipient, without_meta = self.events_of('INITIAL_INVITE')[-1]
        self.assertEqual((recipient, without_meta['meta']), (self.other_id, {}))

    def test_invite_legacy_envelope_and_malformed_meta(self):
        path = f'{support.PARTY}/parties/{self.pid}/invites/{self.guest_id}'
        self.assertEqual(self.call('POST', path, self.captain, {'meta': 'WIN'})[0], 400)
        self.assertEqual(self.backend.party._parties[self.pid]['invites'], [])
        meta = {'urn:epic:conn:platform': 'WIN'}
        self.assertEqual(self.call('POST', path, self.captain, {'meta': meta})[0], 204)
        self.assertEqual(self.events_of('INITIAL_INVITE')[-1][1]['meta'], meta)

    def test_invite_send_ping_query_boolean_overrides_legacy_body(self):
        for index, (query, body, expected) in enumerate((
                ('?sendPing=true', {'urn:epic:conn:platform': 'WIN'}, True),
                ('?sendPing=false', {'meta': {}, 'sendPing': True}, False),
                ('?sendPing=true', {'meta': {}, 'sendPing': False}, True),
                ('?sendPing=false', {'urn:epic:conn:platform': 'WIN'}, False),
                ('', {'urn:epic:conn:platform': 'WIN'}, False),
                ('', {'meta': {}, 'sendPing': True}, True))):
            with self.subTest(query=query, body=body):
                target = self.new_account('Query Guest ' + str(index))
                path = f'{support.PARTY}/parties/{self.pid}/invites/{target}'
                self.events.clear()
                self.assertEqual(self.call('POST', path + query, self.captain, body)[0], 204)
                self.assertEqual(bool(self.events_of('PING')), expected)
                self.assertEqual(any(ping['account_id'] == target for ping in self.backend.party._pings), expected)
                self.assertEqual(len(self.events_of('INITIAL_INVITE')), 1)

    def test_invite_rejects_invalid_send_ping_query_before_mutation(self):
        path = f'{support.PARTY}/parties/{self.pid}/invites/{self.guest_id}?sendPing=yes'
        self.assertEqual(self.call('POST', path, self.captain, {})[0], 400)
        self.assertEqual(self.backend.party._parties[self.pid]['invites'], [])
        self.assertEqual(self.events, [])

    def test_departure_and_succession_use_member_snapshots(self):
        self.join(self.guest_id, self.guest, {'guest-key': 'guest-value'})
        self.join(self.other_id, self.other, {'other-key': 'other-value'})
        party = self.backend.party._parties[self.pid]
        departing = party['members'][0]
        departing['meta']['nested'] = {'key': 'original'}
        departing_meta = dict(departing['meta'])
        departed_revision = departing['revision']
        successor_revision = party['members'][1]['revision']
        self.events.clear()
        self.assertEqual(self.call('DELETE', self.member_path(support.LOCAL), self.captain)[0], 204)
        left = self.events_of('MEMBER_LEFT')
        promoted = self.events_of('MEMBER_NEW_CAPTAIN')
        self.assertEqual({r for r, _ in left}, {support.LOCAL, self.guest_id, self.other_id})
        self.assertEqual({r for r, _ in promoted}, {self.guest_id, self.other_id})
        for _, event in left:
            self.assert_member_dto(event, support.LOCAL, departing_meta, departed_revision)
        for _, event in promoted:
            self.assert_member_dto(event, self.guest_id, {'guest-key': 'guest-value'}, successor_revision)
        departing['meta']['nested']['key'] = 'changed'
        left[0][1]['member_state_updated']['nested']['key'] = 'recipient mutation'
        self.assertEqual(left[1][1]['member_state_updated']['nested']['key'], 'original')
        party['members'][0]['meta']['guest-key'] = 'changed'
        self.assertEqual(promoted[0][1]['member_state_updated']['guest-key'], 'guest-value')

    def test_captain_removal_is_kicked_not_voluntary_departure(self):
        self.join(self.guest_id, self.guest, {'guest-key': 'value'})
        self.events.clear()
        self.assertEqual(self.call('DELETE', self.member_path(self.guest_id), self.captain)[0], 204)
        kicked = self.events_of('MEMBER_KICKED')
        self.assertEqual({r for r, _ in kicked}, {support.LOCAL, self.guest_id})
        self.assertFalse(self.events_of('MEMBER_LEFT'))
        for _, event in kicked:
            self.assert_member_dto(event, self.guest_id, {'guest-key': 'value'}, 0)
        self.assertEqual(self.backend.party._presence[self.guest_id]['status'], 'Online')
        self.assertIsNone(self.backend.party._presence[self.guest_id]['partyId'])

    def test_promotion_and_last_member_departure(self):
        self.join(self.guest_id, self.guest, {'guest-key': 'value'})
        self.events.clear()
        self.assertEqual(self.call('POST', self.member_path(self.guest_id, '/promote'), self.captain)[0], 204)
        promoted = self.events_of('MEMBER_NEW_CAPTAIN')
        self.assertEqual({r for r, _ in promoted}, {support.LOCAL, self.guest_id})
        for _, event in promoted:
            self.assert_member_dto(event, self.guest_id, {'guest-key': 'value'}, 0)
        self.assertEqual(self.call('DELETE', self.member_path(support.LOCAL), self.captain)[0], 204)
        self.events.clear()
        self.assertEqual(self.call('DELETE', self.member_path(self.guest_id), self.guest)[0], 204)
        left = self.events_of('MEMBER_LEFT')
        self.assertEqual(len(left), 1)
        self.assertEqual(left[0][0], self.guest_id)
        self.assert_member_dto(left[0][1], self.guest_id, {'guest-key': 'value'}, 0)


if __name__ == '__main__':
    unittest.main()
