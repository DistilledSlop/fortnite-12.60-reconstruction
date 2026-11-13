"""TNTina's cooked acquire threshold and weekly reward transaction."""
if __package__:
    from . import _bootstrap, test_weekly_quests as weekly
else:
    import _bootstrap
    import test_weekly_quests as weekly

import copy
import unittest
from unittest.mock import patch
from api_common import ApiError
from profile_service import CommandContext, ProfileService
from quest_service import QuestService
from season12_tntina_faction_data import TOKEN
from season12_weekly_quest_data import WEEKLY_QUESTS


class TNTinaEntitlementTests(unittest.TestCase):
    setUp = weekly.WeeklyQuestTests.setUp
    items = weekly.WeeklyQuestTests.items
    get = weekly.WeeklyQuestTests.get
    progress = weekly.WeeklyQuestTests.progress

    def login(self):
        self.profiles.run(weekly.LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})

    def test_weekly_threshold_rollback_and_restart(self):
        self.now = weekly.START + 17 * 86400
        self.login()
        grant = self.get('Quest:quest_AB_tntina_grant')
        self.assertEqual(grant['attributes']['quest_state'], 'Active')
        self.assertIsNone(self.get('ChallengeBundle:MissionBundle_S12_AB_TNTina'))
        ctx = CommandContext(self.profiles, weekly.LOCAL_ACCOUNT_ID, 'athena', {})
        self.quests._award_token(ctx.tx('athena'), TOKEN, 17)
        self.profiles._store(weekly.LOCAL_ACCOUNT_ID, ctx.txs)
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        self.quests = self.profiles.quests = QuestService(self.profiles, {weekly.TAG: weekly.START})
        objectives = WEEKLY_QUESTS['quest_s12_w3_q05']['objectives']
        for index, (_, event, target) in enumerate(objectives[:14]):
            self.progress(event, f'tnt-{index}', target)
        _, event, target = objectives[14]
        before = copy.deepcopy(self.profiles.profile(weekly.LOCAL_ACCOUNT_ID, 'athena'))
        with patch.object(self.quests, '_bundle', side_effect=ApiError(400, 'failure', 'failure')):
            with self.assertRaises(ApiError):
                self.progress(event, 'threshold', target)
        self.assertEqual(self.profiles.profile(weekly.LOCAL_ACCOUNT_ID, 'athena'), before)
        self.assertTrue(self.progress(event, 'threshold', target))
        self.assertFalse(self.progress(event, 'threshold', target))
        self.assertEqual(self.get('Token:' + TOKEN)['quantity'], 18)
        self.assertEqual(self.get('Quest:quest_AB_tntina_grant')['attributes']['quest_state'], 'Claimed')
        bundle = self.get('ChallengeBundle:MissionBundle_S12_AB_TNTina')
        self.assertIsNotNone(bundle)
        for side in ('alter', 'ego'):
            self.assertEqual(self.get('Quest:AB_tntina_' + side)['attributes']['quest_state'], 'Inactive')
        before = copy.deepcopy(self.items())
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        self.quests = self.profiles.quests = QuestService(self.profiles, {weekly.TAG: weekly.START})
        self.login()
        self.assertEqual(self.items(), before)

    def test_existing_balance_is_not_acquisition(self):
        ctx = CommandContext(self.profiles, weekly.LOCAL_ACCOUNT_ID, 'athena', {})
        ctx.tx('athena').add_item('old-token', 'Token:' + TOKEN, {}, 18)
        self.profiles._store(weekly.LOCAL_ACCOUNT_ID, ctx.txs)
        self.login()
        self.assertEqual(self.get('Quest:quest_AB_tntina_grant')['attributes']['completion_season12_cumulative_tntina_token'], 0)
        self.assertIsNone(self.get('ChallengeBundle:MissionBundle_S12_AB_TNTina'))
        with self.assertRaises(ApiError):
            self.progress('item_acquire_tntina_token', 'forged', 18)


if __name__ == '__main__':
    unittest.main()
