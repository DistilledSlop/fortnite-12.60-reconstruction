"""Fixed quest reward claims validate server state and commit rewards once."""
if __package__:
    from . import _bootstrap
else:
    import _bootstrap

import copy
import tempfile
import unittest
from unittest.mock import patch

from backend import Backend
from api_common import ApiError, LOCAL_ACCOUNT_ID
from profile_service import ProfileTx
from season12_daily_quest_data import DAILY_QUESTS
from quest_service import EVENT_QUESTS
from season12_progression_data import XP_TO_NEXT


class QuestClaimTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.restart()
        self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})
        self.quest_id, self.quest = next((i, v) for i, v in self.snapshot()['items'].items()
                                        if v['templateId'].startswith('Quest:Quest_S12_Daily_'))

    def restart(self):
        self.profiles = Backend(state_dir=self.directory.name).profiles

    def snapshot(self):
        return self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')

    def complete(self, *, progress=True):
        tx = ProfileTx(self.profiles.ctx, self.snapshot())
        tx.set_item_attr(self.quest_id, 'quest_state', 'Completed')
        if progress:
            for objective, _, target in DAILY_QUESTS[self.quest['templateId'][6:]]['objectives']:
                tx.set_item_attr(self.quest_id, 'completion_' + objective.lower(), target)
        self.profiles._store(LOCAL_ACCOUNT_ID, {'athena': tx})

    def claim(self, **body):
        return self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClaimQuestReward',
                                 body or {'questId': self.quest_id, 'selectedRewardIndex': -1})

    def test_completed_claim_rewards_once_across_restart(self):
        self.complete()
        response = self.claim()
        notification, = [n for n in response['notifications'] if n.get('primary')]
        self.assertEqual(notification['questId'], self.quest_id)
        self.assertNotIn('type', notification)
        self.assertEqual(notification['loot']['tierGroupName'], '')
        xp, = [v for v in notification['loot']['items']
               if v['itemType'] == 'AccountResource:athenaseasonalxp']
        self.assertEqual(xp['quantity'], 31000)
        self.assertEqual(xp['itemGuid'], '')
        after = self.snapshot()
        self.assertEqual(after['items'][self.quest_id]['attributes']['quest_state'], 'Claimed')
        tokens = [v for v in after['items'].values()
                  if v['templateId'].casefold() == 'token:athena_dailychallenge_token']
        self.assertEqual([v['quantity'] for v in tokens], [1])
        self.assertGreater(after['stats']['attributes']['level'], 1)
        self.assertEqual(self.claim()['notifications'], [])
        self.restart()
        self.assertEqual(self.claim()['notifications'], [])
        again = self.snapshot()
        # MCP command revisions can advance on a no-op; rewards and state cannot.
        self.assertEqual(after['items'], again['items'])
        self.assertEqual(after['stats'], again['stats'])
        self.assertEqual(after['rvn'], again['rvn'])

    def test_active_incomplete_unknown_and_invalid_index_reject(self):
        before = self.snapshot()
        for body in ({'questId': self.quest_id, 'selectedRewardIndex': -1},
                     {'questId': self.quest_id},
                     {'questId': self.quest_id, 'selectedRewardIndex': 0},
                     {'questId': self.quest_id, 'selectedRewardIndex': True},
                     {'questId': 'unowned', 'selectedRewardIndex': -1}):
            with self.subTest(body=body), self.assertRaises(ApiError):
                self.claim(**body)
            self.assertEqual(before, self.snapshot())
        self.complete(progress=False)
        before = self.snapshot()
        with self.assertRaises(ApiError):
            self.claim()
        self.assertEqual(before, self.snapshot())

    def test_award_failure_rolls_back_then_retry_succeeds(self):
        self.complete()
        before = self.snapshot()
        before_core = self.profiles.existing_profile(LOCAL_ACCOUNT_ID, 'common_core')
        with patch('progression_service.award_profile_xp', side_effect=RuntimeError('award failure')):
            with self.assertRaises(RuntimeError):
                self.claim()
        self.assertEqual(before, self.snapshot())
        self.assertEqual(before_core, self.profiles.existing_profile(LOCAL_ACCOUNT_ID, 'common_core'))
        self.restart()
        self.assertEqual(before, self.snapshot())
        self.claim()
        self.assertEqual(self.snapshot()['items'][self.quest_id]['attributes']['quest_state'], 'Claimed')

    def test_stacked_reward_reports_increment_and_actual_attributes(self):
        tx = ProfileTx(self.profiles.ctx, self.snapshot())
        tx.add_item('existing-token', 'Token:athena_dailychallenge_token', {'item_seen': True}, 7)
        self.profiles._store(LOCAL_ACCOUNT_ID, {'athena': tx})
        self.complete()
        response = self.claim()
        reward, = [v for n in response['notifications'] if n.get('primary')
                   for v in n['loot']['items'] if v['itemGuid'] == 'existing-token']
        self.assertEqual(reward['quantity'], 1)
        self.assertEqual(reward['attributes'], {'item_seen': True})
        self.assertEqual(reward['itemProfile'], 'athena')
        self.assertEqual(self.snapshot()['items']['existing-token']['quantity'], 8)

    def test_crossed_battle_pass_currency_is_in_sole_primary(self):
        tx = ProfileTx(self.profiles.ctx, self.snapshot())
        for key, value in {'level': 4, 'book_level': 4, 'xp': XP_TO_NEXT[3] - 1,
                           'book_xp': XP_TO_NEXT[3] - 1}.items():
            tx.set_stat(key, value)
        self.profiles._store(LOCAL_ACCOUNT_ID, {'athena': tx})
        self.complete()
        response = self.claim()
        primary, = [n for n in response['notifications'] if n.get('primary')]
        currency, = [v for v in primary['loot']['items'] if v['itemType'] == 'Currency:MtxGiveaway']
        self.assertEqual(currency['quantity'], 100)
        self.assertEqual(currency['itemProfile'], 'common_core')
        core = self.profiles.profile(LOCAL_ACCOUNT_ID, 'common_core')
        self.assertEqual(currency['attributes'], core['items'][currency['itemGuid']]['attributes'])

    def test_converted_variant_reports_target_not_consumed_token(self):
        # Exercise the real token conversion with an isolated fixed-reward fixture.
        tx = ProfileTx(self.profiles.ctx, self.snapshot())
        tx.add_item('peely', 'AthenaCharacter:CID_701_Athena_Commando_M_BananaAgent',
                    {'variants': [{'channel': 'Progressive', 'active': 'Stage1', 'owned': ['Stage1']}]})
        self.profiles._store(LOCAL_ACCOUNT_ID, {'athena': tx})
        self.complete()
        name = self.quest['templateId'][6:]
        definition = copy.deepcopy(EVENT_QUESTS[name])
        definition['rewards'] = [('CosmeticVariantToken', 'VTID_560_BananaAgent_StyleB', 1)]
        with patch.dict(EVENT_QUESTS, {name: definition}):
            response = self.claim()
        primary, = [n for n in response['notifications'] if n.get('primary')]
        reward, = primary['loot']['items']
        self.assertEqual(reward['itemGuid'], 'peely')
        self.assertEqual(reward['quantity'], 1)
        self.assertIn('Stage2', reward['attributes']['variants'][0]['owned'])
        self.assertFalse(any(v['templateId'].startswith('CosmeticVariantToken:')
                             for v in self.snapshot()['items'].values()))

    def test_old_pending_banner_conversion_is_not_attributed_to_claim(self):
        tx = ProfileTx(self.profiles.ctx, self.snapshot())
        tx.add_item('old-banner', 'BannerToken:BannerToken_043_S12_Donut', {'item_seen': False})
        self.profiles._store(LOCAL_ACCOUNT_ID, {'athena': tx})
        self.complete()
        primary, = [n for n in self.claim()['notifications'] if n.get('primary')]
        self.assertFalse(any(v['itemType'].startswith('HomebaseBannerIcon:')
                             for v in primary['loot']['items']))
        core = self.profiles.profile(LOCAL_ACCOUNT_ID, 'common_core')
        self.assertTrue(any(v['templateId'] == 'HomebaseBannerIcon:brs12_donut'
                            for v in core['items'].values()))

    def test_new_banner_reward_reports_final_common_core_icon(self):
        self.complete()
        name = self.quest['templateId'][6:]
        definition = copy.deepcopy(EVENT_QUESTS[name])
        definition['rewards'] = [('BannerToken', 'BannerToken_043_S12_Donut', 1)]
        with patch.dict(EVENT_QUESTS, {name: definition}):
            response = self.claim()
        primary, = [n for n in response['notifications'] if n.get('primary')]
        reward, = primary['loot']['items']
        self.assertEqual(reward['itemType'], 'HomebaseBannerIcon:brs12_donut')
        self.assertEqual(reward['itemProfile'], 'common_core')
        core = self.profiles.profile(LOCAL_ACCOUNT_ID, 'common_core')
        self.assertEqual(reward['attributes'], core['items'][reward['itemGuid']]['attributes'])
        self.assertFalse(any(v['templateId'].startswith('BannerToken:')
                             for v in self.snapshot()['items'].values()))

    def test_preexisting_variant_token_is_not_reported_as_claim_reward(self):
        tx = ProfileTx(self.profiles.ctx, self.snapshot())
        tx.add_item('peely', 'AthenaCharacter:CID_701_Athena_Commando_M_BananaAgent',
                    {'variants': [{'channel': 'Progressive', 'active': 'Stage1', 'owned': ['Stage1']}]})
        tx.add_item('old-style', 'CosmeticVariantToken:VTID_560_BananaAgent_StyleB', {'item_seen': False})
        self.profiles._store(LOCAL_ACCOUNT_ID, {'athena': tx})
        self.complete()
        primary, = [n for n in self.claim()['notifications'] if n.get('primary')]
        self.assertFalse(any(v['itemGuid'] == 'peely' for v in primary['loot']['items']))
        self.assertNotIn('old-style', self.snapshot()['items'])

    def test_crossed_cosmetic_tier_has_only_one_primary(self):
        tx = ProfileTx(self.profiles.ctx, self.snapshot())
        for key in ('xp', 'book_xp'):
            tx.set_stat(key, XP_TO_NEXT[0] - 1)
        self.profiles._store(LOCAL_ACCOUNT_ID, {'athena': tx})
        self.complete()
        response = self.claim()
        primary, = [n for n in response['notifications'] if n.get('primary')]
        self.assertEqual(primary['questId'], self.quest_id)
        self.assertTrue(any(v['itemType'].startswith('Athena') for v in primary['loot']['items']))
        self.assertFalse(any(n.get('type') == 'CatalogPurchase'
                             for n in response['notifications']))

    def test_direct_banner_icon_reward_is_included_without_conversion_token(self):
        self.complete()
        name = self.quest['templateId'][6:]
        definition = copy.deepcopy(EVENT_QUESTS[name])
        definition['rewards'] = [('HomebaseBannerIcon', 'brs12_donut', 1)]
        with patch.dict(EVENT_QUESTS, {name: definition}):
            response = self.claim()
        primary, = [n for n in response['notifications'] if n.get('primary')]
        reward, = primary['loot']['items']
        self.assertEqual(reward['itemType'], 'HomebaseBannerIcon:brs12_donut')
        self.assertEqual(reward['itemProfile'], 'athena')

    def test_fresh_duplicate_style_does_not_claim_stale_other_style_unlock(self):
        tx = ProfileTx(self.profiles.ctx, self.snapshot())
        tx.add_item('peely', 'AthenaCharacter:CID_701_Athena_Commando_M_BananaAgent',
                    {'variants': [{'channel': 'Progressive', 'active': 'Stage1',
                                   'owned': ['Stage1', 'Stage2']}]})
        tx.add_item('old-style', 'CosmeticVariantToken:VTID_561_BananaAgent_StyleC',
                    {'item_seen': False})
        self.profiles._store(LOCAL_ACCOUNT_ID, {'athena': tx})
        self.complete()
        name = self.quest['templateId'][6:]
        definition = copy.deepcopy(EVENT_QUESTS[name])
        definition['rewards'] = [('CosmeticVariantToken', 'VTID_560_BananaAgent_StyleB', 1)]
        with patch.dict(EVENT_QUESTS, {name: definition}):
            response = self.claim()
        primary, = [n for n in response['notifications'] if n.get('primary')]
        self.assertEqual(primary['loot']['items'], [])
        peely = self.snapshot()['items']['peely']
        self.assertIn('Stage3', peely['attributes']['variants'][0]['owned'])
        self.assertNotIn('old-style', self.snapshot()['items'])
