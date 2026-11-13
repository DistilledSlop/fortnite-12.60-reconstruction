"""Faction reward consumption; token timing is a local policy."""
if __package__:
    from . import _bootstrap
else:
    import _bootstrap

import copy
import tempfile
from types import SimpleNamespace
from unittest.mock import Mock
import unittest

from account_service import AccountService
from api_common import ApiError, Context, LOCAL_ACCOUNT_ID
from profile_service import CommandContext, ProfileService
from quest_service import QuestService
from gameplay_service import GameplayService
from season12_brutus_faction_data import TOKEN
from season12_tntina_faction_data import TOKEN as TNTINA_TOKEN


# These are the cooked branch rewards and VTID target properties, not
# inferred from the service's mapping under test.
BRANCHES = {
    'ego': ('Stage2', 'Particle2', ('VTID_506_HenchmanTough_StyleB', 'VTID_508_HenchmanTough_Backbling_StyleB', 'VTID_546_HenchmanMaleGlider_StyleB')),
    'alter': ('Stage3', 'Particle3', ('VTID_507_HenchmanTough_StyleC', 'VTID_509_HenchmanTough_Backbling_StyleC', 'VTID_547_HenchmanMaleGlider_StyleC')),
}
TARGETS = (
    ('character', 'AthenaCharacter:CID_692_Athena_Commando_M_HenchmanTough', 'Progressive', 'Stage1'),
    ('backpack', 'AthenaBackpack:BID_478_HenchmanTough', 'Progressive', 'Stage1'),
    ('glider', 'AthenaGlider:Glider_ID_197_HenchmanMale', 'Particle', 'Particle1'),
)


TNTINA_BRANCHES = {
    'ego': ('Stage2', 'Particle2', ('VTID_498_TNTina_StyleB', 'VTID_500_TNTina_Backbling_StyleB', 'VTID_544_TNTinaFemalePickaxe_StyleB', 'VTID_550_TNTinaFemaleGlider_StyleB')),
    'alter': ('Stage3', 'Particle3', ('VTID_499_TNTina_StyleC', 'VTID_501_TNTina_Backbling_StyleC', 'VTID_545_TNTinaFemalePickaxe_StyleC', 'VTID_551_TNTinaFemaleGlider_StyleC')),
}
TNTINA_TARGETS = (
    ('character', 'AthenaCharacter:CID_691_Athena_Commando_F_TNTina', 'Progressive', 'Stage1'),
    ('backpack', 'AthenaBackpack:BID_437_TNTina', 'Progressive', 'Stage1'),
    ('pickaxe', 'AthenaPickaxe:Pickaxe_ID_336_TNTinaFemale', 'Mesh', 'Stage1'),
    ('glider', 'AthenaGlider:Glider_ID_201_TNTinaFemale', 'Particle', 'Particle1'),
)


class FactionVariantRewardsTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.ctx = Context(clock=lambda: 1700000000)
        self.accounts = AccountService(self.ctx, state_dir=self.tmp.name)
        self.restart()

    def restart(self):
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        self.quests = self.profiles.quests = QuestService(self.profiles)

    def snapshot(self):
        return self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')

    def edit(self, fn):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        fn(ctx.tx('athena'))
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)

    def login(self):
        self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})

    def select(self, side, *, tntina=False):
        self.login()
        def earn(tx):
            tx.set_stat('book_purchased', True)
            tx.set_stat('level', 100)
            self.quests._award_token(tx, TNTINA_TOKEN if tntina else TOKEN, 18)
        self.edit(earn)
        prefix = 'quest:ab_tntina_' if tntina else 'quest:quest_ab_skulldude_'
        quest = next(i for i, item in self.snapshot()['items'].items()
                     if item['templateId'].casefold() == prefix + side)
        self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ToggleQuestActiveState', {'questIds': [quest]})
        return quest

    def add_targets(self, *, malformed=False, targets=TARGETS):
        def add(tx):
            for item_id, template, channel, default in targets:
                attrs = {'variants': [{'channel': channel, 'active': default, 'owned': [default]}]}
                if malformed and item_id == 'glider':
                    attrs['variants'] = 'invalid'
                tx.add_item(item_id, template, attrs)
        self.edit(add)

    def complete(self, side, *, tntina=False):
        event = ('AB_tntina_' if tntina else 'AB_skulldude_') + side
        return self.quests.progress(LOCAL_ACCOUNT_ID, event, 2 if tntina else 1, event_id='deposit')

    def assert_variants(self, side, *, branches=BRANCHES, targets=TARGETS):
        stage, particle, tokens = branches[side]
        items = self.snapshot()['items']
        for item_id, _, channel, default in targets:
            unlocked = particle if item_id == 'glider' else stage
            self.assertEqual(items[item_id]['attributes']['variants'],
                             [{'channel': channel, 'active': default, 'owned': [default, unlocked]}])
        templates = {item['templateId'].casefold() for item in items.values()}
        for token in tokens:
            self.assertNotIn(('CosmeticVariantToken:' + token).casefold(), templates)

    def test_selected_branches_unlock_all_owned_targets_and_preserve_active(self):
        for side in BRANCHES:
            with self.subTest(side=side):
                # Isolate the mutually exclusive branches in separate accounts.
                self.setUp()
                quest = self.select(side)
                self.add_targets()
                self.assertTrue(self.complete(side))
                self.assertEqual(self.snapshot()['items'][quest]['attributes']['quest_state'], 'Claimed')
                self.assert_variants(side)
                self.assertFalse(self.complete(side))
                self.restart()
                self.login()
                self.assert_variants(side)

    def test_tntina_selected_branches_unlock_all_four_owned_targets(self):
        for side in TNTINA_BRANCHES:
            with self.subTest(side=side):
                self.setUp()
                quest = self.select(side, tntina=True)
                self.add_targets(targets=TNTINA_TARGETS)
                self.assertTrue(self.complete(side, tntina=True))
                self.assertEqual(self.snapshot()['items'][quest]['attributes']['quest_state'], 'Claimed')
                self.assert_variants(side, branches=TNTINA_BRANCHES, targets=TNTINA_TARGETS)
                self.assertFalse(self.complete(side, tntina=True))
                self.restart()
                self.login()
                self.assert_variants(side, branches=TNTINA_BRANCHES, targets=TNTINA_TARGETS)

    def test_existing_nondefault_active_styles_are_preserved(self):
        self.select('ego')
        self.add_targets()
        def activate(tx):
            for item_id, _, channel, default in TARGETS:
                active = 'Particle3' if item_id == 'glider' else 'Stage3'
                tx.set_item_attr(item_id, 'variants', [
                    {'channel': channel, 'active': active, 'owned': [default, active]}])
        self.edit(activate)
        self.assertTrue(self.complete('ego'))
        for item_id, _, channel, default in TARGETS:
            active = 'Particle3' if item_id == 'glider' else 'Stage3'
            unlocked = 'Particle2' if item_id == 'glider' else 'Stage2'
            self.assertEqual(self.snapshot()['items'][item_id]['attributes']['variants'],
                             [{'channel': channel, 'active': active, 'owned': [default, active, unlocked]}])

    def test_tokens_wait_for_targets_then_login_consumes_them(self):
        side = 'alter'
        self.select(side)
        self.assertTrue(self.complete(side))
        templates = {item['templateId'].casefold() for item in self.snapshot()['items'].values()}
        for token in BRANCHES[side][2]:
            self.assertIn(('CosmeticVariantToken:' + token).casefold(), templates)
        self.restart()
        self.add_targets()
        self.login()
        self.assert_variants(side)

    def test_skye_auto_equip_updates_matching_lockers_and_persisted_loadout(self):
        self.login()
        target = 'AthenaCharacter:CID_690_Athena_Commando_F_Photographer'
        other = {'channel': 'Material', 'active': 'Gold', 'owned': ['Gold']}
        old = {'channel': 'Progressive', 'active': 'Stage1', 'owned': ['Stage1']}
        profile = self.snapshot()
        locker_id = profile['stats']['attributes']['last_applied_loadout']
        def setup(tx):
            tx.add_item('skye', target, {'variants': [old, other]})
            tx.add_item('skye-backpack', 'AthenaBackpack:BID_473_Photographer', {'variants': [old]})
            data = copy.deepcopy(tx.profile['items'][locker_id]['attributes']['locker_slots_data'])
            data['slots']['Character'] = {'items': ['skye', 'unrelated'], 'activeVariants': [
                {'variants': [old, other]}, {'variants': [{'channel': 'Other', 'active': 'Keep'}]}]}
            data['slots']['Backpack'] = {'items': ['skye-backpack'], 'activeVariants': [{'variants': [old]}]}
            tx.set_item_attr(locker_id, 'locker_slots_data', data)
            saved = copy.deepcopy(tx.profile['items'][locker_id]['attributes'])
            saved['locker_slots_data']['slots']['Character']['items'][0] = target.lower()
            tx.add_item('saved-preset', 'CosmeticLocker:athenalocker', saved)
            tx.add_item('skye-token', 'CosmeticVariantToken:VTID_502_Photographer_StyleB', {})
            tx.add_item('backpack-token', 'CosmeticVariantToken:VTID_504_Photographer_Backbling_StyleB', {})
            self.quests.consume_variants(tx)
        self.edit(setup)
        self.restart()
        profile = self.snapshot()
        for name in (locker_id, 'saved-preset'):
            slot = profile['items'][name]['attributes']['locker_slots_data']['slots']['Character']
            self.assertEqual(slot['activeVariants'][0]['variants'], [
                {'channel': 'Progressive', 'active': 'Stage2', 'owned': ['Stage1', 'Stage2']}, other])
            self.assertEqual(slot['activeVariants'][1], {'variants': [{'channel': 'Other', 'active': 'Keep'}]})
            backpack = profile['items'][name]['attributes']['locker_slots_data']['slots']['Backpack']
            self.assertEqual(backpack['activeVariants'][0]['variants'], [
                {'channel': 'Progressive', 'active': 'Stage2', 'owned': ['Stage1', 'Stage2']}])
        servers = SimpleNamespace(authorize=lambda req: None,
                                  get_session=lambda session_id, **kw: {'players': [LOCAL_ACCOUNT_ID]})
        party = SimpleNamespace(_party_of=lambda account_id: None)
        gameplay = GameplayService(self.ctx, self.accounts, self.profiles, None, party, servers, None)
        _, response = gameplay.player(None, Mock(group=lambda *args: ('session', LOCAL_ACCOUNT_ID)))
        self.assertEqual(response['variants']['Character'],
                         profile['items'][locker_id]['attributes']['locker_slots_data']['slots']['Character']['activeVariants'])
        self.assertEqual(response['variants']['Backpack'],
                         profile['items'][locker_id]['attributes']['locker_slots_data']['slots']['Backpack']['activeVariants'])

    def test_invalid_matching_locker_snapshot_rolls_back_auto_equip(self):
        self.login()
        locker_id = self.snapshot()['stats']['attributes']['last_applied_loadout']
        def setup(tx):
            tx.add_item('skye', 'AthenaCharacter:CID_690_Athena_Commando_F_Photographer',
                        {'variants': [{'channel': 'Progressive', 'active': 'Stage1', 'owned': ['Stage1']}]})
            data = copy.deepcopy(tx.profile['items'][locker_id]['attributes']['locker_slots_data'])
            data['slots']['Character'] = {'items': ['skye'], 'activeVariants': [{'variants': 'invalid'}]}
            tx.set_item_attr(locker_id, 'locker_slots_data', data)
            tx.add_item('skye-token', 'CosmeticVariantToken:VTID_502_Photographer_StyleB', {})
        self.edit(setup)
        before = copy.deepcopy(self.snapshot())
        with self.assertRaises(ApiError):
            self.login()
        self.assertEqual(self.snapshot(), before)
        self.restart()
        self.assertEqual(self.snapshot(), before)

    def test_non_auto_equip_does_not_change_locker_snapshot(self):
        self.login()
        self.add_targets()
        locker_id = self.snapshot()['stats']['attributes']['last_applied_loadout']
        def setup(tx):
            data = copy.deepcopy(tx.profile['items'][locker_id]['attributes']['locker_slots_data'])
            data['slots']['Character'] = {'items': ['character'], 'activeVariants': [
                {'variants': [{'channel': 'Progressive', 'active': 'Stage1', 'owned': ['Stage1']}]}]}
            tx.set_item_attr(locker_id, 'locker_slots_data', data)
            tx.add_item('brutus-token', 'CosmeticVariantToken:VTID_506_HenchmanTough_StyleB', {})
        self.edit(setup)
        before = copy.deepcopy(self.snapshot()['items'][locker_id]['attributes']['locker_slots_data'])
        self.edit(self.quests.consume_variants)
        self.assertEqual(self.snapshot()['items'][locker_id]['attributes']['locker_slots_data'], before)
        self.assertEqual(self.snapshot()['items']['character']['attributes']['variants'][0]['owned'], ['Stage1', 'Stage2'])

    def test_invalid_target_rolls_back_completion_and_retry(self):
        side = 'ego'
        quest = self.select(side)
        self.add_targets(malformed=True)
        before = copy.deepcopy(self.snapshot())
        with self.assertRaises(ApiError):
            self.complete(side)
        self.assertEqual(self.snapshot(), before)
        self.assertEqual(self.snapshot()['items'][quest]['attributes']['quest_state'], 'Active')
        self.edit(lambda tx: tx.set_item_attr('glider', 'variants', [
            {'channel': 'Particle', 'active': 'Particle1', 'owned': ['Particle1']}]))
        self.assertTrue(self.complete(side))
        self.assert_variants(side)


if __name__ == '__main__':
    unittest.main()
