"""Party build claims remain optional across admission and reconnect."""
if __package__:
    from . import _bootstrap, test_services as support
else:
    import _bootstrap
    import test_services as support

import copy
import json
import unittest

from party_service import BUILD_KEY


class PartyBuildClaimTests(support.ServiceTestCase):
    def setUp(self):
        super().setUp()
        self.captain = self.sign_in()['access_token']
        self.guest_id = self.new_account('Build Claim Guest')
        self.guest = self.sign_in_account(self.guest_id)['access_token']
        self.events = []
        self.backend.party.notify = lambda recipient, event: self.events.append((recipient, event))
        status, party = self.call('POST', support.PARTY + '/parties', self.captain,
                                  {'meta': {BUILD_KEY: '1:1:12896485'}})
        self.assertEqual(status, 200)
        self.pid = party['id']
        self.party = self.backend.party._parties[self.pid]
        self.member_path = f'{support.PARTY}/parties/{self.pid}/members/{self.guest_id}'
        self.ping_path = f'{support.PARTY}/user/{self.guest_id}/pings/{support.LOCAL}'
        self.body = {'connection': {'id': 'native-connection', 'meta': {}},
                     'meta': {'urn:epic:member:dn_s': 'Build Claim Guest',
                              'urn:epic:member:joinrequestusers_j': '{"users":[]}'}}
        self.events.clear()

    def call(self, method, path, token, body=None):
        return self.backend.respond(method, path, headers={'Authorization': 'bearer ' + token},
                                    body=b'' if body is None else json.dumps(body).encode())

    def join(self, body, ping=False):
        path = self.ping_path + '/join' if ping else self.member_path + '/join'
        return self.call('POST', path, self.guest, body)

    def test_direct_and_ping_join_without_claim_preserve_metadata(self):
        for ping in (False, True):
            with self.subTest(ping=ping):
                if ping:
                    self.assertEqual(self.call('POST', self.ping_path, self.captain,
                                               {BUILD_KEY: 'conflicting-ping-claim'})[0], 204)
                status, result = self.join(self.body, ping)
                self.assertEqual((status, result['status']), (200, 'JOINED'))
                self.assertEqual(self.party['members'][1]['meta'], self.body['meta'])
                self.assertEqual(self.call('DELETE', self.member_path, self.guest)[0], 204)

    def test_rejects_explicit_mismatch_without_state_changes(self):
        for ping in (False, True):
            with self.subTest(ping=ping):
                if ping:
                    self.assertEqual(self.call('POST', self.ping_path, self.captain, {})[0], 204)
                before = copy.deepcopy(self.party)
                applicants = copy.deepcopy(self.backend.party._applicants)
                self.events.clear()
                body = {**self.body, 'meta': {BUILD_KEY: 'wrong'}}
                self.assertEqual(self.join(body, ping)[0], 403)
                self.assertEqual(self.party, before)
                self.assertEqual(self.backend.party._applicants, applicants)
                self.assertFalse(self.events)

    def test_confirmation_checks_only_stored_explicit_claim_against_current_build(self):
        self.party['config']['join_confirmation'] = True
        self.assertEqual(self.join(self.body)[1]['status'], 'PENDING_CONFIRMATION')
        self.party['meta'][BUILD_KEY] = 'changed'
        self.assertEqual(self.call('POST', self.member_path + '/confirm', self.captain)[0], 204)
        self.assertEqual(self.party['members'][1]['meta'], self.body['meta'])
        self.assertEqual(self.call('DELETE', self.member_path, self.guest)[0], 204)
        claim = {**self.body, 'meta': {BUILD_KEY: 'changed'}}
        self.assertEqual(self.join(claim)[1]['status'], 'PENDING_CONFIRMATION')
        self.party['meta'][BUILD_KEY] = 'newer'
        before = copy.deepcopy(self.party)
        applicants = copy.deepcopy(self.backend.party._applicants)
        self.events.clear()
        self.assertEqual(self.call('POST', self.member_path + '/confirm', self.captain)[0], 403)
        self.assertEqual(self.party, before)
        self.assertEqual(self.backend.party._applicants, applicants)
        self.assertFalse(self.events)

    def test_reconnect_rejects_bad_claim_and_accepts_omitted_new_claim(self):
        self.assertEqual(self.join(self.body)[0], 200)
        self.assertEqual(self.call('POST', self.member_path + '/disconnect', self.guest)[0], 204)
        before = copy.deepcopy(self.party)
        self.events.clear()
        bad = {'connection': {'id': 'reconnect'}, 'meta': {BUILD_KEY: 'wrong'}}
        self.assertEqual(self.join(bad)[0], 403)
        self.assertEqual(self.party, before)
        self.assertFalse(self.events)
        body = {'connection': {'id': 'reconnect'}, 'meta': {'new': 'state'}}
        self.assertEqual(self.join(body)[0], 200)
        self.assertEqual(self.party['members'][1]['meta'], body['meta'])


if __name__ == '__main__':
    unittest.main()
