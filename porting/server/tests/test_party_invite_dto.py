"""Regression for the original client's GET-party invite deserialization failure."""
if __package__:
    from . import _bootstrap, test_services as support
else:
    import _bootstrap
    import test_services as support

import json
import unittest
from urllib.parse import parse_qsl, urlsplit


class PartyInviteDtoTests(support.ServiceTestCase):
    def call(self, method, path, token, body=None):
        url = urlsplit(path)
        return self.backend.respond(
            method, url.path, query=dict(parse_qsl(url.query)),
            headers={'Authorization': 'bearer ' + token},
            body=b'' if body is None else json.dumps(body).encode())

    def test_invited_party_lookup_preserves_required_native_invite_identity(self):
        sender = self.sign_in()['access_token']
        recipient_id = self.new_account('Invite Recipient')
        recipient = self.sign_in_account(recipient_id)['access_token']
        status, party = self.call('POST', support.PARTY + '/parties', sender, {})
        self.assertEqual(status, 200)
        party_id = party['id']
        path = f'{support.PARTY}/parties/{party_id}'
        metadata = {'urn:epic:conn:platform': 'WIN'}
        events = []
        self.backend.party.notify = lambda account, event: events.append((account, event))
        self.assertEqual(self.call('POST', path + '/invites/' + recipient_id + '?sendPing=true',
                                   sender, metadata)[0], 204)

        status, fetched = self.call('GET', path, recipient)
        self.assertEqual(status, 200)
        self.assertEqual(len(fetched['invites']), 1)
        invite = fetched['invites'][0]
        # FInviteInfo's two required strings (0x140a3c730). An empty invite
        # array during creation concealed the missing nested party identity.
        self.assertEqual((invite['party_id'], invite['sent_by']),
                         (party_id, support.LOCAL))
        self.assertEqual(invite['account_id'], recipient_id)
        self.assertEqual(invite['meta'], metadata)
        self.assertLess(invite['sent_at'], invite['expires_at'])
        initial = [event for account, event in events
                   if account == recipient_id and event['type'].endswith('.INITIAL_INVITE')]
        self.assertEqual(len(initial), 1)
        self.assertEqual((initial[0]['party_id'], initial[0]['inviter_id'],
                          initial[0]['invitee_id'], initial[0]['sent'], initial[0]['meta']),
                         (party_id, support.LOCAL, recipient_id, invite['sent_at'], metadata))
        self.assertNotIn('invite', initial[0])

        status, parties = self.call('GET',
            f'{support.PARTY}/user/{recipient_id}/pings/{support.LOCAL}/parties', recipient)
        self.assertEqual((status, parties), (200, [fetched]))
        status, state = self.call('GET', f'{support.PARTY}/user/{recipient_id}', recipient)
        self.assertEqual((status, state['invites']), (200, [invite]))
        invite['party_id'] = 'modified response'
        parties[0]['invites'][0]['meta']['response'] = 'modified'
        self.assertEqual(self.call('GET', path, recipient)[1]['invites'], state['invites'])

        # The same invite still authorizes joining an invitation-only party.
        stored = self.backend.party._parties[party_id]
        stored['config']['joinability'] = 'INVITE_ONLY'
        self.assertEqual(self.call('POST', path + '/members/' + recipient_id + '/join',
                                   recipient, {})[0], 200)
        self.assertEqual(self.call('GET', path, recipient)[1]['invites'], [])


if __name__ == '__main__':
    unittest.main()
