"""Transactional Maya graph behavior using independent profiles and cooked node data."""
if __package__:
    from . import _bootstrap
else:
    import _bootstrap

from contextlib import closing
import copy
import sqlite3
import tempfile
import unittest
from unittest.mock import patch

from account_service import AccountService
from api_common import ApiError, Context, LOCAL_ACCOUNT_ID
from profile_service import CommandContext, ProfileService
from quest_service import QuestService
from season12_maya_graph_data import GRAPH_TEMPLATE, NODES, PURCHASE_TOKEN, TARGET_ITEM


COLOR = '3F80000000000000000000003F800000'
OTHER_COLOR = '000000003F800000000000003F800000'


class MayaGraphTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.ctx = Context(clock=lambda: 1700000000)
        self.accounts = AccountService(self.ctx, state_dir=self.tmp.name)
        self.restart()

    def restart(self):
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        self.profiles.quests = QuestService(self.profiles)

    def snapshot(self):
        return self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')

    def edit(self, fn):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        fn(ctx.tx('athena'))
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)

    def fixture(self, *, graph=True, outfit=True, purchased=True, keys=(), purchase_token=False):
        def add(tx):
            if graph:
                tx.add_item('maya-graph', GRAPH_TEMPLATE, {'reward_graph_purchased': purchased,
                            'reward_nodes_claimed': [], 'reward_keys': [], 'player_state': [],
                            'unlock_keys_used': 0})
            if outfit:
                tx.add_item('maya-outfit', 'AthenaCharacter:' + TARGET_ITEM,
                            {'item_seen': True, 'variants': []})
            if purchase_token:
                tx.add_item('purchase', PURCHASE_TOKEN, {})
            for index in keys:
                tx.add_item('key-' + str(index), 'Token:AthenaToken_BRS12_HeroSelection_%02d' % index, {})
        self.edit(add)

    def run_command(self, command, body, profile='athena'):
        return self.profiles.run(LOCAL_ACCOUNT_ID, profile, command, body)

    def unlock(self, node, *, graph='maya-graph', cfg=''):
        return self.run_command('UnlockRewardNode', {'rewardGraphId': graph, 'nodeId': node, 'rewardCfg': cfg})

    def preview(self, state, *, graph='maya-graph'):
        return self.run_command('SetRewardGraphConfig', {'rewardGraphId': graph, 'state': state})

    def assert_rejected_unchanged(self, fn):
        before = self.snapshot()
        with self.assertRaises(ApiError):
            fn()
        self.assertEqual(before, self.snapshot())

    def test_preview_does_not_spend_or_grant_and_rejects_invalid(self):
        self.fixture(keys=(1,))
        before = self.snapshot()['items']
        self.preview(['ERG.Node.Tier01.1', 'ERG.Node.Tier02.1'])
        items = self.snapshot()['items']
        self.assertEqual(items['maya-graph']['attributes']['player_state'], ['ERG.Node.Tier01.1', 'ERG.Node.Tier02.1'])
        self.assertEqual(items['key-1'], before['key-1'])
        self.assertEqual(items['maya-outfit'], before['maya-outfit'])
        self.assertEqual(items['maya-graph']['attributes']['reward_keys'], [])
        self.assert_rejected_unchanged(lambda: self.preview(['ERG.Node.Tier01.1', 'ERG.Node.Tier01.2']))
        self.assert_rejected_unchanged(lambda: self.preview(['ERG.Node.Tier99.1']))

    def test_permanent_choice_retry_and_tier_lock_survive_restart(self):
        self.fixture(keys=(1,))
        self.unlock('ERG.Node.Tier01.1')
        profile = self.snapshot()
        self.assertEqual(profile['items']['key-1']['quantity'], 1)
        attrs = profile['items']['maya-graph']['attributes']
        self.assertEqual(attrs['reward_nodes_claimed'], ['ERG.Node.Tier01.1'])
        self.assertEqual(attrs['reward_keys'], [{'static_key_template_id':
            'Token:AthenaToken_BRS12_HeroSelection_01', 'unlock_keys_used': 1}])
        self.restart()
        self.unlock('ERG.Node.Tier01.1')
        retry = self.snapshot()
        self.assertEqual(profile['items'], retry['items'])
        self.assertEqual(profile['rvn'], retry['rvn'])
        self.assert_rejected_unchanged(lambda: self.unlock('ERG.Node.Tier01.2'))
        self.assert_rejected_unchanged(lambda: self.preview(['ERG.Node.Tier01.2']))

    def test_wrong_key_missing_ownership_and_other_graph_are_rejected(self):
        self.fixture(keys=(2,))
        self.edit(lambda tx: tx.add_item('other-graph', 'AthenaRewardGraph:Other', {}))
        self.assert_rejected_unchanged(lambda: self.unlock('ERG.Node.Tier01.1'))
        self.assert_rejected_unchanged(lambda: self.unlock('ERG.Node.Tier02.1', graph='other-graph'))
        self.assert_rejected_unchanged(lambda: self.preview([], graph='other-graph'))
        with self.assertRaises(ApiError):
            self.run_command(
                'UnlockRewardNode', {'rewardGraphId':'maya-graph','nodeId':'ERG.Node.Tier02.1','rewardCfg':''}, 'common_core')
        self.edit(lambda tx: tx.remove_item('maya-outfit'))
        self.assert_rejected_unchanged(lambda: self.unlock('ERG.Node.Tier02.1'))

    def test_unpurchased_graph_cannot_claim_even_with_key(self):
        self.fixture(purchased=False, keys=(1,))
        self.assert_rejected_unchanged(lambda: self.unlock('ERG.Node.Tier01.1'))

    def test_failure_after_variant_mutation_rolls_back_durably(self):
        self.fixture(keys=(1,))
        before = self.snapshot()
        from profile_service import ProfileTx
        original = ProfileTx.set_item_attr
        def fail(tx, item, name, value):
            if item == 'maya-graph' and name == 'reward_keys':
                raise RuntimeError('injected failure')
            return original(tx, item, name, value)
        with patch.object(ProfileTx, 'set_item_attr', fail):
            with self.assertRaises(RuntimeError):
                self.unlock('ERG.Node.Tier01.1')
        self.assertEqual(before, self.snapshot())
        self.restart()
        self.assertEqual(before, self.snapshot())

    def test_database_rejection_keeps_profile_and_cache_unchanged(self):
        self.fixture(keys=(1,))
        before = self.snapshot()
        with closing(sqlite3.connect(self.profiles._database)) as db, db:
            db.execute("CREATE TRIGGER fail_maya BEFORE INSERT ON profiles "
                       "WHEN NEW.profile_id = 'athena' BEGIN SELECT RAISE(ABORT, 'test failure'); END")
        with self.assertRaises(sqlite3.IntegrityError):
            self.unlock('ERG.Node.Tier01.1')
        self.assertEqual(before, self.snapshot())
        self.restart()
        self.assertEqual(before, self.snapshot())

    def test_default_token_does_not_autoequip_nondefault_does(self):
        self.fixture(keys=(1, 2))
        self.edit(lambda tx: tx.set_item_attr('maya-outfit', 'variants', [
            {'channel':'Hair', 'active':'Color.015', 'owned':['Color.014', 'Color.015']},
            {'channel':'Emissive', 'active':'Emissive1', 'owned':['Emissive1']}]))
        self.unlock('ERG.Node.Tier01.0')
        self.unlock('ERG.Node.Tier02.1')
        variants = {v['channel']: v for v in self.snapshot()['items']['maya-outfit']['attributes']['variants']}
        self.assertEqual(variants['Hair']['active'], 'Color.015')
        self.assertEqual(variants['Emissive']['active'], 'Emissive2')
        self.assertIn('Emissive2', variants['Emissive']['owned'])

    def test_purchase_reconciliation_requires_graph_and_outfit(self):
        self.fixture(graph=False, outfit=False, purchase_token=True)
        self.run_command('ClientQuestLogin', {})
        self.assertIn('purchase', self.snapshot()['items'])
        graph_id = next(i for i, item in self.snapshot()['items'].items()
                        if item['templateId'].casefold() == GRAPH_TEMPLATE.casefold())
        self.run_command('ClientQuestLogin', {})
        self.assertIn('purchase', self.snapshot()['items'])
        self.assertFalse(self.snapshot()['items'][graph_id]['attributes'].get('reward_graph_purchased', False))
        self.edit(lambda tx: tx.add_item('maya-outfit', 'AthenaCharacter:' + TARGET_ITEM, {'variants': []}))
        self.run_command('ClientQuestLogin', {})
        items = self.snapshot()['items']
        self.assertNotIn('purchase', items)
        self.assertIs(items[graph_id]['attributes']['reward_graph_purchased'], True)
        self.restart()
        before = self.snapshot()
        self.run_command('ClientQuestLogin', {})
        after = self.snapshot()
        self.assertEqual(before['items'], after['items'])
        self.assertEqual(before['rvn'], after['rvn'])

    def locker(self, channel, active):
        locker_id = self.snapshot()['stats']['attributes']['last_applied_loadout']
        return self.run_command('SetCosmeticLockerSlot', {
            'lockerItem': locker_id, 'category': 'Character', 'itemToSlot': 'maya-outfit',
            'slotIndex': 0, 'variantUpdates': [{'channel': channel, 'active': active, 'owned': []}]})

    def test_rich_color_preview_claim_and_durable_retry(self):
        self.fixture(keys=(10,))
        before = self.snapshot()['items']
        tag = 'ERG.Node.Tier10.1'
        entry = 'Cosmetics.Variant.Channel.RichColor:' + COLOR
        self.preview([tag, entry])
        preview = self.snapshot()['items']
        self.assertEqual(preview['maya-outfit'], before['maya-outfit'])
        self.assertEqual(preview['key-10'], before['key-10'])
        self.assertEqual(preview['maya-graph']['attributes']['reward_keys'], [])
        self.unlock(tag, cfg=COLOR.lower())
        items = self.snapshot()['items']
        rich = next(v for v in items['maya-outfit']['attributes']['variants'] if v['channel'] == 'RichColor')
        self.assertEqual(rich, {'channel': 'RichColor', 'active': 'RichColor.' + COLOR, 'owned': ['RichColor']})
        self.assertEqual(items['key-10']['quantity'], 1)
        self.assertIn(entry, items['maya-graph']['attributes']['player_state'])
        self.restart()
        self.unlock(tag, cfg=COLOR)
        self.assertEqual(items, self.snapshot()['items'])
        self.assert_rejected_unchanged(lambda: self.unlock(tag, cfg=OTHER_COLOR))
        self.assert_rejected_unchanged(lambda: self.preview([tag, 'Cosmetics.Variant.Channel.RichColor:' + OTHER_COLOR]))
        self.locker('RichColor', 'RichColor.' + COLOR)
        self.assert_rejected_unchanged(lambda: self.locker('RichColor', 'RichColor.' + OTHER_COLOR))

    def test_default_scarf_preserves_preview_color_without_granting_it(self):
        self.fixture(keys=(10,))
        self.preview(['ERG.Node.Tier10.1', 'Cosmetics.Variant.Channel.RichColor:' + COLOR])
        self.unlock('ERG.Node.Tier10.0')
        attrs = self.snapshot()['items']['maya-graph']['attributes']
        self.assertEqual(set(attrs['player_state']), {'ERG.Node.Tier10.0',
            'Cosmetics.Variant.Channel.RichColor:' + COLOR})
        self.assertFalse(any(v.get('channel') == 'RichColor'
                             for v in self.snapshot()['items']['maya-outfit']['attributes']['variants']))

    def test_invalid_color_and_noncolor_configuration_are_atomic(self):
        self.fixture(keys=(1, 10))
        for cfg in ('', 'BAD', 'Z' * 32, '7FC0000000000000000000003F800000'):
            with self.subTest(cfg=cfg):
                self.assert_rejected_unchanged(lambda: self.unlock('ERG.Node.Tier10.1', cfg=cfg))
        self.assert_rejected_unchanged(lambda: self.unlock('ERG.Node.Tier01.1', cfg=COLOR))
        self.assert_rejected_unchanged(lambda: self.preview([
            'ERG.Node.Tier10.1', 'Cosmetics.Variant.Channel.RichColor:7FC0000000000000000000003F800000']))

    def test_locked_ordinary_style_cannot_be_replaced_through_locker(self):
        self.fixture(keys=(1,))
        self.unlock('ERG.Node.Tier01.1')
        self.locker('Hair', 'Color.015')
        self.assert_rejected_unchanged(lambda: self.locker('Hair', 'Color.014'))

    def test_captured_default_preview_including_color_has_no_grants(self):
        self.fixture(keys=range(1, 11))
        before = self.snapshot()['items']
        entries = [f'ERG.Node.Tier{tier:02}.0' for tier in range(1, 11)]
        entries.append('Cosmetics.Variant.Channel.RichColor:' + '0' * 32)
        self.preview(entries)
        items = self.snapshot()['items']
        self.assertEqual(items['maya-graph']['attributes']['player_state'], entries)
        self.assertEqual(items['maya-graph']['attributes']['reward_keys'], [])
        self.assertEqual(items['maya-graph']['attributes']['reward_nodes_claimed'], [])
        self.assertEqual(items['maya-outfit'], before['maya-outfit'])
        for tier in range(1, 11):
            self.assertEqual(items[f'key-{tier}'], before[f'key-{tier}'])

    def test_claim_updates_equipped_and_saved_locker_snapshots(self):
        self.fixture(keys=(1, 10))
        profile = self.snapshot()
        locker_id = profile['stats']['attributes']['last_applied_loadout']
        self.run_command('SetCosmeticLockerSlot', {
            'lockerItem': locker_id, 'category': 'Character', 'itemToSlot': 'maya-outfit',
            'slotIndex': 0, 'variantUpdates': []})
        locker = self.snapshot()['items'][locker_id]
        self.edit(lambda tx: tx.add_item('saved-preset', locker['templateId'],
                                         copy.deepcopy(locker['attributes'])))
        for tag, cfg in (('ERG.Node.Tier01.1', ''), ('ERG.Node.Tier10.1', COLOR)):
            self.unlock(tag, cfg=cfg)
            items = self.snapshot()['items']
            variants = items['maya-outfit']['attributes']['variants']
            for slot_id in (locker_id, 'saved-preset'):
                slot = items[slot_id]['attributes']['locker_slots_data']['slots']['Character']
                self.assertEqual(slot['activeVariants'][0]['variants'], variants)
        self.restart()
        items = self.snapshot()['items']
        for slot_id in (locker_id, 'saved-preset'):
            slot = items[slot_id]['attributes']['locker_slots_data']['slots']['Character']
            self.assertEqual(slot['activeVariants'][0]['variants'], items['maya-outfit']['attributes']['variants'])

    def test_each_cooked_choice_on_fresh_graph(self):
        # Each choice is tested separately because choices in a tier are permanent.
        self.fixture(keys=range(1, 11))
        clean = self.snapshot()
        for tag, node in NODES.items():
            with self.subTest(node=tag):
                def reset(tx):
                    for item_id in ('maya-graph', 'maya-outfit'):
                        original = clean['items'][item_id]
                        tx.add_item(item_id, original['templateId'], original['attributes'], original['quantity'])
                self.edit(reset)
                self.unlock(tag, cfg=COLOR if node.operation == 'RichColor' else '')
                items = self.snapshot()['items']
                self.assertEqual(items['maya-graph']['attributes']['reward_nodes_claimed'], [tag])
                self.assertEqual(items['key-' + str(int(tag.split('.')[2][4:]))]['quantity'], 1)
                variants = items['maya-outfit']['attributes']['variants']
                channel = node.channel.removeprefix('Cosmetics.Variant.Channel.')
                option = node.unlocked_tag.removeprefix('Cosmetics.Variant.Property.')
                self.assertIn(option, next(v for v in variants if v['channel'] == channel)['owned'])


if __name__ == '__main__':
    unittest.main()
