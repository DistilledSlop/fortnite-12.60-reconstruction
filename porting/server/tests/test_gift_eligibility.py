"""Recipient eligibility routing, shared policy and read-only behavior."""
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import copy
import sqlite3
import tempfile
import unittest
from contextlib import closing

from api_common import ApiError
from backend import Backend


class GiftEligibilityTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.backend = Backend(state_dir=self.tmp.name, state_step_sec=0)
        self.addCleanup(self.backend.close)
        self.sender = '00000000000000000000000000000001'
        self.recipient = self.backend.accounts.create('Recipient')['id']
        self.token = self.backend.oauth.local_sign_in(self.sender)['access_token']
        self.offer = 'local-20200520-hitchhiker'
        with self.backend.friends._db:
            self.backend.friends._db.execute('INSERT INTO friends VALUES (?, ?, ?)',
                (*sorted((self.sender, self.recipient)), self.backend.ctx.iso()))

    def check(self, recipient=None, offer=None, prefix='', auth=True):
        path = f'{prefix}/api/storefront/v2/gift/check_eligibility/recipient/{recipient or self.recipient}/offer/{offer or self.offer}'
        headers = {'Authorization': 'Bearer ' + self.token} if auth else {}
        return self.backend.respond('GET', path, headers=headers)

    def test_auth_missing_offer_and_recipient(self):
        self.assertEqual(self.check(auth=False)[0], 401)
        self.assertEqual(self.check(recipient='f' * 32)[0], 404)
        self.assertEqual(self.check(offer='missing')[0], 400)
        self.assertEqual(self.check(offer='local-free-credits')[0], 400)
        self.assertEqual(self.check(recipient=self.sender)[0], 400)

    def test_success_has_no_body_and_changes_no_state(self):
        with closing(sqlite3.connect(self.backend.profiles._database)) as db:
            before = db.execute('SELECT * FROM profiles ORDER BY account_id, profile_id').fetchall()
        cache = copy.deepcopy(self.backend.profiles._profiles)
        graph = self.backend.friends._db.execute('SELECT * FROM friends').fetchall()
        for prefix in ('', '/fortnite'):
            self.assertEqual(self.check(prefix=prefix), (204, None))
        with closing(sqlite3.connect(self.backend.profiles._database)) as db:
            self.assertEqual(before, db.execute('SELECT * FROM profiles ORDER BY account_id, profile_id').fetchall())
        self.assertEqual(cache, self.backend.profiles._profiles)
        self.assertEqual(graph, self.backend.friends._db.execute('SELECT * FROM friends').fetchall())

    def test_friend_block_decline_and_ownership(self):
        friends = self.backend.friends
        with friends._db:
            friends._db.execute('DELETE FROM friends')
        self.assertEqual(self.check()[0], 400)
        with friends._db:
            friends._db.execute('INSERT INTO friends VALUES (?, ?, ?)', (*sorted((self.sender, self.recipient)), self.backend.ctx.iso()))
            friends._db.execute('INSERT INTO blocks VALUES (?, ?, ?)', (self.recipient, self.sender, self.backend.ctx.iso()))
        self.assertEqual(self.check()[0], 400)
        with friends._db:
            friends._db.execute('DELETE FROM blocks')
        profiles = self.backend.profiles
        profiles.run(self.recipient, 'common_core', 'SetReceiveGiftsEnabled', {'bReceiveGifts': False})
        status, body = self.check()
        self.assertEqual((status, body['errorCode'].split('.')[-1]), (400, 'receiver_will_not_accept_gifts'))
        profiles.run(self.recipient, 'common_core', 'SetReceiveGiftsEnabled', {'bReceiveGifts': True})
        profiles.grant_item(self.recipient, 'athena', 'AthenaDance:eid_hitchhiker')
        status, body = self.check()
        self.assertEqual((status, body['errorCode'].split('.')[-1]), (400, 'purchase_not_allowed'))

    def test_gift_rechecks_after_success(self):
        self.assertEqual(self.check(), (204, None))
        self.backend.profiles.run(self.recipient, 'common_core', 'SetReceiveGiftsEnabled', {'bReceiveGifts': False})
        with self.assertRaises(ApiError):
            self.backend.profiles.run(self.sender, 'common_core', 'GiftCatalogEntry', {
                'offerId': self.offer, 'receiverAccountIds': [self.recipient],
                'giftWrapTemplateId': 'GiftBox:gb_default', 'currency': 'MtxCurrency',
                'currencySubType': '', 'expectedTotalPrice': 500})


if __name__ == '__main__':
    unittest.main()
