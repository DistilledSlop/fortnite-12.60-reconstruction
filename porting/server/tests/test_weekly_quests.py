if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import copy
import tempfile
import unittest
from unittest.mock import patch

from account_service import AccountService
from api_common import ApiError, Context, LOCAL_ACCOUNT_ID
from profile_service import CommandContext, ProfileService
from quest_service import QuestService
from season12_weekly_quest_data import WEEKLY_QUESTS

START = 1700000000
TAG = 'EventFlag.LobbySeason12'


class WeeklyQuestTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.now = START
        self.ctx = Context(clock=lambda: self.now)
        self.accounts = AccountService(self.ctx, state_dir=self.tmp.name)
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        self.quests = self.profiles.quests = QuestService(self.profiles, {TAG: START})

    def items(self):
        return self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')['items']

    def get(self, template):
        return next((v for v in self.items().values() if v['templateId'].casefold() == template.casefold()), None)

    def login(self):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        self.quests.login(ctx, include_daily=False)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)

    def grant_schedule(self):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        self.quests._grant(ctx.tx('athena'), 'ChallengeBundleSchedule:Season12_Mission_Schedule', {})
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)

    def progress(self, event, eid, amount=1, match_id=None):
        return self.quests.progress(LOCAL_ACCOUNT_ID, event, amount, event_id=eid, match_id=match_id)

    def quest(self, name):
        return self.get('Quest:' + name)['attributes']

    def test_owned_schedule_and_dates(self):
        self.assertEqual(len(WEEKLY_QUESTS), 100)
        self.login()
        self.assertIsNone(self.get('Quest:quest_s12_w1_q01'))
        self.grant_schedule(); self.login()
        self.assertIsNotNone(self.get('Quest:quest_s12_w1_q01'))
        self.assertIsNone(self.get('Quest:quest_s12_w2_q01'))
        self.now = START + 10 * 86400 - 1; self.login()
        self.assertIsNone(self.get('Quest:quest_s12_w2_q01'))
        self.quests.event_starts.clear()
        self.now += 1; self.login()
        self.assertIsNone(self.get('Quest:quest_s12_w2_q01'))
        self.quests.event_starts[TAG] = START; self.login()
        self.assertIsNotNone(self.get('Quest:quest_s12_w2_q01'))
        self.assertIsNone(self.get('Quest:quest_AB_skulldude_ego'))

    def test_tntina_schedule_subset_and_token_family(self):
        self.grant_schedule(); self.login()
        self.now = START + 17 * 86400 - 1; self.login()
        self.assertIsNone(self.get('Quest:quest_s12_w3_q05'))
        self.now += 1; self.login()
        self.assertIsNotNone(self.get('Quest:quest_s12_w3_q05'))
        self.assertIsNone(self.get('Quest:quest_s12_w4_q01'))
        objectives = WEEKLY_QUESTS['quest_s12_w3_q05']['objectives']
        for index, (_, event, target) in enumerate(objectives[:14]):
            self.progress(event, f'landmark-{index}', target)
        self.assertNotEqual(self.quest('quest_s12_w3_q05')['quest_state'], 'Claimed')
        _, event, target = objectives[14]
        self.progress(event, 'landmark-14', target)
        self.assertEqual(self.quest('quest_s12_w3_q05')['quest_state'], 'Claimed')
        self.assertEqual(self.get('Token:AthenaBattlePass_TnTinaBundle_Token')['quantity'], 1)
        self.assertEqual(self.get('Token:AthenaBattlePass_WeeklyChallenge_Token')['quantity'], 1)
        self.assertIsNone(self.get('Token:AthenaBattlePass_SkullDudeBundle_Token'))
        self.now = START + 24 * 86400 - 1; self.login()
        self.assertIsNone(self.get('Quest:quest_s12_w4_q01'))
        self.now += 1; self.login()
        self.assertIsNotNone(self.get('Quest:quest_s12_w4_q01'))

    def test_match_rules_and_receipts(self):
        self.grant_schedule(); self.login()
        event = 'open_faction_locked_doors'; key = 'completion_quest_s12_w1_q03_obj'
        self.progress(event, 'no-context', 3)
        self.assertFalse(self.progress(event, 'no-context', 3, 'legacy-match'))
        self.assertFalse(self.quests.replay_progress(LOCAL_ACCOUNT_ID, event, 3, event_id='no-context', match_id='legacy-match'))
        self.assertEqual(self.quest('quest_s12_w1_q03')[key], 0)
        self.progress(event, 'a', 9, 'm1'); self.progress(event, 'b', 1, 'm1')
        self.assertEqual(self.quest('quest_s12_w1_q03')[key], 1)
        self.quests = self.profiles.quests = QuestService(self.profiles, {TAG: START})
        self.assertFalse(self.progress(event, 'a', 9, 'm1'))
        self.assertFalse(self.quests.replay_progress(LOCAL_ACCOUNT_ID, event, 9, event_id='a', match_id='m1'))
        with self.assertRaises(ApiError): self.progress(event, 'a', 9, 'm2')
        self.progress(event, 'c', 1, 'm2'); self.progress(event, 'd', 1, 'm3')
        self.assertEqual(self.quest('quest_s12_w1_q03')['quest_state'], 'Claimed')
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        self.quests._quest(ctx.tx('athena'), 'Quest_S12_Daily_Search_AmmoBoxes', None)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        ammo = 'interact_athena_ammobox'; key = 'completion_quest_s12_w1_q04_obj'
        self.progress(ammo, 'no-ammo-context', 2)
        self.assertEqual(self.quest('quest_s12_w1_q04')[key], 0)
        self.assertEqual(self.quest('Quest_S12_Daily_Search_AmmoBoxes')['completion_quest_s12_daily_search_ammoboxes'], 2)
        self.progress(ammo, 'e', 4, 'm1'); self.progress(ammo, 'f', 2, 'm2')
        self.progress(ammo, 'g', 5, 'm1')
        self.assertEqual(self.quest('quest_s12_w1_q04')[key], 2)
        self.progress(ammo, 'h', 5, 'm2')
        self.assertEqual(self.quest('quest_s12_w1_q04')['quest_state'], 'Claimed')

    def test_week_one_tenth_completion_reward_and_restart(self):
        self.grant_schedule(); self.login()
        last_event = None
        for number in range(1, 11):
            definition = WEEKLY_QUESTS[f'quest_s12_w1_q{number:02}']
            objectives = definition['objectives'][:definition.get('ObjectiveCompletionCount', len(definition['objectives']))]
            for objective, event, target in objectives:
                if definition.get('bAthenaUpdateObjectiveOncePerMatch'):
                    for match in range(target):
                        self.progress(event, f'{objective}-{match}', match_id=f'match-{match}')
                    last_event = (event, f'{objective}-{target - 1}', 1, f'match-{target - 1}')
                else:
                    match = 'single-match' if definition.get('bAthenaMustCompleteInSingleMatch') else None
                    self.progress(event, objective, target, match)
                    last_event = (event, objective, target, match)
            if number == 9:
                self.assertIsNone(self.get('Token:AthenaBattlePass_WeeklyBundle_Token'))
        self.assertEqual(self.get('Token:AthenaBattlePass_WeeklyChallenge_Token')['quantity'], 10)
        self.assertEqual(self.get('Token:AthenaBattlePass_SkullDudeBundle_Token')['quantity'], 10)
        self.assertEqual(self.get('Token:AthenaBattlePass_WeeklyBundle_Token')['quantity'], 1)
        before = copy.deepcopy(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        self.quests = self.profiles.quests = QuestService(self.profiles, {TAG: START})
        event, eid, amount, match = last_event
        self.assertFalse(self.progress(event, eid, amount, match))
        self.assertEqual(before, self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))

    def test_malformed_match_state_is_rejected_atomically(self):
        self.grant_schedule(); self.login()
        for name, state, event in (
            ('quest_s12_w1_q03', {'objective_matches': {'obj': 'bad'}}, 'open_faction_locked_doors'),
            ('quest_s12_w1_q04', {'seen_matches': None}, 'interact_athena_ammobox'),
        ):
            ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
            tx = ctx.tx('athena')
            item_id = next(i for i, item in tx.profile['items'].items()
                           if item['templateId'].casefold() == ('Quest:' + name).casefold())
            tx.set_item_attr(item_id, 'local_quest_match_state', state)
            self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
            before = copy.deepcopy(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
            with self.assertRaises(ApiError): self.progress(event, name, match_id='match')
            self.assertEqual(before, self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))

    def test_rewards_stacking_and_atomic_failure(self):
        self.grant_schedule(); self.login()
        from progression_service import award_profile_xp
        with patch('progression_service.award_profile_xp', wraps=award_profile_xp) as award:
            self.progress('damage_athena_henchmen', 'one', 2000)
            self.assertEqual(award.call_args.args[1:], (40000, 40000))
        self.progress('interact_athena_chests_mountainbase_or_sharkisland', 'two', 7)
        token = self.get('Token:AthenaBattlePass_WeeklyChallenge_Token')
        self.assertEqual(token['quantity'], 2)
        self.assertEqual(self.get('Token:AthenaBattlePass_SkullDudeBundle_Token')['quantity'], 2)
        before = copy.deepcopy(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
        with patch.object(self.quests, '_award_token', side_effect=ApiError(400, 'failure', 'failure')):
            with self.assertRaises(ApiError): self.progress('crouch_near_henchmen', 'three', 10)
        self.assertEqual(before, self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
        self.progress('crouch_near_henchmen', 'three', 10)
        self.assertEqual(self.get('Token:AthenaBattlePass_WeeklyChallenge_Token')['quantity'], 3)


if __name__ == '__main__': unittest.main()
