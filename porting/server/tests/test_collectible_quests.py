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

from account_service import AccountService
from api_common import ApiError, Context, LOCAL_ACCOUNT_ID
from profile_service import CommandContext, ProfileService
from quest_service import QuestService
from season12_collectible_data import COIN_BUNDLES, COIN_QUESTS, COIN_SCHEDULE, FISH_QUESTS

START = 1700000000
TAG = 'EventFlag.LobbySeason12'


class CollectibleQuestTests(unittest.TestCase):
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

    def xp_state(self):
        stats = self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')['stats']['attributes']
        return (stats['level'], stats['xp'], stats.get('book_level'), stats.get('book_xp'))

    def login(self):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        self.service.login(ctx, include_daily=False)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)

    def test_season_grants_and_schedule_gate(self):
        self.command(lambda tx: tx.set_stat('season_num', 11))
        self.login()
        for name in FISH_QUESTS:
            self.assertIsNone(self.get('Quest:' + name))
        self.command(lambda tx: tx.set_stat('season_num', 12))
        self.login()
        for name in FISH_QUESTS:
            item = self.get('Quest:' + name)
            self.assertIsNotNone(item)
            self.assertFalse(item['attributes'].get('challenge_bundle_id'))
        names = [entry[0] for entry in COIN_SCHEDULE['entries']]
        # Remove the entitlement explicitly to test the schedule ownership gate.
        schedule = 'ChallengeBundleSchedule:' + COIN_SCHEDULE['name']
        owned = next((i for i, v in self.items().items() if v['templateId'].casefold() == schedule.casefold()), None)
        if owned:
            self.command(lambda tx: tx.remove_item(owned))
        self.assertIsNone(self.get('ChallengeBundle:' + names[0]))
        self.command(lambda tx: self.service._grant(tx, 'ChallengeBundleSchedule:' + COIN_SCHEDULE['name'], {}))
        self.login()
        self.assertIsNotNone(self.get('ChallengeBundle:' + names[0]))
        self.now = START + 18 * 86400 - 1
        self.login()
        for name in names[1:3]: self.assertIsNone(self.get('ChallengeBundle:' + name))
        self.now += 1
        self.login()
        for name in names[1:3]: self.assertIsNotNone(self.get('ChallengeBundle:' + name))
        self.assertIsNone(self.get('ChallengeBundle:' + names[3]))

    def test_client_login_grants_for_free_and_paid_pass(self):
        for purchased in (False, True):
            with self.subTest(purchased=purchased), tempfile.TemporaryDirectory() as state_dir:
                accounts = AccountService(self.clock, state_dir=state_dir)
                profiles = ProfileService(self.clock, accounts, None, state_dir=state_dir)
                profiles.quests = QuestService(profiles, {TAG: START})
                ctx = CommandContext(profiles, LOCAL_ACCOUNT_ID, 'athena', {})
                ctx.tx('athena').set_stat('book_purchased', purchased)
                profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
                profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})
                items = profiles.profile(LOCAL_ACCOUNT_ID, 'athena')['items'].values()
                templates = {item['templateId'].casefold() for item in items}
                self.assertIn(('ChallengeBundleSchedule:' + COIN_SCHEDULE['name']).casefold(), templates)
                self.assertIn(('ChallengeBundle:' + COIN_SCHEDULE['entries'][0][0]).casefold(), templates)
                for name in FISH_QUESTS:
                    self.assertIn('quest:' + name, templates)

    def test_distinct_objectives_completion_retry_restart_and_rollback(self):
        name = COIN_SCHEDULE['entries'][0][0]
        self.command(lambda tx: self.service._bundle(tx, name))
        quest = COIN_BUNDLES[name]['quest_infos'][0][0]
        definition = COIN_QUESTS[quest]
        objectives = definition['objectives']
        self.assertGreater(len(objectives), 1)
        item = self.get('Quest:' + quest)
        first, event, amount = objectives[0]
        other = next(q for q in COIN_QUESTS if q != quest and COIN_QUESTS[q]['objectives'][0][1] != event)
        self.command(lambda tx: self.service._quest(tx, other, ''))
        before_stats = self.xp_state()
        self.assertTrue(self.service.progress(LOCAL_ACCOUNT_ID, event, amount, event_id='coin-1'))
        self.assertFalse(self.service.progress(LOCAL_ACCOUNT_ID, event, amount, event_id='coin-1'))
        item = self.get('Quest:' + quest)
        self.assertEqual(item['attributes']['completion_' + first], amount)
        self.assertEqual(item['attributes']['quest_state'], 'Active')
        other_item = self.get('Quest:' + other)
        self.assertEqual(other_item['attributes']['completion_' + COIN_QUESTS[other]['objectives'][0][0]], 0)
        self.accounts = AccountService(self.clock, state_dir=self.tmp.name)
        self.profiles = ProfileService(self.clock, self.accounts, None, state_dir=self.tmp.name)
        self.service = self.profiles.quests = QuestService(self.profiles, {TAG: START})
        self.assertFalse(self.service.progress(LOCAL_ACCOUNT_ID, event, amount, event_id='coin-1'))
        item_id = next(i for i, v in self.items().items() if v['templateId'].casefold() == ('quest:' + quest).casefold())
        second = objectives[1]
        self.command(lambda tx: tx.set_item_attr(item_id, 'completion_' + second[0], 'bad'))
        before = copy.deepcopy(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
        with self.assertRaises(ApiError):
            self.service.progress(LOCAL_ACCOUNT_ID, second[1], second[2], event_id='coin-bad')
        self.assertEqual(before, self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
        self.command(lambda tx: tx.set_item_attr(item_id, 'completion_' + second[0], 0))
        for index, (_, stat, count) in enumerate(objectives[1:]):
            self.service.progress(LOCAL_ACCOUNT_ID, stat, count, event_id='coin-bad' if index == 0 else f'coin-rest-{index}')
        self.assertEqual(self.get('Quest:' + quest)['attributes']['quest_state'], 'Claimed')
        self.assertEqual(before_stats, self.xp_state())
        bundle = self.get('ChallengeBundle:' + name)['attributes']
        self.assertEqual(bundle['num_quests_completed'], 1)
        self.assertEqual(bundle['num_progress_quests_completed'], 1)

    def test_fish_completion_has_no_loot(self):
        self.login()
        name, definition = next(iter(FISH_QUESTS.items()))
        before = self.xp_state()
        _, event, amount = definition['objectives'][0]
        self.service.progress(LOCAL_ACCOUNT_ID, event, amount, event_id='fish')
        self.assertEqual(self.get('Quest:' + name)['attributes']['quest_state'], 'Claimed')
        self.assertEqual(before, self.xp_state())


if __name__ == '__main__':
    unittest.main()
