"""Storm the Agency local grant policy and trusted prequalified progress."""
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
from season12_progression_data import XP_TO_NEXT
from season12_storm_agency_data import STORM_AGENCY_QUESTS, STORM_AGENCY_BUNDLES, STORM_AGENCY_SCHEDULE

START = 1700000000
TAG = 'EventFlag.LTE_StormTheAgency'
BUNDLE = next(iter(STORM_AGENCY_BUNDLES))

class StormAgencyTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.now = START
        self.clock = Context(clock=lambda: self.now)
        self.accounts = AccountService(self.clock, state_dir=self.tmp.name)
        self.profiles = ProfileService(self.clock, self.accounts, None, state_dir=self.tmp.name)
        self.service = self.profiles.quests = QuestService(self.profiles)

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

    def event(self, event, eid=None, amount=1):
        return self.service.progress(LOCAL_ACCOUNT_ID, event, amount, event_id=eid or event)

    def test_date_opt_in_and_owned_schedule(self):
        self.login()
        self.assertIsNone(self.get('ChallengeBundleSchedule:' + STORM_AGENCY_SCHEDULE['name']))
        self.service.event_starts[TAG] = START + 1
        self.login()
        self.assertIsNone(self.get('ChallengeBundle:' + BUNDLE))
        self.command(lambda tx: tx.set_stat('season_num', 11))
        self.now += 1
        self.login()
        self.assertIsNone(self.get('ChallengeBundle:' + BUNDLE))
        self.command(lambda tx: tx.set_stat('season_num', 12))
        self.login()
        self.assertIsNotNone(self.get('ChallengeBundle:' + BUNDLE))
        self.assertFalse(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')['stats']['attributes']['book_purchased'])
        self.assertIsNone(self.get('Quest:quest_s12_sta_findfrontend'))
        for name, *_ in STORM_AGENCY_BUNDLES[BUNDLE]['quest_infos']:
            self.assertIsNotNone(self.get('Quest:' + name))
        # Owned schedules reconcile even without operator activation.
        self.service.event_starts.clear()
        self.command(lambda tx: tx.remove_item(next(i for i, v in tx.profile['items'].items() if v['templateId'].casefold() == ('challengebundle:' + BUNDLE).casefold())))
        self.login()
        self.assertIsNotNone(self.get('ChallengeBundle:' + BUNDLE))

    def test_progress_rewards_replay_and_rollback(self):
        self.service.event_starts[TAG] = START
        self.login()
        with self.assertRaises(ApiError):
            self.event('objective:bogus')
        with self.assertRaises(ApiError):
            self.event('quest_s12_sta_landagency')
        before = copy.deepcopy(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
        with patch('progression_service.award_profile_xp', side_effect=RuntimeError('award failed')):
            with self.assertRaises(RuntimeError):
                self.event('objective:quest_s12_sta_landagency')
        self.assertEqual(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'), before)
        self.assertTrue(self.event('objective:quest_s12_sta_landagency'))
        for prefix, quest in [('open_faction_locked_chests_', 'openfactionchestdifferentspybases'), ('eliminate_henchmen_', 'elimhenchmandifferentsafehouses')]:
            objectives = STORM_AGENCY_QUESTS['quest_s12_sta_' + quest]['objectives']
            for attempt, index in enumerate((0, 0, 1)):
                self.event(objectives[index][1], prefix + str(attempt))
            self.assertEqual(self.get('Quest:quest_s12_sta_' + quest)['attributes']['quest_state'], 'Active')
            self.event(objectives[2][1])
        for index, (_, stat, _) in enumerate(STORM_AGENCY_QUESTS['quest_s12_sta_swimhatches']['objectives']):
            if index == 4:
                self.assertIsNone(self.get('AthenaGlider:Glider_ID_208_BadEggMale'))
            self.event(stat)
        self.assertIsNone(self.get('AthenaGlider:Glider_ID_208_BadEggMale'))
        self.event('outlive_athena_stormcircles', amount=10)
        for template in ('AthenaGlider:Glider_ID_208_BadEggMale', 'AthenaItemWrap:Wrap_232_BadEgg', 'AthenaPickaxe:Pickaxe_ID_385_BadEggMale'):
            self.assertIsNotNone(self.get(template))
        attrs = self.get('ChallengeBundle:' + BUNDLE)['attributes']
        self.assertEqual((attrs['num_quests_completed'], attrs['num_progress_quests_completed']), (5, 5))
        stats = self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')['stats']['attributes']
        self.assertEqual(sum(XP_TO_NEXT[:stats['level'] - 1]) + stats['xp'], 120000)
        self.accounts = AccountService(self.clock, state_dir=self.tmp.name)
        self.profiles = ProfileService(self.clock, self.accounts, None, state_dir=self.tmp.name)
        self.service = self.profiles.quests = QuestService(self.profiles, {TAG: START})
        self.login()
        self.assertFalse(self.event('outlive_athena_stormcircles', amount=10))
        self.assertEqual(self.get('ChallengeBundle:' + BUNDLE)['attributes']['num_progress_quests_completed'], 5)

    def test_manual_frontend_link_casefold(self):
        self.command(lambda tx: self.service._quest(tx, 'quest_s12_sta_findfrontend', None))
        for i in range(1, 4):
            self.event('frontend_midas_sta_' + str(i))
        self.assertEqual(self.get('Quest:quest_s12_sta_findfrontend')['attributes']['quest_state'], 'Claimed')
        self.assertIsNotNone(self.get('Quest:quest_s12_sta_landagency'))
        self.service.event_starts[TAG] = START
        self.login()
        bundle_id = next(i for i, v in self.items().items() if v['templateId'].casefold() == ('challengebundle:' + BUNDLE).casefold())
        self.assertEqual(self.get('Quest:quest_s12_sta_landagency')['attributes']['challenge_bundle_id'], bundle_id)

if __name__ == '__main__':
    unittest.main()
