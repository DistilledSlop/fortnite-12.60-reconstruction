"""Historical weekly claims repair faction eligibility without replaying rewards."""
if __package__:
    from . import _bootstrap, test_weekly_quests as weekly
else:
    import _bootstrap
    import test_weekly_quests as weekly

import copy
import unittest
from unittest.mock import patch

from api_common import ApiError, LOCAL_ACCOUNT_ID
from profile_service import CommandContext, ProfileService
from quest_service import QuestService
from season12_weekly_quest_data import WEEKLY_BUNDLES, WEEKLY_QUESTS


class FactionReconciliationTests(unittest.TestCase):
    setUp = weekly.WeeklyQuestTests.setUp
    items = weekly.WeeklyQuestTests.items
    get = weekly.WeeklyQuestTests.get
    progress = weekly.WeeklyQuestTests.progress

    def login(self):
        return self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})

    def history(self, week, count, skip=()):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        tx = ctx.tx('athena')
        name = f'MissionBundle_S12_Week_{week:02}'
        bundle = self.quests._bundle(tx, name)
        names = [q for q, *_ in WEEKLY_BUNDLES[name]['quest_infos'] if q not in skip][:count]
        for quest in names:
            item_id = next(i for i, v in tx.profile['items'].items() if v['templateId'].casefold() == ('Quest:' + quest).casefold())
            definition = WEEKLY_QUESTS[quest]
            required = definition.get('ObjectiveCompletionCount', len(definition['objectives']))
            for index, (objective, _, target) in enumerate(definition['objectives']):
                tx.set_item_attr(item_id, 'completion_' + objective.lower(), target if index < required else 0)
            tx.set_item_attr(item_id, 'quest_state', 'Claimed')
        tx.set_item_attr(bundle, 'num_quests_completed', len(names))
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)

    def restart(self):
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        self.quests = self.profiles.quests = QuestService(self.profiles, {weekly.TAG: weekly.START})

    def test_old_profile_both_factions_restart_and_no_rewards_replayed(self):
        for week in (1, 2, 3, 4):
            self.history(week, 9)
        self.login()
        for grant, bundle in (('skulldude', 'SkullDude'), ('tntina', 'TNTina')):
            self.assertEqual(self.get('Quest:quest_AB_' + grant + '_grant')['attributes']['quest_state'], 'Claimed')
            self.assertIsNotNone(self.get('ChallengeBundle:MissionBundle_S12_AB_' + bundle))
        self.assertIsNone(self.get('Token:AthenaBattlePass_TnTinaBundle_Token'))
        before = copy.deepcopy(self.items())
        self.restart(); self.login()
        self.assertEqual(self.items(), before)

    def test_partial_history_then_real_claim_is_counted_once(self):
        self.history(3, 5, skip=('quest_s12_w3_q01',))
        self.login()
        key = 'completion_season12_cumulative_tntina_token'
        self.assertEqual(self.get('Quest:quest_AB_tntina_grant')['attributes'][key], 5)
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        tx = ctx.tx('athena')
        for item_id, item in list(tx.profile['items'].items()):
            if (item['templateId'].casefold().startswith('quest:quest_s12_w3_') and
                    item['templateId'].casefold() != 'quest:quest_s12_w3_q01' and
                    item['attributes'].get('quest_state') == 'Active'):
                tx.set_item_attr(item_id, 'quest_state', 'Inactive')
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.progress('killingblow_apollo_explosives', 'new-real-claim', 5)
        self.assertEqual(self.get('Quest:quest_AB_tntina_grant')['attributes'][key], 6)
        self.login(); self.restart(); self.login()
        self.assertEqual(self.get('Quest:quest_AB_tntina_grant')['attributes'][key], 6)
        self.assertEqual(self.get('Token:AthenaBattlePass_TnTinaBundle_Token')['quantity'], 1)

    def test_threshold_failure_rolls_back_and_retry_succeeds(self):
        self.history(3, 9); self.history(4, 9)
        before = copy.deepcopy(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
        with patch.object(self.quests, '_award_completed', side_effect=ApiError(400, 'failure', 'failure')):
            with self.assertRaises(ApiError):
                self.login()
        self.assertEqual(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'), before)
        self.login()
        self.assertEqual(self.get('Quest:quest_AB_tntina_grant')['attributes']['quest_state'], 'Claimed')

    def test_duplicate_invalid_and_wrong_bundle_claims_fail_closed(self):
        self.history(3, 10); self.history(4, 9)
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        tx = ctx.tx('athena')
        items = tx.profile['items']
        original_id, original = next((i, v) for i, v in items.items() if v['templateId'].casefold() == 'quest:quest_s12_w3_q01')
        tx.add_item('duplicate', original['templateId'], copy.deepcopy(original['attributes']))
        invalid_id, invalid = next((i, v) for i, v in items.items() if v['templateId'].casefold() == 'quest:quest_s12_w3_q02')
        objective = WEEKLY_QUESTS['quest_s12_w3_q02']['objectives'][0][0].lower()
        tx.set_item_attr(invalid_id, 'completion_' + objective, True)
        wrong_id = next(i for i, v in items.items() if v['templateId'].casefold() == 'quest:quest_s12_w3_q03')
        tx.set_item_attr(wrong_id, 'challenge_bundle_id', 'wrong')
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.login()
        grant = self.get('Quest:quest_AB_tntina_grant')
        self.assertEqual(grant['attributes']['quest_state'], 'Active')
        self.assertEqual(grant['attributes']['completion_season12_cumulative_tntina_token'], 16)
        self.assertIsNone(self.get('ChallengeBundle:MissionBundle_S12_AB_TNTina'))


if __name__ == '__main__':
    unittest.main()
