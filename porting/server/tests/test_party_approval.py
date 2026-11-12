"""Approval lifecycle through Backend's serialized HTTP dispatcher."""
if __package__:
    from . import _bootstrap, test_services as support
else:
    import _bootstrap
    import test_services as support

import json
import unittest


class PartyApprovalTests(support.ServiceTestCase):
    def setUp(self):
        super().setUp()
        self.captain = self.sign_in()['access_token']
        self.guest_id = self.new_account('Approval Guest')
        self.guest = self.sign_in_account(self.guest_id)['access_token']
        self.other_id = self.new_account('Approval Other')
        self.other = self.sign_in_account(self.other_id)['access_token']
        self.events = []
        self.backend.party.notify = lambda recipient, event: self.events.append((recipient, event))

    def call_backend(self, method, path, token, body=None):
        return self.backend.respond(method, path, headers={'Authorization': 'bearer ' + token},
                                    body=b'' if body is None else json.dumps(body).encode())

    def create(self, token=None, **config):
        status, party = self.call_backend('POST', support.PARTY + '/parties', token or self.captain,
                                          {'config': config})
        self.assertEqual(status, 200, party)
        return party['id']

    def path(self, pid, account, suffix=''):
        return f'{support.PARTY}/parties/{pid}/members/{account}{suffix}'

    def join(self, pid, account=None, token=None, body=None):
        return self.call_backend('POST', self.path(pid, account or self.guest_id, '/join'),
                                 token or self.guest, body if body is not None else {})

    def decide(self, pid, action, token=None, body=None):
        return self.call_backend('POST', self.path(pid, self.guest_id, '/' + action),
                                 token or self.captain, body)

    def test_pending_event_authorization_and_admission(self):
        pid = self.create(join_confirmation=True)
        self.events.clear()
        meta = {'urn:epic:member:joinrequestusers': '{"users":[]}'}
        status, result = self.join(pid, body={'meta': meta, 'connection': {'id': 'c'}})
        self.assertEqual((status, result), (200, {'party_id': pid, 'status': 'PENDING_CONFIRMATION'}))
        self.assertEqual(len(self.events), 1)
        recipient, event = self.events[0]
        self.assertEqual(recipient, support.LOCAL)
        self.assertEqual(event['type'], 'com.epicgames.social.party.notification.v0.MEMBER_REQUIRE_CONFIRMATION')
        self.assertEqual((event['party_id'], event['ns'], event['account_id']), (pid, 'Fortnite', self.guest_id))
        self.assertEqual(event['member_state_updated'], meta)
        self.assertEqual(event['member_state_removed'], [])
        self.assertEqual(event['connection'], {'id': 'c', 'disconnected_at': '', 'meta': {}})
        self.assertIsInstance(event['account_dn'], str)
        self.assertIsInstance(event['sent'], str)
        self.assertIsInstance(event['revision'], int)
        party = self.call_backend('GET', f'{support.PARTY}/parties/{pid}', self.guest)[1]
        self.assertEqual(len(party['members']), 1)
        state = self.call_backend('GET', f'{support.PARTY}/user/{self.guest_id}', self.guest)[1]
        self.assertEqual(state['current'], [])
        self.assertNotIn(self.guest_id, self.backend.party._presence)
        self.assertEqual(self.decide(pid, 'confirm', self.other)[0], 403)
        self.assertEqual(self.decide(pid, 'reject', body={'reason': []})[0], 400)
        self.clock.now += 5
        self.assertEqual(self.decide(pid, 'confirm')[0], 204)
        joined = [event for recipient, event in self.events if recipient == self.guest_id and event['type'].endswith('MEMBER_JOINED')][-1]
        self.assertEqual((joined['ns'], joined['account_id'], joined['revision']), ('Fortnite', self.guest_id, 0))
        self.assertEqual(joined['connection'], {'id': 'c', 'disconnected_at': '', 'meta': {}})
        self.assertEqual(joined['member_state_updated'], meta)
        self.assertNotIn('member', joined)
        party = self.call_backend('GET', f'{support.PARTY}/parties/{pid}', self.guest)[1]
        self.assertEqual(party['members'][1]['meta'], meta)
        self.assertEqual(party['members'][1]['connections'][0]['id'], 'c')
        self.assertEqual(party['members'][1]['joined_at'], self.backend.ctx.iso())
        self.backend.party._parties[pid]['members'][1]['meta']['later'] = 'change'
        self.assertNotIn('later', joined['member_state_updated'])
        self.assertNotIn('later', event['member_state_updated'])
        self.assertEqual(self.decide(pid, 'confirm')[0], 404)

    def test_expiry_rejection_and_validation(self):
        pid = self.create(join_confirmation=True, intentions_ttl_seconds=5)
        self.assertEqual(self.join(pid, body={'connection': []})[0], 400)
        self.assertEqual(self.decide(pid, 'confirm')[0], 404)
        self.assertEqual(self.join(pid)[0], 200)
        self.clock.now += 5
        self.assertEqual(self.decide(pid, 'confirm')[0], 404)
        self.assertEqual(self.join(pid, body={'meta': {'key': 'value'}, 'connection': {'id': 'reject-connection', 'meta': {'device': 'pc'}}})[0], 200)
        self.events.clear()
        self.assertEqual(self.decide(pid, 'reject', body={'reason': '17'})[0], 204)
        self.assertEqual(len(self.events), 1)
        recipient, event = self.events[0]
        self.assertEqual(recipient, self.guest_id)
        self.assertEqual(event['type'], 'com.epicgames.social.party.notification.v0.MEMBER_REJECTED')
        self.assertEqual((event['party_id'], event['ns'], event['account_id'], event['reason']),
                         (pid, 'Fortnite', self.guest_id, '17'))
        self.assertEqual((event['member_state_removed'], event['member_state_updated'], event['revision']), ([], {'key': 'value'}, 0))
        self.assertIsInstance(event['account_dn'], str)
        self.assertIsInstance(event['sent'], str)
        self.assertEqual(event['connection'], {'id': 'reject-connection', 'disconnected_at': '', 'meta': {'device': 'pc'}})
        self.assertEqual(self.decide(pid, 'confirm')[0], 404)
        self.assertEqual(self.call_backend('GET', f'{support.PARTY}/user/{self.guest_id}', self.guest)[1]['current'], [])

    def test_capacity_build_and_competing_requests(self):
        pid = self.create(join_confirmation=True, maxSize=2)
        party = self.backend.party._parties[pid]
        party['meta']['urn:epic:cfg:build-id_s'] = 'one'
        self.assertEqual(self.join(pid, body={'meta': {'urn:epic:cfg:build-id_s': 'bad'}})[0], 403)
        body = {'meta': {'urn:epic:cfg:build-id_s': 'one'}}
        self.assertEqual(self.join(pid, body=body)[0], 200)
        self.assertEqual(self.join(pid, self.other_id, self.other, body)[0], 200)
        self.assertEqual(self.call_backend('POST', self.path(pid, self.other_id, '/confirm'), self.captain)[0], 204)
        self.assertEqual(self.decide(pid, 'confirm')[0], 409)
        self.assertEqual(self.call_backend('DELETE', self.path(pid, self.other_id), self.other)[0], 204)
        second = self.create(self.other, join_confirmation=True)
        self.assertEqual(self.join(second)[0], 200)
        self.assertEqual(self.decide(second, 'confirm', self.other)[0], 204)
        self.assertEqual(self.decide(pid, 'confirm')[0], 404)

    def test_private_party_invite_consumed_only_on_admission(self):
        pid = self.create(join_confirmation=True, joinability='INVITE_ONLY')
        invite = f'{support.PARTY}/parties/{pid}/invites/{self.guest_id}'
        self.assertEqual(self.join(pid)[0], 403)
        self.assertEqual(self.call_backend('POST', invite, self.captain, {})[0], 204)
        self.assertEqual(self.join(pid)[0], 200)
        party = self.backend.party._parties[pid]
        self.assertEqual(len(party['invites']), 1)
        self.assertEqual(self.decide(pid, 'reject')[0], 204)
        self.assertEqual(len(party['invites']), 1)
        self.assertEqual(self.join(pid)[0], 200)
        self.assertEqual(self.decide(pid, 'confirm')[0], 204)
        self.assertEqual(party['invites'], [])

    def test_captain_transfer_and_disband_cleanup(self):
        pid = self.create()
        self.assertEqual(self.join(pid, self.other_id, self.other)[1]['status'], 'JOINED')
        party = self.backend.party._parties[pid]
        party['config']['join_confirmation'] = True
        self.assertEqual(self.join(pid)[0], 200)
        self.assertEqual(self.call_backend('DELETE', self.path(pid, support.LOCAL), self.captain)[0], 204)
        self.assertEqual(self.decide(pid, 'confirm', self.captain)[0], 403)
        self.assertEqual(self.decide(pid, 'reject', self.other)[0], 204)
        self.assertEqual(self.join(pid)[0], 200)
        self.assertEqual(self.call_backend('DELETE', self.path(pid, self.other_id), self.other)[0], 204)
        self.assertNotIn(pid, self.backend.party._applicants)


if __name__ == '__main__':
    unittest.main()
