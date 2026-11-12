if __package__:
    from . import _bootstrap
    from . import test_weekly_quests as weekly
else:
    import _bootstrap
    import test_weekly_quests as weekly

import unittest

from season12_weekly_quest_data import WEEKLY_QUESTS


class MidasWeeklyTests(unittest.TestCase):
    setUp = weekly.WeeklyQuestTests.setUp
    items = weekly.WeeklyQuestTests.items
    get = weekly.WeeklyQuestTests.get
    login = weekly.WeeklyQuestTests.login
    grant_schedule = weekly.WeeklyQuestTests.grant_schedule
    progress = weekly.WeeklyQuestTests.progress
    quest = weekly.WeeklyQuestTests.quest

    def test_gates_and_five_distinct_chest_locations(self):
        self.grant_schedule()
        self.now = weekly.START + 59 * 86400 - 1
        self.login()
        self.assertIsNone(self.get('Quest:quest_s12_w9_q01'))
        self.now += 1
        self.login()
        self.assertIsNotNone(self.get('Quest:quest_s12_w9_q01'))
        self.assertIsNone(self.get('Quest:quest_s12_w10_q01'))
        objectives = WEEKLY_QUESTS['quest_s12_w9_q01']['objectives']
        for n, (_, row, _) in enumerate(objectives[:4]):
            self.progress(row, f'loc-{n}')
        self.progress(objectives[0][1], 'duplicate-location')
        self.assertEqual(self.quest('quest_s12_w9_q01')['quest_state'], 'Active')
        self.progress(objectives[4][1], 'loc-4')
        self.assertEqual(self.quest('quest_s12_w9_q01')['quest_state'], 'Claimed')
        self.assertEqual(self.get('Token:AthenaBattlePass_MidasBundle_Token')['quantity'], 1)
        self.now = weekly.START + 66 * 86400 - 1
        self.login()
        self.assertIsNone(self.get('Quest:quest_s12_w10_q01'))
        self.now += 1
        self.login()
        self.assertIsNotNone(self.get('Quest:quest_s12_w10_q01'))

    def test_week_ten_match_rules(self):
        self.grant_schedule()
        self.now = weekly.START + 66 * 86400
        self.login()
        self.progress('damage_opponent_agency', 'a', match_id='m1')
        self.progress('damage_opponent_yacht', 'b', match_id='m2')
        self.assertEqual(self.quest('quest_s12_w10_q08')['quest_state'], 'Active')
        self.progress('damage_opponent_agency', 'c', match_id='m2')
        self.assertEqual(self.quest('quest_s12_w10_q08')['quest_state'], 'Claimed')
        self.progress('interact_athena_treasurechest', 'd', 7, 'm1')
        self.progress('interact_athena_treasurechest', 'e', 7, 'm1')
        for i in range(2, 7):
            self.progress('interact_athena_treasurechest', f'chest-{i}', 1, f'm{i}')
        self.assertEqual(self.quest('quest_s12_w10_q02')['quest_state'], 'Active')
        self.progress('interact_athena_treasurechest', 'last', 1, 'm7')
        self.assertEqual(self.quest('quest_s12_w10_q02')['quest_state'], 'Claimed')

    def test_tenth_completion_awards_bundle_token(self):
        self.grant_schedule()
        self.now = weekly.START + 66 * 86400
        self.login()
        for number in range(1, 11):
            definition = WEEKLY_QUESTS[f'quest_s12_w10_q{number:02}']
            objectives = definition['objectives'][:definition.get('ObjectiveCompletionCount', len(definition['objectives']))]
            for objective, event, target in objectives:
                if definition.get('bAthenaUpdateObjectiveOncePerMatch'):
                    for match in range(target):
                        self.progress(event, f'{objective}-{match}', match_id=f'm{match}')
                else:
                    match = 'm1' if definition.get('bAthenaMustCompleteInSingleMatch') else None
                    self.progress(event, objective, target, match)
            self.assertEqual(self.quest(f'quest_s12_w10_q{number:02}')['quest_state'], 'Claimed')
            if number == 9:
                self.assertIsNone(self.get('Token:AthenaBattlePass_WeeklyBundle_Token'))
        self.assertEqual(self.get('Token:AthenaBattlePass_WeeklyBundle_Token')['quantity'], 1)
        self.assertEqual(self.get('Token:AthenaBattlePass_MidasBundle_Token')['quantity'], 10)
