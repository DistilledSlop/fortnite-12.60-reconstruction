"""Free entitlement and atomic item-acquisition threshold."""
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
from season12_brutus_faction_data import TOKEN


class BrutusEntitlementTests(unittest.TestCase):
    setUp = weekly.WeeklyQuestTests.setUp
    items = weekly.WeeklyQuestTests.items
    get = weekly.WeeklyQuestTests.get
    progress = weekly.WeeklyQuestTests.progress

    def login(self):
        self.profiles.run(weekly.LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})

    def test_threshold_rollback_retry_and_restart(self):
        self.login()
        schedule = self.get('ChallengeBundleSchedule:Season12_ABChallenges_Schedule')
        grant = self.get('Quest:quest_AB_skulldude_grant')
        self.assertIsNotNone(schedule)
        self.assertEqual(grant['attributes']['quest_state'], 'Active')
        self.assertIsNone(self.get('ChallengeBundle:MissionBundle_S12_AB_Skulldude'))
        # Exercise the same transaction path as quest reward issuance. A balance
        # changed independently does not count as an acquisition.
        ctx = CommandContext(self.profiles, weekly.LOCAL_ACCOUNT_ID, 'athena', {})
        tx = ctx.tx('athena')
        self.quests._award_token(tx, TOKEN, 17)
        self.profiles._store(weekly.LOCAL_ACCOUNT_ID, ctx.txs)
        self.assertEqual(self.get('Quest:quest_AB_skulldude_grant')['attributes']['completion_season12_cumulative_skulldude_token'], 17)
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        self.quests = self.profiles.quests = QuestService(self.profiles, {weekly.TAG: weekly.START})
        before = copy.deepcopy(self.profiles.profile(weekly.LOCAL_ACCOUNT_ID, 'athena'))
        with patch.object(self.quests, '_bundle', side_effect=ApiError(400, 'failure', 'failure')):
            with self.assertRaises(ApiError):
                self.progress('damage_athena_henchmen', 'threshold', 2000)
        self.assertEqual(self.profiles.profile(weekly.LOCAL_ACCOUNT_ID, 'athena'), before)
        self.assertTrue(self.progress('damage_athena_henchmen', 'threshold', 2000))
        self.assertFalse(self.progress('damage_athena_henchmen', 'threshold', 2000))
        self.assertEqual(self.get('Token:' + TOKEN)['quantity'], 18)
        self.assertEqual(self.get('Quest:quest_AB_skulldude_grant')['attributes']['quest_state'], 'Claimed')
        self.assertIsNotNone(self.get('ChallengeBundle:MissionBundle_S12_AB_Skulldude'))
        for side in ('alter', 'ego'):
            self.assertEqual(self.get('Quest:quest_AB_skulldude_' + side)['attributes']['quest_state'], 'Inactive')
        before = copy.deepcopy(self.items())
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        self.quests = self.profiles.quests = QuestService(self.profiles, {weekly.TAG: weekly.START})
        self.login()
        self.assertEqual(self.items(), before)

    def test_existing_token_balance_is_not_acquisition(self):
        ctx = CommandContext(self.profiles, weekly.LOCAL_ACCOUNT_ID, 'athena', {})
        ctx.tx('athena').add_item('old-token', 'Token:' + TOKEN, {}, 18)
        self.profiles._store(weekly.LOCAL_ACCOUNT_ID, ctx.txs)
        self.login()
        self.assertEqual(self.get('Quest:quest_AB_skulldude_grant')['attributes']['completion_season12_cumulative_skulldude_token'], 0)
        self.assertIsNone(self.get('ChallengeBundle:MissionBundle_S12_AB_Skulldude'))


if __name__ == '__main__':
    unittest.main()
