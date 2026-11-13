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
from quest_service import QuestService
from season12_location_data import LOCATION_BUNDLES, LOCATION_QUESTS
from season12_storm_agency_data import STORM_AGENCY_BUNDLES

START = 1700000000
TAG = 'EventFlag.LTE_StormTheAgency'


class BundleMembershipTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.clock = Context(clock=lambda: START)
        self.restart()

    def restart(self):
        self.accounts = AccountService(self.clock, state_dir=self.tmp.name)
        self.profiles = ProfileService(self.clock, self.accounts, None, state_dir=self.tmp.name)
        self.service = self.profiles.quests = QuestService(self.profiles, {TAG: START})

    def profile(self):
        return self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')

    def find(self, template):
        return next((i for i, v in self.profile()['items'].items()
                     if v['templateId'].casefold() == template.casefold()), None)

    def command(self, fn):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        fn(ctx.tx('athena'))
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)

    def login(self):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        self.service.login(ctx, include_daily=False)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)

    def members(self, bundle):
        return self.profile()['items'][bundle]['attributes']['GrantedQuestInstanceIds']

    def test_legacy_repair_and_restart(self):
        self.login()
        name = next(iter(STORM_AGENCY_BUNDLES))
        bundle = self.find('ChallengeBundle:' + name)
        roots = [self.find('Quest:' + q) for q, *_ in STORM_AGENCY_BUNDLES[name]['quest_infos']]
        self.assertEqual(self.members(bundle), roots)
        self.assertEqual(len(roots), 5)
        self.assertIsNone(self.find('Quest:quest_s12_sta_findfrontend'))
        other = next(iter(LOCATION_BUNDLES))
        self.command(lambda tx: self.service._bundle(tx, other))
        other_id = self.find('ChallengeBundle:' + other)
        foreign = self.members(other_id)[0]
        self.command(lambda tx: tx.set_item_attr(bundle, 'GrantedQuestInstanceIds', [foreign, roots[0], roots[0], 'missing']))
        self.login()
        self.assertEqual(self.members(bundle), roots)
        before = copy.deepcopy(self.profile()['items'])
        self.login()
        self.restart()
        self.login()
        self.assertEqual(self.profile()['items'], before)

    def test_successor_is_member_in_progress_transaction_and_repair_no_reward(self):
        name = next(iter(LOCATION_BUNDLES))
        self.command(lambda tx: self.service._bundle(tx, name))
        bundle = self.find('ChallengeBundle:' + name)
        root = LOCATION_BUNDLES[name]['quest_infos'][0][0]
        _, event, target = LOCATION_QUESTS[root]['objectives'][0]
        self.assertTrue(self.service.progress(LOCAL_ACCOUNT_ID, event, target, event_id='membership'))
        successor = next(r for k, r, _ in LOCATION_QUESTS[root]['rewards'] if k == 'Quest')
        root_id = self.find('Quest:' + root)
        successor_id = self.find('Quest:' + successor)
        self.assertIn(root_id, self.members(bundle))
        self.assertIn(successor_id, self.members(bundle))
        self.command(lambda tx: tx.set_item_attr(bundle, 'GrantedQuestInstanceIds', []))
        stats = copy.deepcopy(self.profile()['stats'])
        items = copy.deepcopy(self.profile()['items'])
        self.login()
        self.assertIn(successor_id, self.members(bundle))
        self.assertEqual(self.profile()['stats'], stats)
        for item_id in items:
            if item_id != bundle:
                self.assertEqual(self.profile()['items'][item_id], items[item_id])
        self.assertFalse(self.service.progress(LOCAL_ACCOUNT_ID, event, target, event_id='membership'))

    def test_malformed_membership_rolls_back_progress_and_disk(self):
        name = next(iter(LOCATION_BUNDLES))
        self.command(lambda tx: self.service._bundle(tx, name))
        bundle = self.find('ChallengeBundle:' + name)
        root = LOCATION_BUNDLES[name]['quest_infos'][0][0]
        _, event, target = LOCATION_QUESTS[root]['objectives'][0]
        for invalid in ('bad', ['valid', 3]):
            self.command(lambda tx: tx.set_item_attr(bundle, 'GrantedQuestInstanceIds', invalid))
            before = copy.deepcopy(self.profile())
            with self.assertRaises(ApiError):
                self.service.progress(LOCAL_ACCOUNT_ID, event, target, event_id='rollback')
            self.assertEqual(self.profile(), before)
            self.restart()
            self.assertEqual(self.profile(), before)
        self.command(lambda tx: tx.set_item_attr(bundle, 'GrantedQuestInstanceIds', []))
        self.assertTrue(self.service.progress(LOCAL_ACCOUNT_ID, event, target, event_id='rollback'))


if __name__ == '__main__':
    unittest.main()
