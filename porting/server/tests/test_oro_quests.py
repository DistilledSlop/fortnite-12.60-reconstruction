"""Oro calendar, progression, and bundle reward integration."""
if __package__:
    from . import _bootstrap, test_storm_agency_quests as helpers
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap
    import test_storm_agency_quests as helpers

import copy
import unittest
from unittest.mock import patch

from account_service import AccountService
from api_common import LOCAL_ACCOUNT_ID
from backend import Backend
from probe_server import quest_event_start
from profile_service import ProfileService
from quest_service import QuestService
from season12_oro_data import ORO_BUNDLES, ORO_QUESTS, ORO_SCHEDULE
from season12_progression_data import XP_TO_NEXT

START = 1700000000
TAG = 'EventFlag.LTE_Oro'
BUNDLE = next(iter(ORO_BUNDLES))
WRAP = 'AthenaItemWrap:Wrap_191_GoldenSkeleton'
PICKAXE = 'AthenaPickaxe:Pickaxe_ID_350_TheGoldenSkeleton'


class OroTests(unittest.TestCase):
    setUp = helpers.StormAgencyTests.setUp
    items = helpers.StormAgencyTests.items
    get = helpers.StormAgencyTests.get
    command = helpers.StormAgencyTests.command
    login = helpers.StormAgencyTests.login
    event = helpers.StormAgencyTests.event

    def test_activation_season_and_owned_schedule(self):
        self.login()
        self.assertIsNone(self.get('ChallengeBundleSchedule:' + ORO_SCHEDULE['name']))
        self.service.event_starts[TAG] = START + 1
        self.login()
        self.assertIsNone(self.get('ChallengeBundle:' + BUNDLE))
        self.command(lambda tx: tx.set_stat('season_num', 11))
        self.now += 1
        self.login()
        self.assertIsNone(self.get('ChallengeBundle:' + BUNDLE))
        self.command(lambda tx: tx.set_stat('season_num', 12))
        self.login()
        self.assertFalse(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')['stats']['attributes']['book_purchased'])
        for name, *_ in ORO_BUNDLES[BUNDLE]['quest_infos']:
            self.assertIsNotNone(self.get('Quest:' + name))
        self.service.event_starts.clear()
        self.command(lambda tx: tx.remove_item(next(i for i, v in tx.profile['items'].items()
                     if v['templateId'].casefold() == ('ChallengeBundle:' + BUNDLE).casefold())))
        self.login()
        self.assertIsNotNone(self.get('ChallengeBundle:' + BUNDLE))

    def test_counts_rewards_receipts_restart_and_rollback(self):
        self.service.event_starts[TAG] = START
        self.login()
        expected = [('killcontribution_athena_player_teammate', 10),
                    ('complete_athena_friend', 10), ('damage_athena_player', 1000),
                    ('accolade_medal', 40)]
        self.assertEqual([(ORO_QUESTS[n]['objectives'][0][1], ORO_QUESTS[n]['objectives'][0][2])
                          for n, *_ in ORO_BUNDLES[BUNDLE]['quest_infos']], expected)
        self.assertTrue(self.event(expected[0][0], 'first', 5))
        self.assertFalse(self.event(expected[0][0], 'first', 5))
        self.assertTrue(self.event(expected[0][0], 'second', 5))
        self.assertIsNone(self.get(WRAP))
        before = copy.deepcopy(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
        grant = self.service._grant
        def fail_wrap(tx, template, attrs):
            if template.casefold() == WRAP.casefold():
                raise RuntimeError('bundle reward failed')
            return grant(tx, template, attrs)
        with patch.object(self.service, '_grant', side_effect=fail_wrap):
            with self.assertRaises(RuntimeError):
                self.event(expected[1][0], 'finish-friend', expected[1][1])
        self.assertEqual(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'), before)
        self.assertTrue(self.event(expected[1][0], 'finish-friend', expected[1][1]))
        self.assertIsNotNone(self.get(WRAP))
        self.assertIsNone(self.get(PICKAXE))
        self.event(expected[2][0], 'damage', expected[2][1])
        self.assertIsNone(self.get(PICKAXE))
        self.event(expected[3][0], 'medals', expected[3][1])
        self.assertIsNotNone(self.get(PICKAXE))
        stats = self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')['stats']['attributes']
        self.assertEqual(sum(XP_TO_NEXT[:stats['level'] - 1]) + stats['xp'], 400000)
        self.accounts = AccountService(self.clock, state_dir=self.tmp.name)
        self.profiles = ProfileService(self.clock, self.accounts, None, state_dir=self.tmp.name)
        self.service = self.profiles.quests = QuestService(self.profiles, {TAG: START})
        self.login()
        self.assertFalse(self.event(expected[3][0], 'medals', expected[3][1]))
        self.assertEqual(self.get('ChallengeBundle:' + BUNDLE)['attributes']['num_progress_quests_completed'], 4)

    def test_unrelated_linked_quest_does_not_trigger_bundle_rewards(self):
        self.service.event_starts[TAG] = START
        self.login()
        bundle_id = next(i for i, v in self.items().items() if v['templateId'].casefold() == ('ChallengeBundle:' + BUNDLE).casefold())
        self.command(lambda tx: self.service._quest(tx, 'quest_s12_sta_landagency', bundle_id))
        self.event('objective:quest_s12_sta_landagency', 'unrelated')
        self.assertEqual(self.get('ChallengeBundle:' + BUNDLE)['attributes']['num_progress_quests_completed'], 0)
        self.assertIsNone(self.get(WRAP))
        name = ORO_BUNDLES[BUNDLE]['quest_infos'][0][0]
        _, stat, count = ORO_QUESTS[name]['objectives'][0]
        self.event(stat, 'first-root', count)
        self.assertEqual(self.get('ChallengeBundle:' + BUNDLE)['attributes']['num_progress_quests_completed'], 1)
        self.assertIsNone(self.get(WRAP))
        self.assertIsNone(self.get(PICKAXE))

    def test_parser_and_calendar_match_login_boundary(self):
        self.assertEqual(quest_event_start(TAG + '=1700000001'), (TAG, START + 1))
        backend = Backend(clock=lambda: self.now, quest_event_starts={TAG: START + 1})
        self.addCleanup(backend.close)
        def check(active):
            _, calendar = backend.content.timeline(None, None)
            flags = calendar['channels']['client-events']['states'][0]['activeEvents']
            self.assertEqual(any(e['eventType'] == TAG for e in flags), active)
            backend.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})
            items = backend.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')['items'].values()
            self.assertEqual(any(v['templateId'].casefold() == ('ChallengeBundle:' + BUNDLE).casefold() for v in items), active)
        check(False)
        self.now += 1
        check(True)


if __name__ == '__main__':
    unittest.main()
