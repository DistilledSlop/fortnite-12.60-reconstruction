"""Transaction, persistence and isolation tests for local gifting."""
import copy
import sqlite3
from contextlib import closing
import tempfile
import unittest
from account_service import AccountService
from api_common import ApiError, Context
from friends_service import FriendsService
from gifting_service import GiftingService
from profile_service import ProfileService


class GiftingTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.ctx = Context(clock=lambda: 1767225600.0)
        self.accounts = AccountService(self.ctx, self.tmp.name)
        self.sender = '00000000000000000000000000000001'
        self.recipient = self.accounts.create('Recipient')['id']
        self.other = self.accounts.create('Other')['id']
        self.friends = FriendsService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        with self.friends._db:
            for recipient in (self.recipient, self.other):
                self.friends._db.execute('INSERT INTO friends VALUES (?, ?, ?)', (*sorted((self.sender, recipient)), self.ctx.iso()))
        self.events = []
        self.restart()
        self.profiles.run(self.sender, 'common_core', 'PurchaseCatalogEntry', {'offerId': 'local-free-credits', 'purchaseQuantity': 1, 'currency': 'MtxCurrency', 'currencySubType': '', 'expectedTotalPrice': 0})
        self.body = {'offerId': 'local-20200520-hitchhiker', 'receiverAccountIds': [self.recipient], 'giftWrapTemplateId': 'GiftBox:gb_default', 'personalMessage': 'Enjoy!', 'currency': 'MtxCurrency', 'currencySubType': '', 'expectedTotalPrice': 500}

    def restart(self):
        self.profiles = ProfileService(self.ctx, self.accounts, None, self.tmp.name)
        self.service = GiftingService(self.ctx, self.accounts, self.profiles, self.friends, notify=self.record_event)

    def record_event(self, recipient, event):
        # A separate connection verifies the callback observes committed data.
        with closing(sqlite3.connect(self.profiles._database)) as db, db:
            row = db.execute('SELECT data FROM profiles WHERE account_id=? AND profile_id=?',
                             (recipient, 'common_core')).fetchone()
        self.events.append((recipient, event, row))

    def tearDown(self):
        self.friends.close()
        self.tmp.cleanup()

    def gift(self, body=None):
        return self.service.run(self.sender, 'common_core', 'GiftCatalogEntry', body or self.body)

    def wallet(self):
        return next(i['quantity'] for i in self.profiles.profile(self.sender, 'common_core')['items'].values() if i['templateId'] == 'Currency:MtxGiveaway')

    def test_restart_claim_and_isolation(self):
        self.gift()
        self.assertEqual(self.wallet(), 500)
        self.assertEqual(len(self.events), 1)
        recipient, event, row = self.events[0]
        self.assertEqual(recipient, self.recipient)
        self.assertEqual(event['type'], 'gift_received')
        self.assertIn(event['giftBoxItemId'], row[0])
        core = self.profiles.profile(self.recipient, 'common_core')
        box = next(k for k, v in core['items'].items() if v['templateId'].startswith('GiftBox:'))
        athena = self.profiles.profile(self.recipient, 'athena')
        self.assertEqual(sum(i['templateId'].casefold() == 'athenadance:eid_hitchhiker' for i in athena['items'].values()), 1)
        self.restart()
        self.service.run(self.other, 'common_core', 'RemoveGiftBox', {'giftBoxItemIds': [box]})
        self.assertIn(box, self.profiles.profile(self.recipient, 'common_core')['items'])
        for _ in range(2):
            self.service.run(self.recipient, 'common_core', 'RemoveGiftBox', {'giftBoxItemIds': [box]})
        self.assertNotIn(box, self.profiles.profile(self.recipient, 'common_core')['items'])
        self.assertEqual(self.profiles.profile(self.recipient, 'athena'), athena)
        self.assertEqual(self.wallet(), 500)

    def test_all_recipients_validated_before_debit(self):
        self.profiles.grant_item(self.other, 'athena', 'AthenaDance:eid_hitchhiker')
        before = copy.deepcopy(self.profiles._profiles)
        with self.assertRaises(ApiError):
            self.gift(dict(self.body, receiverAccountIds=[self.recipient, self.other], expectedTotalPrice=1000))
        self.assertEqual(before, self.profiles._profiles)
        self.assertEqual(self.wallet(), 1000)

    def test_database_failure_rolls_back_everything(self):
        with closing(sqlite3.connect(self.profiles._database)) as db, db:
            db.execute("CREATE TRIGGER reject_gift BEFORE INSERT ON profiles WHEN NEW.account_id = '%s' BEGIN SELECT RAISE(ABORT, 'injected'); END" % self.recipient)
            rows = db.execute('SELECT * FROM profiles ORDER BY account_id, profile_id').fetchall()
        cache = copy.deepcopy(self.profiles._profiles)
        with self.assertRaises(sqlite3.IntegrityError):
            self.gift()
        self.assertEqual(cache, self.profiles._profiles)
        self.assertEqual(self.events, [])
        with closing(sqlite3.connect(self.profiles._database)) as db, db:
            self.assertEqual(rows, db.execute('SELECT * FROM profiles ORDER BY account_id, profile_id').fetchall())

    def test_message_fields_and_bounds(self):
        for changes in ({'userMessageToRecipients': 'different'},
                        {'personalMessage': 'x' * 1001},
                        {'receiverAccountIds': [self.recipient] * 101}):
            with self.subTest(changes=changes), self.assertRaises(ApiError):
                self.gift(dict(self.body, **changes))
        body = dict(self.body, userMessageToRecipients='Hello')
        del body['personalMessage']
        self.gift(body)
        core = self.profiles.profile(self.recipient, 'common_core')
        box = next(i for i in core['items'].values() if i['templateId'].startswith('GiftBox:'))
        self.assertEqual(box['attributes']['gift_box']['params']['personalMessage'], 'Hello')

    def test_rejections(self):
        for changes in ({'receiverAccountIds': [self.recipient, self.recipient], 'expectedTotalPrice': 1000}, {'expectedTotalPrice': 0}, {'offerId': 'local-free-credits'}, {'currency': 'RealMoney'}, {'giftWrapTemplateId': 'Currency:bad'}):
            with self.subTest(changes=changes), self.assertRaises(ApiError):
                self.gift(dict(self.body, **changes))
        self.profiles.run(self.recipient, 'common_core', 'SetReceiveGiftsEnabled', {'bReceiveGifts': False})
        with self.assertRaises(ApiError):
            self.gift()
        self.assertEqual(self.wallet(), 1000)

    def test_gift_includes_outfit_backpack_and_initial_styles_after_restart(self):
        self.profiles.run(self.sender, 'common_core', 'PurchaseCatalogEntry', {
            'offerId': 'local-free-credits', 'purchaseQuantity': 1,
            'currency': 'MtxCurrency', 'currencySubType': '', 'expectedTotalPrice': 0})
        self.gift(dict(self.body, offerId='local-20200520-echo', expectedTotalPrice=1200))
        self.assertEqual(self.wallet(), 800)
        self.restart()
        items = self.profiles.profile(self.recipient, 'athena')['items'].values()
        by_template = {item['templateId']: item for item in items}
        for template in ('AthenaCharacter:CID_715_Athena_Commando_F_TwinDark',
                         'AthenaBackpack:BID_489_TwinDark'):
            self.assertEqual(by_template[template]['attributes']['variants'],
                             [{'channel': 'Material', 'active': 'Mat1', 'owned': ['Mat1', 'Mat2']}])


if __name__ == '__main__':
    unittest.main()
