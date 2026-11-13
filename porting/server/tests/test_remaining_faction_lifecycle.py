"""Recovered faction contracts exercised through selection and reward transactions."""
if __package__:
    from . import _bootstrap, test_weekly_quests as weekly, test_faction_entitlement_reconciliation as reconciliation
else:
    import _bootstrap
    import test_weekly_quests as weekly
    import test_faction_entitlement_reconciliation as reconciliation

import copy
import tempfile
import unittest

from account_service import AccountService
from api_common import ApiError, Context, LOCAL_ACCOUNT_ID
from profile_service import CommandContext, ProfileService
from quest_service import QuestService

# Cooked quest and VTID reports in generated/remaining-faction-contracts.
FACTIONS = {
    'meowscles': ('AthenaBattlePass_MeowsclesBundle_Token', 'buffcat', 10, (
        ('AthenaCharacter:CID_693_Athena_Commando_M_BuffCat', 'Progressive', 'Stage', ('VTID_494_BuffCat_StyleB', 'VTID_495_BuffCat_StyleC')),
        ('AthenaBackpack:BID_477_BuffCat', 'Progressive', 'Stage', ('VTID_496_BuffCat_Backbling_StyleB', 'VTID_497_BuffCat_Backbling_StyleC')))),
    'skye': ('AthenaBattlePass_AdventureGirlBundle_Token', 'photographer', 1, (
        ('AthenaCharacter:CID_690_Athena_Commando_F_Photographer', 'Progressive', 'Stage', ('VTID_502_Photographer_StyleB', 'VTID_503_Photographer_StyleC')),
        ('AthenaBackpack:BID_473_Photographer', 'Progressive', 'Stage', ('VTID_504_Photographer_Backbling_StyleB', 'VTID_505_Photographer_Backbling_StyleC')),
        ('AthenaPickaxe:Pickaxe_ID_364_PhotographerFemale1H', 'Particle', 'Particle', ('VTID_542_PhotographerFemalePickaxe_ColorB', 'VTID_543_PhotographerFemalePickaxe_ColorC')),
        ('AthenaGlider:Glider_ID_200_PhotographerFemale', 'Particle', 'Particle', ('VTID_548_PhotographerFemaleGlider_StyleB', 'VTID_549_PhotographerFemaleGlider_StyleC')))),
    'midas': ('AthenaBattlePass_MidasBundle_Token', 'midas', 2, (
        ('AthenaCharacter:CID_694_Athena_Commando_M_CatBurglar', 'Progressive', 'Stage', ('VTID_510_CatBurglar_StyleB', 'VTID_511_CatBurglar_StyleC')),
        ('AthenaBackpack:BID_479_CatBurglar', 'Progressive', 'Stage', ('VTID_512_CatBurglar_Backbling_StyleB', 'VTID_513_CatBurglar_Backbling_StyleC')))),
}


