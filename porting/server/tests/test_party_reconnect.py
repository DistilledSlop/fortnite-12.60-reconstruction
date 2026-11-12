"""Disconnected membership reattachment through the party HTTP dispatcher."""
if __package__:
    from . import _bootstrap, test_services as support
else:
    import _bootstrap
    import test_services as support

import copy
import json
import unittest

from party_service import BUILD_KEY


class PartyReconnectTests(support.ServiceTestCase):
    def setUp(self):
        super().setUp()
        self.captain = self.sign_in()['access_token']
        self.guest_id = self.new_account('Reconnect Guest')
        self.guest = self.sign_in_account(self.guest_id)['access_token']
        self.other_id = self.new_account('Reconnect Other')
        self.other = self.sign_in_account(self.other_id)['access_token']
        self.events = []
        self.backend.party.notify = lambda recipient, event: self.events.append((recipient, event))
        _, party = self.call('POST', support.PARTY + '/parties', self.captain,
                             {'meta': {BUILD_KEY: 'build'}, 'config': {'maxSize': 2}})
        self.pid = party['id']
        self.assertEqual(self.call('POST', self.path(self.guest_id, '/join'), self.guest,
                                   {'meta': {BUILD_KEY: 'build', 'old': 'state'}})[0], 200)
        self.assertEqual(self.call('POST', self.path(self.guest_id, '/disconnect'), self.guest)[0], 204)
        self.events.clear()

    def call(self, method, path, token, body=None):
        return self.backend.respond(method, path, headers={'Authorization': 'bearer ' + token},
                                    body=b'' if body is None else json.dumps(body).encode())

    def path(self, account, suffix=''):
        return f'{support.PARTY}/parties/{self.pid}/members/{account}{suffix}'

    def test_reconnect_preserves_membership_and_emits_flat_connection(self):
        party = self.backend.party._parties[self.pid]
        party['config']['joinability'] = 'CLOSED'
        party['config']['join_confirmation'] = True
        before = copy.deepcopy(party)
        member = before['members'][1]
        body = {'revision': member['revision'], 'role': 'CAPTAIN',
                'connection': {'id': 'fresh', 'meta': {'platform': 'WIN'},
                               'offline_ttl': 30, 'yield_leadership': True},
                'meta': {BUILD_KEY: 'build', 'new': 'state'}}
        status, result = self.call('POST', self.path(self.guest_id, '/join'), self.guest, body)
        self.assertEqual((status, result['status']), (200, 'JOINED'))
        _, readback = self.call('GET', f'{support.PARTY}/parties/{self.pid}', self.guest)
        self.assertEqual(len(readback['members']), 2)
        restored = readback['members'][1]
        self.assertEqual((restored['account_id'], restored['role'], restored['joined_at']),
                         (self.guest_id, member['role'], member['joined_at']))
        self.assertEqual(restored['revision'], member['revision'] + 1)
        self.assertEqual(readback['revision'], before['revision'] + 1)
        self.assertEqual(restored['meta'], body['meta'])
        self.assertEqual(restored['connections'][0]['id'], 'fresh')
        self.assertEqual(restored['connections'][0]['meta'], {'platform': 'WIN'})
        self.assertEqual((restored['connections'][0]['offline_ttl'],
                          restored['connections'][0]['yield_leadership']), (30, True))
        self.assertEqual(self.backend.party._presence[self.guest_id]['partyId'], self.pid)
        self.assertEqual({r for r, _ in self.events}, {support.LOCAL, self.guest_id})
        self.assertEqual(len(self.events), 2)
        for _, event in self.events:
            self.assertTrue(event['type'].endswith('.MEMBER_CONNECTED'))
            self.assertEqual((event['party_id'], event['ns'], event['account_id'], event['revision']),
                             (self.pid, 'Fortnite', self.guest_id, restored['revision']))
            self.assertTrue(event['sent'] and event['account_dn'])
            self.assertEqual(event['member_state_updated'], body['meta'])
            self.assertEqual(event['connection'], {'id': 'fresh', 'meta': {'platform': 'WIN'},
                                                    'disconnected_at': ''})
            self.assertNotIn('member', event)
        self.assertEqual(self.call('POST', self.path(self.guest_id, '/join'), self.guest, body)[0], 409)
        self.assertEqual(len(self.events), 2)

    def test_rejected_attempts_do_not_change_membership(self):
        party = self.backend.party._parties[self.pid]
        before = copy.deepcopy(party)
        valid = {'connection': {'id': 'fresh', 'meta': {}}}
        for token, body, status in (
            (self.other, valid, 403),
            (self.guest, {'revision': -1, **valid}, 409),
            (self.guest, {'connection': {}}, 400),
            (self.guest, {'connection': {'id': 'fresh', 'meta': []}}, 400),
            (self.guest, {**valid, 'meta': {BUILD_KEY: 'wrong'}}, 403),
        ):
            self.assertEqual(self.call('POST', self.path(self.guest_id, '/join'), token, body)[0], status)
            self.assertEqual(party, before)
            self.assertFalse(self.events)
        _, other_party = self.call('POST', support.PARTY + '/parties', self.other, {})
        self.assertEqual(self.call('POST', f"{support.PARTY}/parties/{other_party['id']}/members/{self.guest_id}/join",
                                   self.guest, valid)[0], 409)
        self.assertEqual(party, before)
        self.assertEqual(self.call('POST', self.path(self.guest_id, '/join'), self.guest, valid)[0], 200)
        self.assertEqual(party['members'][1]['meta'], {BUILD_KEY: 'build', 'old': 'state'})


if __name__ == '__main__':
    unittest.main()
