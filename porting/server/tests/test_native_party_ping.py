"""Original CreatePing uses POST on the base ping route and a flat JSON object."""
if __package__:
    from . import _bootstrap, test_services as support
else:
    import _bootstrap
    import test_services as support

import json
import unittest


class NativePartyPingTests(support.ServiceTestCase):
    def setUp(self):
        super().setUp()
        self.sender = self.sign_in()['access_token']
        self.target_id = self.new_account('Native Ping Recipient')
        self.target = self.sign_in_account(self.target_id)['access_token']
        self.events = []
        self.backend.party.notify = lambda recipient, event: self.events.append((recipient, event))
        status, party = self.call('POST', support.PARTY + '/parties', self.sender, {})
        self.assertEqual(status, 200)
        self.pid = party['id']
        self.path = f'{support.PARTY}/user/{self.target_id}/pings/{support.LOCAL}'
        self.events.clear()

    def call(self, method, path, token, body=None):
        return self.backend.respond(method, path, headers={'Authorization': 'bearer ' + token},
                                    body=b'' if body is None else json.dumps(body).encode())

    def test_native_post_preserves_flat_metadata_and_recipient_direction(self):
        meta = {'urn:epic:cfg:build-id': 'test-build', 'nested': {'value': 'original'},
                'meta': 'ordinary flat metadata'}
        self.assertEqual(self.call('POST', self.path, self.sender, meta)[0], 204)
        status, state = self.call('GET', support.PARTY + '/user/' + self.target_id, self.target)
        self.assertEqual(status, 200)
        self.assertEqual(len(state['pings']), 1)
        ping = state['pings'][0]
        self.assertEqual((ping['account_id'], ping['sent_by'], ping['party_id']),
                         (self.target_id, support.LOCAL, self.pid))
        self.assertEqual(ping['meta'], meta)
        self.assertEqual(len(self.events), 1)
        recipient, event = self.events[0]
        self.assertEqual(recipient, self.target_id)
        self.assertTrue(event['type'].endswith('.PING'))
        self.assertEqual(event['meta'], meta)
        # Notification parity is not established; verify only local copying behavior.
        self.backend.party._pings[0]['meta']['nested']['value'] = 'changed'
        self.assertEqual(event['meta']['nested']['value'], 'original')
        self.assertNotIn('urn:epic:conn:platform', event['meta'])

    def test_legacy_post_parties_alias_preserves_envelope(self):
        self.assertEqual(self.call('POST', self.path + '/parties', self.sender,
                                   {'meta': {'local': 'value'}})[0], 204)
        self.assertEqual(self.backend.party._pings[0]['meta'], {'local': 'value'})

    def test_native_post_rejects_spoofed_sender_and_malformed_body(self):
        self.assertEqual(self.call('POST', self.path, self.target, {})[0], 403)
        self.assertEqual(self.call('POST', self.path, self.sender, ['invalid'])[0], 400)
        self.assertEqual(self.call('POST', self.path, self.sender, {'meta': 'invalid'})[0], 400)
        self.assertEqual(self.backend.party._pings, [])
        self.assertEqual(self.events, [])


if __name__ == '__main__':
    unittest.main()
