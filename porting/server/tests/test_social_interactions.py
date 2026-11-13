"""Real social transitions feed the authenticated native interactions query."""
if __package__:
    from . import _bootstrap, test_services as support
else:
    import _bootstrap
    import test_services as support

import json


class SocialInteractionTests(support.ServiceTestCase):
    def setUp(self):
        super().setUp()
        self.at = self.sign_in()['access_token']
        self.b = self.new_account('Interaction Friend')
        self.bt = self.sign_in_account(self.b)['access_token']

    def call(self, method, path, token, body=None, **kwargs):
        return self.backend.respond(method, path,
                                    headers={'Authorization': 'Bearer ' + token},
                                    body=b'' if body is None else json.dumps(body).encode(), **kwargs)

    def events(self, token=None):
        status, body = self.call('GET', '/api/v1/Fortnite/get', token or self.at)
        self.assertEqual(status, 200)
        return body['interactions']

    def test_friend_transitions_are_recorded_once_and_for_both_accounts(self):
        forward = f'/friends/api/v1/{support.LOCAL}/friends/{self.b}'
        reverse = f'/friends/api/v1/{self.b}/friends/{support.LOCAL}'
        self.assertEqual(self.call('POST', forward, self.at)[0], 204)
        self.assertEqual(self.call('POST', forward, self.at)[0], 204)
        self.clock.now += 1
        self.assertEqual(self.call('POST', reverse, self.bt)[0], 204)
        self.assertEqual(self.call('POST', reverse, self.bt)[0], 204)
        self.clock.now += 1
        self.assertEqual(self.call('DELETE', forward, self.at)[0], 204)
        self.call('DELETE', forward, self.at)
        events = self.events()
        self.assertEqual([e['interactionType'] for e in events],
                         ['FriendshipCreate', 'FriendshipAccept', 'FriendshipDelete'])
        self.assertEqual([e['fromAccountId'] for e in events], [support.LOCAL, self.b, support.LOCAL])
        self.assertEqual([e['happenedAt'] for e in events],
                         [int((support.T0 + n) * 1000) for n in range(3)])
        self.assertEqual(self.events(self.bt), events)
        self.assertEqual(self.backend.friends.interaction_failures, 0)

    def test_invite_join_confirmation_and_departure_record_actual_transitions(self):
        status, party = self.call('POST', support.PARTY + '/parties', self.at,
                                  {'config': {'join_confirmation': True}})
        self.assertEqual(status, 200)
        base = f'{support.PARTY}/parties/{party["id"]}'
        invite = base + '/invites/' + self.b
        self.assertEqual(self.call('POST', invite, self.at, {}, query={'sendPing': 'true'})[0], 204)
        self.assertEqual(self.call('POST', invite, self.at, {})[0], 409)
        member = base + '/members/' + self.b
        self.assertEqual(self.call('POST', member + '/join', self.bt, {})[1]['status'],
                         'PENDING_CONFIRMATION')
        self.assertEqual([e['interactionType'] for e in self.events()], ['PartyInviteSent', 'PingSent'])
        self.assertEqual(self.call('POST', member + '/confirm', self.at)[0], 204)
        self.call('POST', member + '/confirm', self.at)
        self.assertEqual(self.call('DELETE', member, self.bt)[0], 204)
        self.call('DELETE', member, self.bt)
        self.assertEqual([e['interactionType'] for e in self.events()],
                         ['PartyInviteSent', 'PingSent', 'PartyJoined', 'PartyLeft'])
        self.assertEqual(self.events(self.bt), self.events())
        self.assertEqual(self.backend.party.interaction_failures, 0)

    def test_recording_failure_preserves_committed_friendship_and_is_counted(self):
        def fail(*args):
            raise OSError('storage unavailable')
        self.backend.friends.record_interaction = fail
        path = f'/friends/api/v1/{support.LOCAL}/friends/{self.b}'
        self.assertEqual(self.call('POST', path, self.at)[0], 204)
        self.assertEqual(self.backend.friends.interaction_failures, 1)
        self.assertEqual(self.events(), [])
