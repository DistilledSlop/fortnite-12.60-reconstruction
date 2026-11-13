if __package__:
    from . import test_peely_client_quests as base
else:
    import test_peely_client_quests as base

import copy
import unittest
from api_common import ApiError, LOCAL_ACCOUNT_ID
from profile_service import CommandContext
from quest_service import QuestService, VALID_EVENTS
from season12_yacht_data import SCHEDULE, BUNDLE

START = base.START
DAY = 86400
QUEST = 'Quest:Quest_S12_Donut_YachtChallenge'
BUNDLE_ID = 'ChallengeBundle:' + BUNDLE
SCHEDULE_ID = 'ChallengeBundleSchedule:' + SCHEDULE['name']
EVENT = 'visit_namedpoi_spybase_yacht'
REWARD = 'AthenaMusicPack:MusicPack_047_FreestylinClubRemix'

class YachtQuestTests(unittest.TestCase):
    setUp = base.PeelyClientQuestTests.setUp
    restart = base.PeelyClientQuestTests.restart
    run_command = base.PeelyClientQuestTests.run_command
    items = base.PeelyClientQuestTests.items
    find = base.PeelyClientQuestTests.find
    entry = base.PeelyClientQuestTests.entry

    def test_schedule_gate_trusted_progress_and_restart(self):
        self.run_command('ClientQuestLogin', {})
        self.assertIsNotNone(self.find(SCHEDULE_ID))
        self.assertIsNotNone(self.find(BUNDLE_ID))
        self.assertIsNone(self.find(QUEST))
        self.assertIn(EVENT, VALID_EVENTS)
        self.profiles.quests.progress(LOCAL_ACCOUNT_ID, EVENT, event_id='early')
        self.now = START + 46 * DAY - 1
        self.run_command('ClientQuestLogin', {})
        self.assertIsNone(self.find(QUEST))
        self.now += 1
        self.run_command('ClientQuestLogin', {})
        quest = self.find(QUEST)
        self.assertEqual(quest['attributes']['quest_state'], 'Active')
        bundle = self.find(BUNDLE_ID)
        self.assertEqual(len(bundle['attributes']['GrantedQuestInstanceIds']), 1)
        self.assertFalse(self.profiles.quests.progress(LOCAL_ACCOUNT_ID, EVENT, event_id='early'))
        self.assertEqual(quest['attributes']['completion_quest_s12_donut_yachtchallenge'], 0)
        self.assertTrue(self.profiles.quests.progress(LOCAL_ACCOUNT_ID, EVENT, event_id='visit'))
        self.assertIsNotNone(self.find(REWARD))
        attrs = self.find(BUNDLE_ID)['attributes']
        self.assertEqual((attrs['num_quests_completed'], attrs['num_progress_quests_completed']), (1, 1))
        self.restart()
        self.run_command('ClientQuestLogin', {})
        self.assertFalse(self.profiles.quests.progress(LOCAL_ACCOUNT_ID, EVENT, event_id='visit'))
        self.assertEqual(sum(v['templateId'].casefold() == REWARD.casefold() for v in self.items().values()), 1)

    def test_paid_missing_calendar_and_spoof(self):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        ctx.tx('athena').set_stat('book_purchased', True)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.now = START + 46 * DAY
        self.profiles.quests = QuestService(self.profiles, {})
        self.run_command('ClientQuestLogin', {})
        self.assertIsNotNone(self.find(BUNDLE_ID))
        self.assertIsNone(self.find(QUEST))
        with self.assertRaises(ApiError):
            self.run_command('UpdateQuestClientObjectives', {'advance': [self.entry(EVENT)]})
        self.profiles.quests = QuestService(self.profiles, {'EventFlag.LobbySeason12': START})
        self.run_command('ClientQuestLogin', {})
        self.assertIsNotNone(self.find(QUEST))

    def test_season_and_owned_schedule_boundaries(self):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        ctx.tx('athena').set_stat('season_num', 11)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.run_command('ClientQuestLogin', {})
        self.assertIsNone(self.find(SCHEDULE_ID))
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        ctx.tx('athena').set_stat('season_num', 12)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        # Quest reconciliation alone does not invent schedule ownership.
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        self.profiles.quests.login(ctx, include_daily=False)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.assertIsNone(self.find(BUNDLE_ID))
        self.run_command('ClientQuestLogin', {})
        self.assertIsNotNone(self.find(SCHEDULE_ID))
        self.assertIsNotNone(self.find(BUNDLE_ID))

    def test_corrupt_counter_rolls_back(self):
        self.now = START + 46 * DAY
        self.run_command('ClientQuestLogin', {})
        bundle_id = next(i for i, v in self.items().items() if v['templateId'].casefold() == BUNDLE_ID.casefold())
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        ctx.tx('athena').set_item_attr(bundle_id, 'num_quests_completed', True)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        before = copy.deepcopy(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
        with self.assertRaises(ApiError):
            self.profiles.quests.progress(LOCAL_ACCOUNT_ID, EVENT, event_id='invalid')
        self.assertEqual(before, self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
