if __package__:
    from . import _bootstrap
else:
    import sys
    from pathlib import Path
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    import _bootstrap

import copy
import tempfile
import unittest

from account_service import AccountService
from api_common import ApiError, Context, LOCAL_ACCOUNT_ID
from profile_service import CommandContext, ProfileService
from quest_service import QuestService, VALID_EVENTS
from season12_peely_data import QUESTS, SCHEDULE, CLIENT_OBJECTIVES

START = 1700000000

class PeelyClientQuestTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.now = START
        self.clock = Context(clock=lambda: self.now)
        self.restart()

    def restart(self):
        self.accounts = AccountService(self.clock, state_dir=self.tmp.name)
        self.profiles = ProfileService(self.clock, self.accounts, None, state_dir=self.tmp.name)
        self.profiles.quests = QuestService(self.profiles, {'EventFlag.LobbySeason12': START})

    def run_command(self, name, body):
        return self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', name, body)

    def items(self):
        return self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')['items']

    def find(self, template):
        return next((v for v in self.items().values() if v['templateId'].casefold() == template.casefold()), None)

    def entry(self, stat, count=1, offset=0):
        return {'statName': stat, 'count': count, 'timestampOffset': offset}

    def test_free_login_gate_boundary_and_restart_replay(self):
        self.run_command('ClientQuestLogin', {})
        self.assertIsNotNone(self.find('ChallengeBundleSchedule:' + SCHEDULE['name']))
        bundle = self.find('ChallengeBundle:QuestBundle_Event_S12_PeelySpy')
        self.assertIsNotNone(bundle)
        stats = list(CLIENT_OBJECTIVES)
        self.run_command('UpdateQuestClientObjectives', {'advance': [self.entry(stats[0])]})
        self.assertTrue(all(self.find('Quest:' + name) is None for name in QUESTS))
        self.now = START + 43 * 86400 - 1
        self.run_command('ClientQuestLogin', {})
        self.assertTrue(all(self.find('Quest:' + name) is None for name in QUESTS))
        self.now += 1
        self.run_command('ClientQuestLogin', {})
        self.assertEqual(len(self.find('ChallengeBundle:QuestBundle_Event_S12_PeelySpy')['attributes']['GrantedQuestInstanceIds']), 3)
        payload = {'advance': [self.entry(stats[0]), self.entry(stats[0]), self.entry(stats[1], 99, -2), self.entry(stats[2])]}
        self.run_command('UpdateQuestClientObjectives', payload)
        self.assertTrue(all(self.find('Quest:' + name)['attributes']['quest_state'] == 'Claimed' for name in QUESTS))
        attrs = self.find('ChallengeBundle:QuestBundle_Event_S12_PeelySpy')['attributes']
        self.assertEqual((attrs['num_quests_completed'], attrs['num_progress_quests_completed']), (3, 3))
        self.assertIsNotNone(self.find('AthenaDance:Emoji_S12_BananaAgent'))
        self.restart()
        self.run_command('UpdateQuestClientObjectives', payload)
        self.run_command('ClientQuestLogin', {})
        self.assertEqual(sum(v['templateId'].casefold() == 'athenadance:emoji_s12_bananaagent' for v in self.items().values()), 1)
        self.assertEqual(self.find('ChallengeBundle:QuestBundle_Event_S12_PeelySpy')['attributes']['num_quests_completed'], 3)

    def test_invalid_batch_rolls_back_and_rejects_spoof(self):
        self.now = START + 43 * 86400
        self.run_command('ClientQuestLogin', {})
        stat = next(iter(CLIENT_OBJECTIVES))
        before = copy.deepcopy(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
        for invalid in [self.entry('peely_spy_frontend_01'), self.entry(next(iter(VALID_EVENTS))), self.entry(stat, True), self.entry(stat, 0), self.entry(stat, 2**31), self.entry(stat, 1, True), self.entry(stat, 1, 2**31), {'statName': stat, 'count': 1, 'timestampOffset': 0, 'actualAdvance': 1}]:
            with self.subTest(invalid=invalid), self.assertRaises(ApiError):
                self.run_command('UpdateQuestClientObjectives', {'advance': [self.entry(stat), invalid]})
            self.assertEqual(before, self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
        self.assertNotIn('objective:' + stat, VALID_EVENTS)

    def test_paid_pass_missing_epoch_and_removed_schedule(self):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        ctx.tx('athena').set_stat('book_purchased', True)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.now = START + 43 * 86400
        self.profiles.quests = QuestService(self.profiles, {})
        self.run_command('ClientQuestLogin', {})
        self.assertIsNotNone(self.find('ChallengeBundle:QuestBundle_Event_S12_PeelySpy'))
        self.assertTrue(all(self.find('Quest:' + name) is None for name in QUESTS))
        schedule_id = next(i for i, item in self.items().items() if item['templateId'].casefold() ==
                           ('challengebundleschedule:' + SCHEDULE['name']).casefold())
        bundle_id = next(i for i, item in self.items().items() if item['templateId'].casefold() ==
                         'challengebundle:questbundle_event_s12_peelyspy')
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        ctx.tx('athena').remove_item(schedule_id)
        ctx.tx('athena').remove_item(bundle_id)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.profiles.quests = QuestService(self.profiles, {'EventFlag.LobbySeason12': START})
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        self.profiles.quests.login(ctx, include_daily=False)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.assertIsNone(self.find('ChallengeBundle:QuestBundle_Event_S12_PeelySpy'))
        self.run_command('ClientQuestLogin', {})
        self.assertTrue(all(self.find('Quest:' + name) is not None for name in QUESTS))

    def test_bounds_and_corrupt_bundle_roll_back(self):
        self.now = START + 43 * 86400
        self.run_command('ClientQuestLogin', {})
        stats = list(CLIENT_OBJECTIVES)
        before = copy.deepcopy(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
        for batch in [[self.entry(stats[0])] * 101, [self.entry(stats[0], -1)],
                      [self.entry(stats[0], 1, -2147483649)]]:
            with self.assertRaises(ApiError):
                self.run_command('UpdateQuestClientObjectives', {'advance': batch})
            self.assertEqual(before, self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
        bundle_id = next(i for i, item in self.items().items() if item['templateId'].casefold() ==
                         'challengebundle:questbundle_event_s12_peelyspy')
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        ctx.tx('athena').set_item_attr(bundle_id, 'num_quests_completed', True)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        before = copy.deepcopy(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
        with self.assertRaises(ApiError):
            self.run_command('UpdateQuestClientObjectives', {'advance': [self.entry(stats[0]), self.entry(stats[1])]})
        self.assertEqual(before, self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))

    def test_inactive_quest_is_ignored(self):
        self.now = START + 43 * 86400
        self.run_command('ClientQuestLogin', {})
        stat, name = next(iter(CLIENT_OBJECTIVES.items()))
        item_id = next(i for i, v in self.items().items() if v['templateId'].casefold() == 'quest:' + name)
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        ctx.tx('athena').set_item_attr(item_id, 'quest_state', 'Inactive')
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.run_command('UpdateQuestClientObjectives', {'advance': [self.entry(stat)]})
        self.assertEqual(self.find('Quest:' + name)['attributes']['completion_' + stat], 0)

if __name__ == '__main__':
    unittest.main()
