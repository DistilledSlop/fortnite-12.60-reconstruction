if __package__:
    from . import _bootstrap
    from . import test_weekly_quests as weekly
else:
    import _bootstrap
    import test_weekly_quests as weekly

import unittest

from season12_weekly_quest_data import WEEKLY_QUESTS


class MeowsclesWeeklyTests(unittest.TestCase):
    setUp = weekly.WeeklyQuestTests.setUp
    items = weekly.WeeklyQuestTests.items
    get = weekly.WeeklyQuestTests.get
    login = weekly.WeeklyQuestTests.login
    grant_schedule = weekly.WeeklyQuestTests.grant_schedule
    progress = weekly.WeeklyQuestTests.progress
    quest = weekly.WeeklyQuestTests.quest

    def test_schedule_gates_and_single_match_objectives(self):
        self.grant_schedule()
        self.now = weekly.START + 31 * 86400 - 1
        self.login()
        self.assertIsNone(self.get('Quest:quest_s12_w5_q07'))
        self.now += 1
        self.login()
        self.assertIsNotNone(self.get('Quest:quest_s12_w5_q07'))
        self.assertIsNone(self.get('Quest:quest_s12_w6_q10'))
        objectives = WEEKLY_QUESTS['quest_s12_w5_q07']['objectives']
        self.progress(objectives[0][1], 'first', match_id='m1')
        self.progress(objectives[1][1], 'second', match_id='m2')
        self.progress(objectives[2][1], 'third', match_id='m2')
        self.assertEqual(self.quest('quest_s12_w5_q07')['quest_state'], 'Active')
        self.progress(objectives[0][1], 'fourth', match_id='m2')
        self.assertEqual(self.quest('quest_s12_w5_q07')['quest_state'], 'Claimed')
        self.assertEqual(self.get('Token:AthenaBattlePass_MeowsclesBundle_Token')['quantity'], 1)
        self.assertEqual(self.get('Token:AthenaBattlePass_WeeklyChallenge_Token')['quantity'], 1)
        self.assertIsNone(self.get('Quest:quest_AB_meowscle_ego'))
        self.now = weekly.START + 38 * 86400 - 1
        self.login()
        self.assertIsNone(self.get('Quest:quest_s12_w6_q10'))
        self.now += 1
        self.login()
        self.assertIsNotNone(self.get('Quest:quest_s12_w6_q10'))

    def test_tenth_week_six_completion_awards_bundle_token(self):
        self.grant_schedule()
        self.now = weekly.START + 38 * 86400
        self.login()
        for number in range(1, 11):
            definition = WEEKLY_QUESTS[f'quest_s12_w6_q{number:02}']
            for objective, event, target in definition['objectives']:
                match = 'm1' if definition.get('bAthenaMustCompleteInSingleMatch') else None
                self.progress(event, objective, target, match)
            self.assertEqual(self.quest(f'quest_s12_w6_q{number:02}')['quest_state'], 'Claimed')
            if number == 9:
                self.assertIsNone(self.get('Token:AthenaBattlePass_WeeklyBundle_Token'))
        self.assertEqual(self.get('Token:AthenaBattlePass_WeeklyBundle_Token')['quantity'], 1)
        self.assertEqual(self.get('Token:AthenaBattlePass_MeowsclesBundle_Token')['quantity'], 10)
