"""Earned banner conversion, durable and atomic across profiles."""
if __package__:
    from . import _bootstrap
else:
    import _bootstrap
import copy
import sqlite3
import tempfile
import unittest
from api_common import ApiError
from backend import Backend
from banner_reward_service import MAPPINGS
from profile_service import CommandContext, ProfileTx
from catalog import SEASON12_LEVEL_OFFER


class EarnedBannerTests(unittest.TestCase):
    def test_login_converts_known_preserves_unknown_and_existing_then_restarts(self):
        with tempfile.TemporaryDirectory() as directory:
            service = Backend(state_dir=directory).profiles
            account = 'earned'
            ctx = CommandContext(service, account, 'athena', {})
            athena, core = ctx.tx('athena'), ctx.tx('common_core')
            token, icon = next(iter(MAPPINGS.items()))
            athena.add_item('known', token.upper(), {}, 2)
            athena.add_item('unknown', 'BannerToken:unknown', {}, 1)
            core.add_item('existing', icon.upper(), {'item_seen': True, 'custom': 9})
            service._store(account, ctx.txs)
            result = service.run(account, 'athena', 'ClientQuestLogin', {})
            self.assertTrue(any(c.get('itemId') == 'known' and c['changeType'] == 'itemRemoved' for c in result['profileChanges']))
            restarted = Backend(state_dir=directory).profiles
            a, c = restarted.profile(account, 'athena'), restarted.profile(account, 'common_core')
            self.assertNotIn('known', a['items'])
            self.assertIn('unknown', a['items'])
            self.assertEqual(c['items']['existing']['attributes'], {'item_seen': True, 'custom': 9})
            self.assertEqual(sum(v['templateId'].casefold() == icon.casefold() for v in c['items'].values()), 1)
            restarted.run(account, 'athena', 'ClientQuestLogin', {})
            self.assertEqual(restarted.profile(account, 'common_core'), c)

    def test_multi_update_and_rollback_invalid_batch(self):
        service = Backend().profiles
        account = 'atomic'
        ctx = CommandContext(service, account, 'athena', {})
        tokens = list(MAPPINGS)
        ctx.tx('athena').add_item('valid', tokens[0], {})
        ctx.tx('athena').add_item('invalid', tokens[1], {}, 0)
        service._store(account, ctx.txs)
        before_a = copy.deepcopy(service.profile(account, 'athena'))
        before_c = copy.deepcopy(service.profile(account, 'common_core'))
        with self.assertRaises(ApiError):
            service.run(account, 'athena', 'ClientQuestLogin', {})
        self.assertEqual(service.profile(account, 'athena'), before_a)
        self.assertEqual(service.profile(account, 'common_core'), before_c)
        ctx = CommandContext(service, account, 'athena', {})
        ctx.tx('athena').remove_item('invalid')
        service._store(account, ctx.txs)
        response = service.run(account, 'athena', 'ClientQuestLogin', {})
        core_delta = next(x for x in response['multiUpdate'] if x['profileId'] == 'common_core')
        self.assertTrue(any(c['changeType'] == 'itemAdded' and c['item']['attributes'] == {'item_seen': False} for c in core_delta['profileChanges']))

    def test_core_level_purchase_returns_athena_and_core_deltas(self):
        service = Backend().profiles
        account = 'purchase'
        core = service.profile(account, 'common_core')
        tx = ProfileTx(service.ctx, core)
        wallet = next(k for k, v in core['items'].items() if v['templateId'].casefold() == 'currency:mtxgiveaway')
        tx.set_item_quantity(wallet, 10000)
        service._store(account, {'common_core': tx})
        response = service.run(account, 'common_core', 'PurchaseCatalogEntry', {
            'offerId': SEASON12_LEVEL_OFFER, 'purchaseQuantity': 32,
            'currency': 'MtxCurrency', 'currencySubType': '', 'expectedTotalPrice': 4800,
        })
        self.assertTrue(any(c['changeType'] == 'itemAdded' and c['item']['templateId'].casefold() == 'homebasebannericon:brs12_cowboy' for c in response['profileChanges']))
        athena = next(x for x in response['multiUpdate'] if x['profileId'] == 'athena')
        self.assertTrue(any(c['changeType'] == 'itemRemoved' for c in athena['profileChanges']))
        self.assertFalse(any(v['templateId'].casefold() == 'bannertoken:bannertoken_029_s12_cowboy' for v in service.profile(account, 'athena')['items'].values()))

    def test_sqlite_failure_rolls_back_both_profiles_and_retry_succeeds(self):
        with tempfile.TemporaryDirectory() as directory:
            service = Backend(state_dir=directory).profiles
            ctx = CommandContext(service, 'failure', 'athena', {})
            ctx.tx('athena').add_item('token', next(iter(MAPPINGS)), {})
            service._store('failure', ctx.txs)
            before = {p: copy.deepcopy(service.profile('failure', p)) for p in ('athena', 'common_core')}
            with sqlite3.connect(service._database) as db:
                db.execute("CREATE TRIGGER reject_core BEFORE INSERT ON profiles "
                           "WHEN NEW.profile_id = 'common_core' "
                           "BEGIN SELECT RAISE(ABORT, 'test core failure'); END")
            with self.assertRaises(sqlite3.IntegrityError):
                service.run('failure', 'athena', 'ClientQuestLogin', {})
            for p in before:
                self.assertEqual(service.profile('failure', p), before[p])
            restarted = Backend(state_dir=directory).profiles
            for p in before:
                self.assertEqual(restarted.profile('failure', p), before[p])
            with sqlite3.connect(service._database) as db:
                db.execute('DROP TRIGGER reject_core')
            restarted.run('failure', 'athena', 'ClientQuestLogin', {})
            self.assertNotIn('token', restarted.profile('failure', 'athena')['items'])
            self.assertTrue(any(v['templateId'].casefold() == next(iter(MAPPINGS.values())).casefold()
                                for v in restarted.profile('failure', 'common_core')['items'].values()))
