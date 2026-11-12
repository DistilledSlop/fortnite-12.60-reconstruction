"""Party succession and presence through the HTTP dispatcher."""
if __package__:
    from . import _bootstrap, test_services as support
else:
    import _bootstrap
    import test_services as support

import json
import unittest


class PartyLifecycleTests(support.ServiceTestCase):
    def setUp(self):
        super().setUp()
        self.captain = self.sign_in()['access_token']
        self.first_id = self.new_account('First Member')
        self.first = self.sign_in_account(self.first_id)['access_token']
        self.second_id = self.new_account('Second Member')
        self.second = self.sign_in_account(self.second_id)['access_token']
        self.events = []
        self.backend.party.notify = lambda recipient, event: self.events.append((recipient, event))
        status, party = self.call('POST', support.PARTY + '/parties', self.captain, {})
        self.assertEqual(status, 200)
        self.pid = party['id']
        for account, token in ((self.first_id, self.first), (self.second_id, self.second)):
            self.clock.now += 1
            self.assertEqual(self.call('POST', self.path(account, '/join'), token, {})[0], 200)
        self.events.clear()

    def call(self, method, path, token, body=None):
        return self.backend.respond(method, path, headers={'Authorization': 'bearer ' + token},
                                    body=b'' if body is None else json.dumps(body).encode())

    def path(self, account, suffix=''):
        return f'{support.PARTY}/parties/{self.pid}/members/{account}{suffix}'

    def presence(self, account, token):
        status, entry = self.call('GET', f'/presence/api/v1/_/{account}/last-online', token)
        self.assertEqual(status, 200)
        return entry

    def test_captain_departure_notifies_remaining_members(self):
        before = self.presence(support.LOCAL, self.captain)
        self.assertEqual(self.call('DELETE', self.path(support.LOCAL), self.captain)[0], 204)
        after = self.presence(support.LOCAL, self.captain)
        self.assertEqual((after['status'], after['partyId'], after['lastOnline']),
                         ('Online', None, before['lastOnline']))
        party = self.backend.party._parties[self.pid]
        self.assertEqual([(m['account_id'], m['role']) for m in party['members']],
                         [(self.first_id, 'CAPTAIN'), (self.second_id, 'MEMBER')])
        left = [(r, e) for r, e in self.events if e['type'].endswith('MEMBER_LEFT')]
        promoted = [(r, e) for r, e in self.events if e['type'].endswith('MEMBER_NEW_CAPTAIN')]
        self.assertEqual({r for r, _ in left}, {support.LOCAL, self.first_id, self.second_id})
        self.assertEqual({r for r, _ in promoted}, {self.first_id, self.second_id})
        self.assertEqual(len(promoted), 2)
        for _, event in promoted:
            self.assertEqual((event['party_id'], event['account_id'], event['revision']),
                             (self.pid, self.first_id, party['revision']))
        self.assertEqual(self.call('POST', self.path(self.second_id, '/promote'), self.first)[0], 204)

    def test_non_captain_departure_kick_and_offline_disband(self):
        self.assertEqual(self.call('DELETE', self.path(self.first_id), self.first)[0], 204)
        self.assertEqual((self.presence(self.first_id, self.first)['status'],
                          self.presence(self.first_id, self.first)['partyId']), ('Online', None))
        self.assertFalse(any(e['type'].endswith('MEMBER_NEW_CAPTAIN') for _, e in self.events))
        self.events.clear()
        self.assertEqual(self.call('DELETE', self.path(self.second_id), self.captain)[0], 204)
        self.assertEqual((self.presence(self.second_id, self.second)['status'],
                          self.presence(self.second_id, self.second)['partyId']), ('Online', None))
        self.assertFalse(any(e['type'].endswith('MEMBER_NEW_CAPTAIN') for _, e in self.events))
        self.assertEqual(self.call('POST', self.path(support.LOCAL, '/disconnect'), self.captain)[0], 204)
        offline = self.presence(support.LOCAL, self.captain)
        self.assertEqual(offline['status'], 'Offline')
        self.events.clear()
        self.assertEqual(self.call('DELETE', self.path(support.LOCAL), self.captain)[0], 204)
        self.assertEqual(self.presence(support.LOCAL, self.captain), offline)
        self.assertNotIn(self.pid, self.backend.party._parties)
        self.assertFalse(any(e['type'].endswith('MEMBER_NEW_CAPTAIN') for _, e in self.events))


if __name__ == '__main__':
    unittest.main()
