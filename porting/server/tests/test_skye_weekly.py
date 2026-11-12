if __package__:
    from . import _bootstrap
    from . import test_weekly_quests as weekly
else:
    import _bootstrap
    import test_weekly_quests as weekly

import unittest

from season12_weekly_quest_data import WEEKLY_QUESTS


class SkyeWeeklyTests(unittest.TestCase):
    setUp = weekly.WeeklyQuestTests.setUp
    items = weekly.WeeklyQuestTests.items
    get = weekly.WeeklyQuestTests.get
    login = weekly.WeeklyQuestTests.login
    grant_schedule = weekly.WeeklyQuestTests.grant_schedule
    progress = weekly.WeeklyQuestTests.progress
    quest = weekly.WeeklyQuestTests.quest

    def test_gates_and_three_of_five_campsites(self):
        self.grant_schedule()
        self.now = weekly.START + 45 * 86400 - 1
        self.login()
        self.assertIsNone(self.get('Quest:quest_s12_w7_q09'))
        self.now += 1
        self.login()
        self.assertIsNotNone(self.get('Quest:quest_s12_w7_q09'))
        self.assertIsNone(self.get('Quest:quest_s12_w8_q06'))
        for i in (5, 2):
            self.progress(f'visit_skye_campsite_{i}', f'site-{i}')
        self.assertEqual(self.quest('quest_s12_w7_q09')['quest_state'], 'Active')
        self.progress('visit_skye_campsite_4', 'site-4')
        self.assertEqual(self.quest('quest_s12_w7_q09')['quest_state'], 'Claimed')
        self.assertEqual(self.get('Token:AthenaBattlePass_AdventureGirlBundle_Token')['quantity'], 1)
        self.assertEqual(self.get('Token:AthenaBattlePass_WeeklyChallenge_Token')['quantity'], 1)
        self.assertIsNone(self.get('Quest:quest_AB_adventuregirl_ego'))
        self.now = weekly.START + 52 * 86400 - 1
        self.login()
        self.assertIsNone(self.get('Quest:quest_s12_w8_q06'))
        self.now += 1
        self.login()
        self.assertIsNotNone(self.get('Quest:quest_s12_w8_q06'))

    def test_week_eight_match_rules(self):
        self.grant_schedule()
        self.now = weekly.START + 52 * 86400
        self.login()
        self.progress('land_spybase_shark', 'a', match_id='m1')
        self.progress('visit_landmark_theagencyhq', 'b', match_id='m2')
        self.assertEqual(self.quest('quest_s12_w8_q06')['quest_state'], 'Active')
        self.progress('land_spybase_shark', 'c', match_id='m2')
        self.assertEqual(self.quest('quest_s12_w8_q06')['quest_state'], 'Claimed')
        self.progress('use_applesun', 'd', 9, 'm1')
        self.progress('use_applesun', 'e', 1, 'm1')
        self.progress('use_applesun', 'f', 1, 'm2')
        self.assertEqual(self.quest('quest_s12_w8_q10')['quest_state'], 'Active')
        self.progress('use_applesun', 'g', 1, 'm3')
        self.assertEqual(self.quest('quest_s12_w8_q10')['quest_state'], 'Claimed')

    def test_tenth_week_eight_completion_awards_bundle_token(self):
        self.grant_schedule()
        self.now = weekly.START + 52 * 86400
        self.login()
        for number in range(1, 11):
            definition = WEEKLY_QUESTS[f'quest_s12_w8_q{number:02}']
            objectives = definition['objectives'][:definition.get('ObjectiveCompletionCount', len(definition['objectives']))]
            for objective, event, target in objectives:
                if definition.get('bAthenaUpdateObjectiveOncePerMatch'):
                    for match in range(target):
                        self.progress(event, f'{objective}-{match}', match_id=f'm{match}')
                else:
                    match = 'm1' if definition.get('bAthenaMustCompleteInSingleMatch') else None
                    self.progress(event, objective, target, match)
            self.assertEqual(self.quest(f'quest_s12_w8_q{number:02}')['quest_state'], 'Claimed')
            if number == 9:
                self.assertIsNone(self.get('Token:AthenaBattlePass_WeeklyBundle_Token'))
        self.assertEqual(self.get('Token:AthenaBattlePass_WeeklyBundle_Token')['quantity'], 1)
        self.assertEqual(self.get('Token:AthenaBattlePass_AdventureGirlBundle_Token')['quantity'], 10)
