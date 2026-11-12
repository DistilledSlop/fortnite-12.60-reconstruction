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
from api_common import Context, LOCAL_ACCOUNT_ID
from profile_service import CommandContext, ProfileService
from quest_service import QuestService
from season12_location_data import LOCATION_BUNDLES, LOCATION_QUESTS
from season12_weekly_quest_data import WEEKLY_QUESTS
from season12_progression_data import XP_TO_NEXT

START = 1700000000
TAG = 'EventFlag.LobbySeason12'


class LocationQuestTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.now = START
        self.clock = Context(clock=lambda: self.now)
        self.accounts = AccountService(self.clock, state_dir=self.tmp.name)
        self.profiles = ProfileService(self.clock, self.accounts, None, state_dir=self.tmp.name)
        self.service = self.profiles.quests = QuestService(self.profiles, {TAG: START})

    def items(self):
        return self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')['items']

    def get(self, template):
        return next((v for v in self.items().values() if v['templateId'].casefold() == template.casefold()), None)

    def command(self, fn):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        fn(ctx.tx('athena'))
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)

    def login(self):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        self.service.login(ctx, include_daily=False)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)

    def test_schedule_dates(self):
        self.assertEqual(len(WEEKLY_QUESTS), 100)
        self.assertEqual(len(LOCATION_QUESTS), 60)
        self.command(lambda tx: self.service._grant(tx, 'ChallengeBundleSchedule:Season12_Mission_Schedule', {}))
        names = list(LOCATION_BUNDLES)
        self.now = START + 73 * 86400 - 1
        self.login()
        self.assertIsNone(self.get('ChallengeBundle:' + names[0]))
        self.now += 1; self.login()
        self.assertIsNotNone(self.get('ChallengeBundle:' + names[0]))
        self.assertIsNone(self.get('ChallengeBundle:' + names[1]))
        self.now = START + 87 * 86400
        self.service.event_starts.clear(); self.login()
        self.assertIsNone(self.get('ChallengeBundle:' + names[1]))
        self.service.event_starts[TAG] = START; self.login()
        self.assertIsNotNone(self.get('ChallengeBundle:' + names[1]))

    def test_all_cooked_chain_rewards(self):
        for bundle in LOCATION_BUNDLES.values():
            self.assertEqual(len(bundle['quest_infos']), 10)
            self.assertEqual(set(bundle['completion_rewards']), set(range(1, 11)))
            for root, _, _, _ in bundle['quest_infos']:
                current = root
                for xp in (10000, 25000, 55000):
                    rewards = LOCATION_QUESTS[current]['rewards']
                    self.assertIn(('AccountResource', 'athenaseasonalxp', xp), rewards)
                    following = [reward for kind, reward, _ in rewards if kind == 'Quest']
                    if xp == 55000:
                        self.assertEqual(following, [])
                        self.assertIn(('Token', 'athenatoken_feat_locationdomination', 1), rewards)
                    else:
                        self.assertEqual(len(following), 1)
                        current = following[0]

    def test_chains_rewards_repair_and_atomic_retry(self):
        name = next(iter(LOCATION_BUNDLES))
        self.command(lambda tx: self.service._bundle(tx, name))
        root = LOCATION_BUNDLES[name]['quest_infos'][0][0]
        _, event, target = LOCATION_QUESTS[root]['objectives'][0]
        before = copy.deepcopy(self.items())
        with patch.object(self.service, '_award_token', side_effect=RuntimeError('injected')):
            with self.assertRaises(RuntimeError):
                self.service.progress(LOCAL_ACCOUNT_ID, event, target, event_id='atomic')
        self.assertEqual(self.items(), before)
        self.assertTrue(self.service.progress(LOCAL_ACCOUNT_ID, event, target, event_id='atomic'))
        self.assertFalse(self.service.progress(LOCAL_ACCOUNT_ID, event, target, event_id='atomic'))
        next_name = next(r for k, r, _ in LOCATION_QUESTS[root]['rewards'] if k == 'Quest')
        next_item = self.get('Quest:' + next_name)
        self.assertEqual(next_item['attributes']['completion_' + LOCATION_QUESTS[next_name]['objectives'][0][0].lower()], 0)
        bundle = self.get('ChallengeBundle:' + name)
        self.assertEqual(bundle['attributes']['num_quests_completed'], 1)
        self.assertEqual(bundle['attributes']['num_progress_quests_completed'], 0)
        for index in range(100):
            active = [v for v in self.items().values() if v['templateId'].casefold().startswith('quest:quest_s12_locationdomination_w1_') and v['attributes']['quest_state'] == 'Active']
            if not active:
                break
            quest_name = active[0]['templateId'].split(':', 1)[1]
            _, stat, amount = LOCATION_QUESTS[quest_name]['objectives'][0]
            self.service.progress(LOCAL_ACCOUNT_ID, stat, amount, event_id=f'finish-{index}')
        self.assertFalse(active)
        bundle = self.get('ChallengeBundle:' + name)
        self.assertEqual(bundle['attributes']['num_quests_completed'], 30)
        self.assertEqual(bundle['attributes']['num_progress_quests_completed'], 10)
        self.assertEqual(self.get('Token:athenatoken_feat_locationdomination')['quantity'], 10)
        self.assertEqual(self.get('Token:AthenaBattlePass_WeeklyChallenge_Token')['quantity'], 10)
        self.assertEqual(self.get('Token:AthenaBattlePass_WeeklyBundle_Token')['quantity'], 1)
        stats = self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')['stats']['attributes']
        self.assertEqual(sum(XP_TO_NEXT[:stats['level'] - 1]) + stats['xp'], 900000)
        bundle_id = next(i for i, v in self.items().items() if v['templateId'].casefold() == ('ChallengeBundle:' + name).casefold())
        self.command(lambda tx: tx.set_item_attr(bundle_id, 'num_progress_quests_completed', 0))
        self.service = self.profiles.quests = QuestService(self.profiles, {TAG: START})
        self.login(); self.login()
        self.assertEqual(self.get('ChallengeBundle:' + name)['attributes']['num_progress_quests_completed'], 10)
        self.assertEqual(self.get('Token:AthenaBattlePass_WeeklyChallenge_Token')['quantity'], 10)
        self.assertEqual(self.get('Token:athenatoken_feat_locationdomination')['quantity'], 10)

    def test_login_repairs_missing_successor_without_replaying_rewards(self):
        name, future = list(LOCATION_BUNDLES)
        self.command(lambda tx: self.service._bundle(tx, name))
        root = LOCATION_BUNDLES[name]['quest_infos'][0][0]
        _, event, target = LOCATION_QUESTS[root]['objectives'][0]
        self.service.progress(LOCAL_ACCOUNT_ID, event, target, event_id='repair-root')
        successor = next(reward for kind, reward, _ in LOCATION_QUESTS[root]['rewards'] if kind == 'Quest')
        successor_id = next(i for i, v in self.items().items()
                            if v['templateId'].casefold() == ('Quest:' + successor).casefold())
        self.command(lambda tx: tx.remove_item(successor_id))
        before_stats = copy.deepcopy(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')['stats'])
        before_bundle = copy.deepcopy(self.get('ChallengeBundle:' + name))
        before_token = copy.deepcopy(self.get('Token:AthenaBattlePass_WeeklyChallenge_Token'))

        self.login()
        repaired = self.get('Quest:' + successor)
        self.assertIsNotNone(repaired)
        self.assertEqual(repaired['attributes']['quest_state'], 'Active')
        self.assertEqual(repaired['attributes']['completion_' + LOCATION_QUESTS[successor]['objectives'][0][0]], 0)
        self.assertIsNone(self.get('ChallengeBundle:' + future))
        after_first = copy.deepcopy(self.items())
        self.login()
        self.assertEqual(self.items(), after_first)
        self.assertEqual(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')['stats'], before_stats)
        self.assertEqual(self.get('ChallengeBundle:' + name), before_bundle)
        self.assertEqual(self.get('Token:AthenaBattlePass_WeeklyChallenge_Token'), before_token)


if __name__ == '__main__':
    unittest.main()
