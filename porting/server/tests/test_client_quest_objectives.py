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
from season12_client_objective_data import CLIENT_OBJECTIVES

START = 1700000000

class ClientObjectiveTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.clock = Context(clock=lambda: START + 60 * 86400)
        self.restart()

    def restart(self):
        self.accounts = AccountService(self.clock, state_dir=self.tmp.name)
        self.profiles = ProfileService(self.clock, self.accounts, None, state_dir=self.tmp.name)
        self.service = self.profiles.quests = QuestService(self.profiles, {'EventFlag.LobbySeason12': START})

    def command(self, entries):
        return self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'UpdateQuestClientObjectives',
                                 {'advance': [{'statName': s, 'count': 1, 'timestampOffset': 0} for s in entries]})

    def item(self, template):
        return next((v for v in self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')['items'].values()
                     if v['templateId'].casefold() == template.casefold()), None)

    def grant(self, name, bundle=None):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        tx = ctx.tx('athena')
        bundle_id = self.service._bundle(tx, bundle) if bundle else None
        self.service._quest(tx, name, bundle_id)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)

    def test_multiobjective_claim_and_restart(self):
        self.grant('Quest_S12_Donut_W2_B', 'QuestBundle_S12_Donut_W2')
        names = ['quest_s12_donut_w2_b_' + str(i) for i in range(3)]
        for i in range(2):
            self.command([names[i], names[i]])
            attrs = self.item('Quest:Quest_S12_Donut_W2_B')['attributes']
            self.assertEqual(attrs['quest_state'], 'Active')
            self.assertEqual(attrs['completion_' + names[i]], 1)
            self.assertEqual(self.item('ChallengeBundle:QuestBundle_S12_Donut_W2')['attributes']['num_quests_completed'], 0)
        self.command([names[2]])
        self.assertEqual(self.item('Quest:Quest_S12_Donut_W2_B')['attributes']['quest_state'], 'Claimed')
        before = copy.deepcopy(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
        self.restart()
        self.command(names)
        self.assertEqual(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')['items'], before['items'])
        self.assertEqual(self.item('ChallengeBundle:QuestBundle_S12_Donut_W2')['attributes']['num_quests_completed'], 1)
        self.assertIsNotNone(self.item('ChallengeBundle:QuestBundle_S12_Donut_W3'))

    def test_owned_midas_and_invalid_batch(self):
        names = ['quest_s12_sta_findfrontend_' + str(i) for i in range(3)]
        self.command(names)
        self.assertIsNone(self.item('Quest:quest_s12_sta_findfrontend'))
        self.grant('quest_s12_sta_findfrontend')
        before = copy.deepcopy(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
        for invalid in ['frontend_midas_sta_1', names[0].upper(), 'unknown']:
            with self.assertRaises(ApiError):
                self.command([names[0], invalid])
            self.assertEqual(self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'), before)
        self.command(names[:2])
        self.assertEqual(self.item('Quest:quest_s12_sta_findfrontend')['attributes']['quest_state'], 'Active')
        self.command(names[2:])
        self.assertEqual(self.item('Quest:quest_s12_sta_findfrontend')['attributes']['quest_state'], 'Claimed')
        self.assertEqual(len(CLIENT_OBJECTIVES), 20)

    def test_newly_granted_successor_waits_for_next_batch(self):
        self.grant('Quest_S12_Donut_W2_A', 'QuestBundle_S12_Donut_W2')
        a = 'quest_s12_donut_w2_a'
        b = 'quest_s12_donut_w2_b_0'
        self.assertIsNone(self.item('Quest:Quest_S12_Donut_W2_B'))
        self.command([a, b])
        successor = self.item('Quest:Quest_S12_Donut_W2_B')
        self.assertIsNotNone(successor)
        self.assertEqual(successor['attributes']['completion_' + b], 0)
        self.command([b])
        self.assertEqual(self.item('Quest:Quest_S12_Donut_W2_B')['attributes']['completion_' + b], 1)

if __name__ == '__main__':
    unittest.main()
