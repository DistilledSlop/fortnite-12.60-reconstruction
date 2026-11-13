"""Native interaction response, authentication, durability and real co-play."""
if __package__:
    from . import _bootstrap
else:
    import _bootstrap

import tempfile
import unittest

from account_service import AccountService
from api_common import ApiError, Context, LOCAL_ACCOUNT_ID, Request, Router
from friends_service import FriendsService
from interactions_service import InteractionsService
from oauth_service import OAuthService
from profile_service import ProfileService
from progression_service import ProgressionService


class InteractionTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.ctx = Context(clock=lambda: 1700000000)
        self.accounts = AccountService(self.ctx, state_dir=self.tmp.name)
        self.oauth = OAuthService(self.ctx, self.accounts)
        self.friends = FriendsService(self.ctx, self.accounts, self.oauth, state_dir=self.tmp.name)
        self.addCleanup(self.friends.close)
        profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        self.progression = ProgressionService(self.ctx, self.accounts, profiles, state_dir=self.tmp.name)
        self.service = self.new_service()
        self.a = LOCAL_ACCOUNT_ID
        self.b = self.accounts.create('Interaction Bee')['id']
        self.c = self.accounts.create('Interaction Cee')['id']

    def new_service(self):
        service = InteractionsService(self.ctx, self.accounts, self.oauth, self.friends,
                                      self.progression, state_dir=self.tmp.name)
        self.addCleanup(service.close)
        return service

    def req(self, account=None):
        headers = {}
        if account:
            token = self.oauth._issue('test', account, 'client').access_token
            headers['Authorization'] = 'Bearer ' + token
        return Request('GET', '/api/v1/Fortnite/get', headers=headers)

    def query(self, account):
        return self.service.query(self.req(account))[1]['interactions']

    def test_authentication_isolation_and_persistence(self):
        router = Router()
        self.service.register(router)
        route, match, _ = router.resolve('GET', '/api/v1/Fortnite/get')
        with self.assertRaises(ApiError) as error:
            route.handler(self.req(), match)
        self.assertEqual(error.exception.status, 401)
        client = self.oauth._issue('client_credentials', None, 'client')
        with self.assertRaises(ApiError):
            route.handler(Request('GET', '/', headers={'Authorization': 'Bearer ' + client.access_token}), match)
        self.assertEqual(route.handler(self.req(self.a), match), (200, {'interactions': []}))
        self.service.record(self.a, self.b, 'FriendshipCreate', 1000)
        self.service.record(self.b, self.a, 'FriendshipAccept', 2000)
        self.service.record(self.b, self.c, 'PingSent', 3000)
        expected = [{'fromAccountId': self.a, 'toAccountId': self.b, 'interactionType': 'FriendshipCreate', 'happenedAt': 1000},
                    {'fromAccountId': self.b, 'toAccountId': self.a, 'interactionType': 'FriendshipAccept', 'happenedAt': 2000}]
        self.assertEqual(self.query(self.a), expected)
        self.assertEqual(len(self.query(self.c)), 1)
        self.service = self.new_service()
        self.assertEqual(self.query(self.a), expected)

    def test_validation_and_clock(self):
        for args in [(self.a, self.a, 'PingSent', 0), ('invalid', self.b, 'PingSent', 0),
                     ('f' * 32, self.b, 'PingSent', 0), (self.a, self.b, 'PlayedWith', 0),
                     (self.a, self.b, 'PingSent', True), (self.a, self.b, 'PingSent', '1'),
                     (self.a, self.b, 'PingSent', 2**63)]:
            with self.subTest(args=args), self.assertRaises(ValueError):
                self.service.record(*args)
        self.assertEqual(self.query(self.a), [])
        self.service.record(self.a, self.b, 'PingSent')
        self.assertEqual(self.query(self.a)[0]['happenedAt'], 1700000000000)

    def test_committed_gameplay_and_blocks(self):
        def result(account):
            self.progression.record_result('match', {'accountId': account, 'placement': 4,
                                                     'kills': 0, 'xp': 0,
                                                     'endedAt': '2023-11-14T22:13:20Z'})
        result(self.a)
        self.assertEqual(self.query(self.a), [])
        result(self.b)
        self.service.record(self.b, self.a, 'PingSent', 1700000000001)
        self.assertEqual(self.query(self.a), [
            {'fromAccountId': self.a, 'toAccountId': self.b, 'interactionType': 'GamePlayed', 'happenedAt': 1700000000000},
            {'fromAccountId': self.b, 'toAccountId': self.a, 'interactionType': 'PingSent', 'happenedAt': 1700000000001}])
        self.friends._db.execute('INSERT INTO blocks VALUES (?,?,?)', (self.b, self.a, self.ctx.iso()))
        self.friends._db.commit()
        self.assertEqual(self.query(self.a), [])
        self.assertEqual(self.query(self.b), [])


if __name__ == '__main__':
    unittest.main()
