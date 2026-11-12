"""Native JoinPartyFromPing route, with the existing local admission policies."""
if __package__:
    from . import _bootstrap, test_services as support
else:
    import _bootstrap
    import test_services as support

import json
import unittest
from urllib.parse import parse_qsl, urlsplit


class NativePingJoinTests(support.ServiceTestCase):
    def setUp(self):
        super().setUp()
        self.sender = self.sign_in()['access_token']
        self.target_id = self.new_account('Ping Join Recipient')
        self.target = self.sign_in_account(self.target_id)['access_token']
        self.pid = self.call('POST', support.PARTY + '/parties', self.sender, {})[1]['id']
        self.party_path = f'{support.PARTY}/parties/{self.pid}'
        self.ping_path = f'{support.PARTY}/user/{self.target_id}/pings/{support.LOCAL}'
        self.assertEqual(self.call('POST', self.ping_path, self.sender, {})[0], 204)
        self.events = []
        self.backend.party.notify = lambda account, event: self.events.append((account, event))

    def call(self, method, path, token, body=None):
        url = urlsplit(path)
        return self.backend.respond(method, url.path, query=dict(parse_qsl(url.query)),
            headers={'Authorization': 'bearer ' + token},
            body=b'' if body is None else json.dumps(body).encode())

    def join(self, body=None, token=None):
        return self.call('POST', self.ping_path + '/join', token or self.target,
                         {} if body is None else body)

    def test_native_depart_lookup_join_delete_sequence(self):
        solo = self.call('POST', support.PARTY + '/parties', self.target, {})[1]['id']
        self.assertEqual(self.call('GET', self.ping_path + '/parties', self.target)[1][0]['id'], self.pid)
        self.assertEqual(self.call('DELETE', f'{support.PARTY}/parties/{solo}/members/{self.target_id}',
                                   self.target)[0], 204)
        body = {'connection': {'id': 'native:connection', 'meta': {'platform': 'WIN'}},
                'meta': {'urn:epic:member:dn_s': 'Ping Join Recipient'}}
        self.assertEqual(self.join(body), (200, {'party_id': self.pid, 'status': 'JOINED'}))
        member = self.backend.party._member(self.backend.party._parties[self.pid], self.target_id)
        self.assertEqual(member['meta'], body['meta'])
        self.assertEqual(member['connections'][0]['id'], body['connection']['id'])
        self.assertEqual(member['connections'][0]['meta'], body['connection']['meta'])
        self.assertTrue(any(account == self.target_id and event['type'].endswith('MEMBER_JOINED')
                            for account, event in self.events))
        self.assertEqual(len(self.backend.party._pings), 1)
        self.assertEqual(self.call('DELETE', self.ping_path, self.target)[0], 204)
        self.assertEqual(self.backend.party._pings, [])
        self.assertEqual(self.join()[0], 404)

    def test_recipient_authorization_and_invalid_ping_selection(self):
        self.assertEqual(self.join(token=self.sender)[0], 403)
        self.assertEqual(self.call('POST', self.ping_path.replace(support.LOCAL, 'unknown') + '/join',
                                   self.target, {})[0], 404)
        self.backend.party._pings[0]['expires_at'] = self.backend.ctx.iso()
        self.assertEqual(self.join()[0], 404)
        self.assertIsNone(self.backend.party._party_of(self.target_id))

    def test_sender_must_remain_in_ping_party(self):
        self.backend.party._parties[self.pid]['members'] = []
        self.assertEqual(self.join()[0], 404)
        self.assertIsNone(self.backend.party._party_of(self.target_id))

    def test_recipient_must_leave_existing_party(self):
        solo = self.call('POST', support.PARTY + '/parties', self.target, {})[1]['id']
        self.assertEqual(self.join()[0], 409)
        self.assertEqual(self.backend.party._party_of(self.target_id)['id'], solo)

    def test_ping_does_not_bypass_privacy_build_capacity_or_body_validation(self):
        party = self.backend.party._parties[self.pid]
        party['config']['joinability'] = 'INVITE_ONLY'
        self.assertEqual(self.join()[0], 403)
        self.assertEqual(self.call('POST', self.party_path + '/invites/' + self.target_id,
                                   self.sender, {})[0], 204)
        party['meta']['urn:epic:cfg:build-id_s'] = 'compatible'
        self.assertEqual(self.join()[0], 403)
        body = {'meta': {'urn:epic:cfg:build-id_s': 'compatible'}}
        party['config']['maxSize'] = 1
        self.assertEqual(self.join(body)[0], 409)
        party['config']['maxSize'] = 4
        self.assertEqual(self.join({'connection': []})[0], 400)
        self.assertEqual(self.join(body), (200, {'party_id': self.pid, 'status': 'JOINED'}))
        self.assertEqual(party['invites'], [])

    def test_ping_join_preserves_approval_flow(self):
        party = self.backend.party._parties[self.pid]
        party['config']['join_confirmation'] = True
        body = {'connection': {'id': 'pending:connection'}, 'meta': {'request': 'kept'}}
        self.assertEqual(self.join(body), (200, {'party_id': self.pid, 'status': 'PENDING_CONFIRMATION'}))
        self.assertIsNone(self.backend.party._member(party, self.target_id))
        self.assertTrue(any(account == support.LOCAL and event['type'].endswith('MEMBER_REQUIRE_CONFIRMATION')
                            for account, event in self.events))
        self.assertEqual(self.call('POST', self.party_path + '/members/' + self.target_id + '/confirm',
                                   self.sender, {})[0], 204)
        self.assertEqual(self.backend.party._member(party, self.target_id)['meta'], body['meta'])


if __name__ == '__main__':
    unittest.main()
