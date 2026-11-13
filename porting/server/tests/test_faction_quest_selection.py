"""Native Brutus selection request shapes and local validation boundaries."""
if __package__:
    from . import _bootstrap
else:
    import _bootstrap

import tempfile
import unittest

from account_service import AccountService
from api_common import ApiError, Context, LOCAL_ACCOUNT_ID
from profile_service import CommandContext, ProfileService
from quest_service import QuestService
from season12_brutus_faction_data import TOKEN


class FactionSelectionTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.now = 1700000000
        self.ctx = Context(clock=lambda: self.now)
        self.accounts = AccountService(self.ctx, state_dir=self.tmp.name)
        self.restart()

    def restart(self):
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        self.quests = self.profiles.quests = QuestService(self.profiles)

    def snapshot(self):
        return self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')

    def earn(self):
        self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        ctx.tx('athena').set_stat('book_purchased', True)
        ctx.tx('athena').set_stat('level', 100)
        self.quests._award_token(ctx.tx('athena'), TOKEN, 18)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.ids = {side: next(item_id for item_id, item in self.snapshot()['items'].items()
                              if item['templateId'].casefold() == 'quest:quest_ab_skulldude_' + side)
                    for side in ('ego', 'alter')}

    def select(self, ids, *, key='questIds'):
        return self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ToggleQuestActiveState', {key: ids})

    def states(self):
        return {side: self.snapshot()['items'][item_id]['attributes']['quest_state']
                for side, item_id in self.ids.items()}

    def reject_unchanged(self, ids, **kwargs):
        before = self.snapshot()
        with self.assertRaises(ApiError):
            self.select(ids, **kwargs)
        self.assertEqual(before, self.snapshot())

    def test_initial_selection_switch_and_restart(self):
        self.earn()
        before = self.snapshot()
        self.now += 1
        response = self.select([self.ids['alter']])
        self.assertEqual(self.states(), {'ego': 'Inactive', 'alter': 'Active'})
        self.assertEqual(response['profileRevision'], before['rvn'] + 1)
        self.assertEqual({change['attributeName'] for change in response['profileChanges']},
                         {'quest_state', 'last_state_change_time'})
        # Actual EGO native caller sends Ego first, then the active ALTER.
        self.select([self.ids['ego'], self.ids['alter']])
        self.assertEqual(self.states(), {'ego': 'Active', 'alter': 'Inactive'})
        self.restart()
        self.assertEqual(self.states(), {'ego': 'Active', 'alter': 'Inactive'})
        self.select([self.ids['ego'], self.ids['alter']])
        self.assertEqual(self.states(), {'ego': 'Inactive', 'alter': 'Active'})

    def test_unearned_and_invalid_batch_are_atomic(self):
        self.reject_unchanged(['unowned'])
        self.earn()
        for ids in ([], 'not-an-array', [None], [[]], [True], [self.ids['ego']] * 2,
                    [self.ids['ego'], 'unowned'], ['Quest:quest_AB_skulldude_ego'],
                    list(self.ids.values())):
            with self.subTest(ids=ids):
                self.reject_unchanged(ids)
        self.reject_unchanged([self.ids['ego']], key='QuestIds')
        self.select([self.ids['ego']])
        # A lone inactive sibling would activate both; a lone active ID would
        # clear the choice. Neither is a supported selection request.
        self.reject_unchanged([self.ids['alter']])
        self.reject_unchanged([self.ids['ego']])

    def test_completed_branch_and_wrong_bundle_are_rejected(self):
        self.earn()
        self.select([self.ids['ego']])
        for state in ('Completed', 'Claimed'):
            ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
            ctx.tx('athena').set_item_attr(self.ids['ego'], 'quest_state', state)
            self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
            self.reject_unchanged([self.ids['alter']])
            self.reject_unchanged([self.ids['ego'], self.ids['alter']])
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        tx = ctx.tx('athena')
        tx.set_item_attr(self.ids['ego'], 'quest_state', 'Inactive')
        tx.set_item_attr(self.ids['alter'], 'challenge_bundle_id', 'other-bundle')
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.reject_unchanged([self.ids['ego']])


if __name__ == '__main__':
    unittest.main()
