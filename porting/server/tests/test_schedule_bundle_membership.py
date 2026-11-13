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
from season12_location_data import LOCATION_BUNDLES


class ScheduleMembershipTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.clock = Context(clock=lambda: 1700000000)
        self.restart()

    def restart(self):
        self.accounts = AccountService(self.clock, state_dir=self.tmp.name)
        self.profiles = ProfileService(self.clock, self.accounts, None, state_dir=self.tmp.name)
        self.service = self.profiles.quests = QuestService(self.profiles)

    def profile(self):
        return self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')

    def command(self, fn):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        result = fn(ctx.tx('athena'))
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        return result

    def login(self):
        return self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})

    def test_grants_repair_foreign_and_restart(self):
        def seed(tx):
            a = self.service._grant(tx, 'ChallengeBundleSchedule:Unknown_A', {})
            b = self.service._grant(tx, 'ChallengeBundleSchedule:Unknown_B', {})
            self.assertEqual(tx.item(a)['attributes']['Granted_Bundles'], [])
            first = self.service._bundle(tx, next(iter(LOCATION_BUNDLES)), a)
            self.assertEqual(tx.item(a)['attributes']['Granted_Bundles'], [first])
            foreign = self.service._grant(tx, 'ChallengeBundle:Unknown', {'challenge_bundle_schedule_id': b})
            extra = self.service._grant(tx, 'ChallengeBundle:Legacy', {'challenge_bundle_schedule_id': a})
            tx.set_item_attr(a, 'Granted_Bundles', [foreign, first, first, 'missing'])
            del tx.item(b)['attributes']['Granted_Bundles']
            return a, b, first, foreign, extra
        a, b, first, foreign, extra = self.command(seed)
        self.login()
        items = self.profile()['items']
        self.assertEqual(items[a]['attributes']['Granted_Bundles'], [first, extra])
        self.assertEqual(items[b]['attributes']['Granted_Bundles'], [foreign])
        before = copy.deepcopy(items)
        self.login()
        self.assertEqual(self.profile()['items'], before)
        self.restart()
        self.assertEqual(self.profile()['items'], before)

    def test_malformed_membership_rolls_back_login_and_disk(self):
        schedule = self.command(lambda tx: self.service._grant(tx, 'ChallengeBundleSchedule:Unknown', {}))
        for invalid in ('bad', ['id', 3]):
            self.command(lambda tx: tx.set_item_attr(schedule, 'Granted_Bundles', invalid))
            before = copy.deepcopy(self.profile())
            with self.assertRaises(ApiError):
                self.login()
            self.assertEqual(self.profile(), before)
            self.restart()
            self.assertEqual(self.profile(), before)


if __name__ == '__main__':
    unittest.main()