class RemainingFactionLifecycleTests(unittest.TestCase):
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

    def earn(self, faction, quantity=18):
        def earn(tx):
            tx.set_stat('book_purchased', True)
            tx.set_stat('level', 100)
            self.quests._award_token(tx, FACTIONS[faction][0], quantity)
        self.edit(earn)

    def ids(self, faction):
        stem = FACTIONS[faction][1]
        return {side: next((i for i, v in self.snapshot()['items'].items()
                            if v['templateId'].casefold() == f'quest:quest_ab_{stem}_{side}'), None)
                for side in ('ego', 'alter')}

    def select(self, *ids):
        self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ToggleQuestActiveState', {'questIds': list(ids)})

    def states(self, ids):
        return {side: self.snapshot()['items'][item_id]['attributes']['quest_state'] for side, item_id in ids.items()}

    def add_targets(self, faction, indexes):
        targets = FACTIONS[faction][3]
        def add(tx):
            for index in indexes:
                template, channel, prefix, _ = targets[index]
                tx.add_item(f'target-{index}', template, {'item_seen': True, 'variants': [
                    {'channel': channel, 'active': prefix + '1', 'owned': [prefix + '1']}]})
        self.edit(add)

    def test_threshold_selection_switch_and_completion(self):
        for faction, (_, stem, count, targets) in FACTIONS.items():
            for side in ('ego', 'alter'):
                with self.subTest(faction=faction, side=side):
                    self.setUp()
                    self.login()
                    self.earn(faction, 17)
                    self.assertEqual(self.ids(faction), {'ego': None, 'alter': None})
                    self.earn(faction, 1)
                    ids = self.ids(faction)
                    self.assertEqual(self.states(ids), {'ego': 'Inactive', 'alter': 'Inactive'})
                    other = 'alter' if side == 'ego' else 'ego'
                    self.select(ids[other])
                    self.select(ids[side], ids[other])
                    self.assertEqual(self.states(ids), {side: 'Active', other: 'Inactive'})
                    self.add_targets(faction, range(len(targets)))
                    if count > 1:
                        self.assertTrue(self.quests.progress(LOCAL_ACCOUNT_ID, f'ab_{stem}_{side}', count - 1, event_id='partial'))
                        self.assertEqual(self.states(ids)[side], 'Active')
                        for index, (_, channel, prefix, _) in enumerate(targets):
                            self.assertEqual(self.snapshot()['items'][f'target-{index}']['attributes']['variants'],
                                             [{'channel': channel, 'active': prefix + '1', 'owned': [prefix + '1']}])
                    self.assertTrue(self.quests.progress(LOCAL_ACCOUNT_ID, f'ab_{stem}_{side}', 1, event_id='complete'))
                    self.assertEqual(self.states(ids), {side: 'Claimed', other: 'Inactive'})
                    templates = {v['templateId'].casefold() for v in self.snapshot()['items'].values()}
                    for index, (_, channel, prefix, tokens) in enumerate(targets):
                        variants = self.snapshot()['items'][f'target-{index}']['attributes']['variants'][0]
                        self.assertEqual(variants['owned'], [prefix + '1', prefix + ('2' if side == 'ego' else '3')])
                        active = prefix + ('2' if side == 'ego' else '3') if faction == 'skye' else prefix + '1'
                        self.assertEqual(variants['active'], active)
                        self.assertFalse(self.snapshot()['items'][f'target-{index}']['attributes']['item_seen'])
                        self.assertNotIn(('CosmeticVariantToken:' + tokens[side == 'alter']).casefold(), templates)
                    variants_before_restart = [copy.deepcopy(self.snapshot()['items'][f'target-{i}']['attributes']['variants'])
                                               for i in range(len(targets))]
                    # The first full login after this fixture's manual level
                    # change also grants Midas's independently earned gold style.
                    if faction == 'midas':
                        for variants in variants_before_restart:
                            variants[0]['owned'].append('Stage4')
                    self.restart(); self.login()
                    self.assertEqual([self.snapshot()['items'][f'target-{i}']['attributes']['variants']
                                      for i in range(len(targets))], variants_before_restart)
                    self.assertEqual(self.states(ids), {side: 'Claimed', other: 'Inactive'})
                    with self.assertRaises(ApiError):
                        self.select(ids[side], ids[other])

    def test_cross_faction_selection_rejected_atomically(self):
        self.login()
        self.earn('meowscles'); self.earn('midas')
        before = copy.deepcopy(self.snapshot())
        with self.assertRaises(ApiError):
            self.select(self.ids('meowscles')['ego'], self.ids('midas')['alter'])
        self.assertEqual(self.snapshot(), before)

    def test_missing_target_token_waits_for_later_acquisition(self):
        for faction, (_, stem, count, targets) in FACTIONS.items():
            with self.subTest(faction=faction):
                self.setUp(); self.login(); self.earn(faction)
                ids = self.ids(faction); self.select(ids['ego'])
                self.add_targets(faction, range(len(targets) - 1))
                self.assertTrue(self.quests.progress(LOCAL_ACCOUNT_ID, f'ab_{stem}_ego', count, event_id='complete'))
                missing = ('CosmeticVariantToken:' + targets[-1][3][0]).casefold()
                self.assertIn(missing, {v['templateId'].casefold() for v in self.snapshot()['items'].values()})
                self.restart(); self.add_targets(faction, [len(targets) - 1]); self.login()
                self.assertNotIn(missing, {v['templateId'].casefold() for v in self.snapshot()['items'].values()})
                _, channel, prefix, _ = targets[-1]
                variants = self.snapshot()['items'][f'target-{len(targets)-1}']['attributes']['variants'][0]
                expected = [prefix + '1', prefix + '2']
                if faction == 'midas':
                    expected.append('Stage4')
                self.assertEqual(variants['owned'], expected)


class RemainingFactionHistoryTests(unittest.TestCase):
    setUp = weekly.WeeklyQuestTests.setUp
    items = weekly.WeeklyQuestTests.items
    get = weekly.WeeklyQuestTests.get
    history = reconciliation.FactionReconciliationTests.history

    def test_seventeen_real_claims_and_one_new_weekly_claim_unlock(self):
        for first, second, final, event, amount, grant, bundle in (
            (5, 6, 'quest_s12_w5_q01', 'interact_athena_chests_salty_or_mistymeadows', 10, 'meowscles', 'Meowscle'),
            (7, 8, 'quest_s12_w7_q01', 'interact_treasurechest_spybases', 7, 'adventuregirl', 'AdventureGirl'),
            (9, 10, 'quest_s12_w9_q02', 'damage_athena_player_sniper', 300, 'midas', 'Midas'),
        ):
            with self.subTest(grant=grant):
                self.setUp()
                self.now = weekly.START + 100 * 86400
                self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})
                self.history(first, 9, skip=(final,))
                self.history(second, 8)
                self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})
                item = self.get('Quest:quest_AB_' + grant + '_grant')
                self.assertEqual(item['attributes']['quest_state'], 'Active')
                self.assertIsNone(self.get('ChallengeBundle:MissionBundle_S12_AB_' + bundle))
                ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
                tx = ctx.tx('athena')
                for item_id, quest in list(tx.profile['items'].items()):
                    if (quest['templateId'].casefold().startswith('quest:quest_s12_w') and
                            quest['templateId'].casefold() != ('Quest:' + final).casefold() and
                            quest['attributes'].get('quest_state') == 'Active'):
                        tx.set_item_attr(item_id, 'quest_state', 'Inactive')
                self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
                self.assertTrue(self.quests.progress(LOCAL_ACCOUNT_ID, event, amount, event_id='last-claim'))
                self.assertEqual(self.get('Quest:' + final)['attributes']['quest_state'], 'Claimed')
                self.assertEqual(self.get('Quest:quest_AB_' + grant + '_grant')['attributes']['quest_state'], 'Claimed')
                self.assertIsNotNone(self.get('ChallengeBundle:MissionBundle_S12_AB_' + bundle))
                self.restart_history()
                self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})
                self.assertEqual(self.get('Quest:quest_AB_' + grant + '_grant')['attributes']['quest_state'], 'Claimed')

    def restart_history(self):
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        self.quests = self.profiles.quests = QuestService(self.profiles, {weekly.TAG: weekly.START})


if __name__ == '__main__':
    unittest.main()
