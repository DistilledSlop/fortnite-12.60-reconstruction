"""The Season 12 mission entitlement is reconciled through MCP login."""
if __package__:
    from . import _bootstrap, test_services as support
else:
    import _bootstrap
    import test_services as support

import copy
import tempfile
from unittest.mock import patch

from api_common import ApiError
from backend import Backend
from profile_service import CommandContext


class SeasonMissionEntitlementTests(support.ServiceTestCase):
    def setUp(self):
        super().setUp()
        self.state = tempfile.TemporaryDirectory()
        self.addCleanup(self.state.cleanup)
        self.restart()

    def restart(self):
        self.start_backend(Backend(clock=self.clock, state_step_sec=0, state_dir=self.state.name))
        self.token = self.sign_in()['access_token']

    def mcp(self, command):
        return support.mcp(self.base, self.token, support.LOCAL, command, 'athena')

    def profile(self):
        return self.backend.profiles.profile(support.LOCAL, 'athena')

    def find(self, template):
        return [(key, item) for key, item in self.profile()['items'].items()
                if item['templateId'].casefold() == template.casefold()]

    def test_free_profile_login_and_restart_preserve_progress(self):
        self.assertFalse(self.profile()['stats']['attributes']['book_purchased'])
        self.assertEqual(self.mcp('QueryProfile')[0], 200)
        self.assertFalse(self.find('ChallengeBundleSchedule:Season12_Mission_Schedule'))
        self.assertEqual(self.mcp('ClientQuestLogin')[0], 200)
        schedule = self.find('ChallengeBundleSchedule:Season12_Mission_Schedule')
        self.assertEqual(len(schedule), 1)
        self.assertEqual(schedule[0][1]['quantity'], 1)
        quest = self.find('Quest:quest_s12_w1_q01')
        self.assertEqual(len(quest), 1)
        self.assertFalse(self.find('Quest:quest_s12_w2_q01'))
        self.backend.profiles.quests.progress(support.LOCAL, 'damage_athena_henchmen', 100,
                                              event_id='durable-progress')
        progress = self.find('Quest:quest_s12_w1_q02')[0][1]['attributes']
        self.assertEqual(progress['completion_quest_s12_w1_q02_obj'], 100)
        before = copy.deepcopy(self.profile()['items'])
        self.assertEqual(self.mcp('ClientQuestLogin')[0], 200)
        self.assertEqual(self.profile()['items'], before)
        self.restart()
        self.assertEqual(self.mcp('ClientQuestLogin')[0], 200)
        self.assertEqual(self.profile()['items'], before)
        self.assertFalse(self.backend.profiles.quests.progress(
            support.LOCAL, 'damage_athena_henchmen', 100, event_id='durable-progress'))
        self.assertEqual(self.profile()['items'], before)
        self.clock.now += 30 * support.DAY
        self.token = self.sign_in()['access_token']
        self.assertEqual(self.mcp('ClientQuestLogin')[0], 200)
        self.assertFalse(self.find('Quest:quest_s12_w2_q01'))

    def test_other_season_and_failed_login_do_not_grant(self):
        ctx = CommandContext(self.backend.profiles, support.LOCAL, 'athena', {})
        ctx.tx('athena').set_stat('season_num', 11)
        self.backend.profiles._store(support.LOCAL, ctx.txs)
        self.assertEqual(self.mcp('ClientQuestLogin')[0], 200)
        self.assertFalse(self.find('ChallengeBundleSchedule:Season12_Mission_Schedule'))
        ctx = CommandContext(self.backend.profiles, support.LOCAL, 'athena', {})
        ctx.tx('athena').set_stat('season_num', 12)
        self.backend.profiles._store(support.LOCAL, ctx.txs)
        before = copy.deepcopy(self.profile())
        with patch.object(self.backend.profiles.quests, 'login',
                          side_effect=ApiError(400, 'test.failure', 'failure')):
            self.assertEqual(self.mcp('ClientQuestLogin')[0], 400)
        self.assertEqual(self.profile(), before)
        self.restart()
        self.assertEqual(self.profile(), before)
