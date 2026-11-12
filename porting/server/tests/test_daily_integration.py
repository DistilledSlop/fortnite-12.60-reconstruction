"""Daily MCP lifecycle and trusted progress share durable XP/reward receipts."""
if __package__:
    from . import _bootstrap, test_services as support
else:
    import _bootstrap
    import test_services as support

import tempfile
from unittest.mock import patch

from backend import Backend
from daily_quest_service import DailyPolicy, DailyService
from season12_daily_quest_data import DAILY_QUESTS, daily_quest_name
from season12_progression_data import XP_TO_NEXT


class DailyIntegrationTests(support.ServiceTestCase):
    def setUp(self):
        super().setUp()
        self.state = tempfile.TemporaryDirectory()
        self.addCleanup(self.state.cleanup)
        self.restart()

    def restart(self):
        self.start_backend(Backend(clock=self.clock, state_step_sec=0, state_dir=self.state.name))
        self.quests = self.backend.profiles.quests
        self.quests.daily = DailyService(self.backend.profiles,
            DailyPolicy(candidates=('Quest_S12_Daily_Damage', 'Quest_S12_Daily_Search_Chests')))
        self.token = self.sign_in()['access_token']

    def mcp(self, name, body=None):
        return support.mcp(self.base, self.token, support.LOCAL, name, body=body)

    def profile(self):
        return self.backend.profiles.profile(support.LOCAL, 'athena')

    def active(self):
        return [(i, v) for i, v in self.profile()['items'].items()
                if daily_quest_name(v['templateId']) and v['attributes']['quest_state'] == 'Active']

    def finish(self, item, event_id):
        definition = DAILY_QUESTS[daily_quest_name(item['templateId'])]
        _, event, target = definition['objectives'][0]
        return self.quests.progress(support.LOCAL, event, target, event_id=event_id)

    def cumulative(self, key, level):
        stats = self.profile()['stats']['attributes']
        return sum(XP_TO_NEXT[:stats[level] - 1]) + stats.get(key, 0)

    def test_recurring_completion_xp_token_and_durable_replay(self):
        self.assertEqual(self.mcp('ClientQuestLogin')[0], 200)
        first_id, first = self.active()[0]
        start_xp = self.cumulative('xp', 'level')
        start_book_xp = self.cumulative('book_xp', 'book_level')
        self.assertTrue(self.finish(first, 'first'))
        self.assertEqual(self.cumulative('xp', 'level') - start_xp, 31000)
        self.assertEqual(self.cumulative('book_xp', 'book_level') - start_book_xp, 31000)
        self.assertFalse(self.finish(first, 'first'))
        self.clock.now += support.DAY
        self.token = self.sign_in()['access_token']
        self.assertEqual(self.mcp('ClientQuestLogin')[0], 200)
        second_id, second = self.active()[0]
        self.assertNotEqual(first_id, second_id)
        self.assertTrue(self.finish(second, 'second'))
        self.restart()
        self.assertFalse(self.finish(first, 'first'))
        self.assertFalse(self.finish(second, 'second'))
        self.assertEqual(self.cumulative('xp', 'level') - start_xp, 62000)
        tokens = [v for v in self.profile()['items'].values()
                  if v['templateId'].casefold() == 'token:athena_dailychallenge_token']
        self.assertEqual(len(tokens), 1)
        self.assertEqual(tokens[0]['quantity'], 2)
        self.assertEqual(self.profile()['items'][first_id]['attributes']['quest_state'], 'Claimed')

    def test_mcp_reroll_and_failed_xp_leave_no_reward_or_receipt(self):
        self.assertEqual(self.mcp('ClientQuestLogin')[0], 200)
        old_id, old = self.active()[0]
        self.assertEqual(self.mcp('FortRerollDailyQuest', {'questId': old_id})[0], 200)
        new_id, new = self.active()[0]
        self.assertNotEqual(old['templateId'], new['templateId'])
        self.assertNotIn(old_id, self.profile()['items'])
        self.assertEqual(self.mcp('FortRerollDailyQuest', {'questId': new_id})[0], 400)
        before = self.profile()
        with patch('progression_service.award_profile_xp', side_effect=RuntimeError('award failure')):
            with self.assertRaises(RuntimeError):
                self.finish(new, 'retry')
        self.assertEqual(self.profile(), before)
        self.restart()
        self.assertTrue(self.finish(new, 'retry'))
        self.assertFalse(self.finish(new, 'retry'))
        self.assertEqual(self.profile()['stats']['attributes']['quest_manager']['dailyQuestRerolls'], 0)
