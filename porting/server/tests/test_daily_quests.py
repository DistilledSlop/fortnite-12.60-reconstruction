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
from daily_quest_service import DailyPolicy, DailyService
from profile_service import CommandContext, ProfileService
from season12_daily_quest_data import DAILY_QUESTS, daily_quest_name


class DailyQuestTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.time = 1700000000
        self.ctx = Context(clock=lambda: self.time)
        self.accounts = AccountService(self.ctx, state_dir=self.tmp.name)
        self.restart()

    def restart(self, policy=None):
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        self.daily = DailyService(self.profiles, policy)

    def execute(self, method, *args):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        result = getattr(self.daily, method)(ctx, *args)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        return result

    def profile(self):
        return self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')

    def active(self):
        return [(k, v) for k, v in self.profile()['items'].items() if daily_quest_name(v['templateId'])
                and v['attributes']['quest_state'] == 'Active']

    def test_definitions(self):
        self.assertEqual(len(DAILY_QUESTS), 62)
        self.assertEqual(sum(len(d['objectives']) for d in DAILY_QUESTS.values()), 70)
        self.assertEqual(sum(d.get('Weight') == 0 for d in DAILY_QUESTS.values()), 3)
        special = [d for d in DAILY_QUESTS.values() if d.get('ObjectiveCompletionCount') == 5]
        self.assertEqual(len(special), 1)
        self.assertGreaterEqual(len(special[0]['objectives']), 5)
        self.assertTrue(all(len(d['objectives']) == len(d['stat_conditions']) for d in DAILY_QUESTS.values()))

    def test_login_cap_restart_and_no_catchup(self):
        self.execute('login'); first = self.active()
        self.assertEqual(len(first), 1)
        self.execute('login'); self.assertEqual(first, self.active())
        self.restart(); self.execute('login'); self.assertEqual(first, self.active())
        self.time += 86400 * 5
        self.execute('login'); self.assertEqual(len(self.active()), 2)
        self.time += 86400; self.execute('login'); self.assertEqual(len(self.active()), 3)
        self.time += 86400; self.execute('login'); self.assertEqual(len(self.active()), 3)

    def test_recurrence_and_reroll(self):
        names = tuple(name for name, d in DAILY_QUESTS.items() if d.get('Weight', 1) > 0)[:2]
        self.restart(DailyPolicy(candidates=names))
        self.execute('login'); item_id, item = self.active()[0]
        new_id = self.execute('reroll', item_id)
        self.assertNotEqual(item_id, new_id)
        self.assertNotEqual(item['templateId'], self.active()[0][1]['templateId'])
        before = copy.deepcopy(self.profile())
        with self.assertRaises(ApiError): self.execute('reroll', new_id)
        self.assertEqual(before, self.profile())
        self.time += 86400
        self.execute('reroll', new_id)
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        tx = ctx.tx('athena'); current_id, current = self.active()[0]
        tx.set_item_attr(current_id, 'quest_state', 'Claimed')
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.execute('login')
        self.assertEqual(len(self.active()), 1)
        self.assertNotEqual(self.active()[0][0], current_id)

    def test_claimed_template_can_recur_as_new_instance(self):
        name = next(name for name, d in DAILY_QUESTS.items() if d.get('Weight', 1) > 0)
        self.restart(DailyPolicy(candidates=(name,)))
        self.execute('login')
        first_id, first = self.active()[0]
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        ctx.tx('athena').set_item_attr(first_id, 'quest_state', 'Claimed')
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.time += 86400
        self.execute('login')
        next_id, next_item = self.active()[0]
        self.assertNotEqual(first_id, next_id)
        self.assertEqual(first['templateId'], next_item['templateId'])

    def test_invalid_and_atomic(self):
        self.execute('login'); item_id = self.active()[0][0]
        before = copy.deepcopy(self.profile())
        for bad in ('unknown', '', None, 'Quest:' + next(iter(DAILY_QUESTS))):
            with self.assertRaises(ApiError): self.execute('reroll', bad)
            self.assertEqual(before, self.profile())
        self.daily = DailyService(self.profiles, DailyPolicy(candidates=(daily_quest_name(self.active()[0][1]['templateId']),)))
        with self.assertRaises(ApiError): self.execute('reroll', item_id)
        self.assertEqual(before, self.profile())

    def test_utc_boundary_and_determinism(self):
        self.time = 172799
        self.execute('login'); self.assertEqual(len(self.active()), 1)
        self.time = 172800
        self.execute('login'); self.assertEqual(len(self.active()), 2)
        self.time = 172799
        self.execute('login'); self.assertEqual(len(self.active()), 2)
        service = self.daily
        self.assertEqual(service._choose('account', '2020-01-01', 'grant', set()),
                         service._choose('account', '2020-01-01', 'grant', set()))
