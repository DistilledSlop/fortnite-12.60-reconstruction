if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import tempfile
import unittest
from unittest.mock import patch
from account_service import AccountService
from api_common import Context, LOCAL_ACCOUNT_ID
from profile_service import ProfileService, CommandContext, _grant_season12_rewards
from quest_service import QuestService
from season12_rewards import PAID

TAG = 'EventFlag.LobbySeason12'
START = 1700000000
DAY = 86400

class DeadpoolQuestTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.now = START
        self.ctx = Context(clock=lambda: self.now)
        self.accounts = AccountService(self.ctx, state_dir=self.tmp.name)
        self.restart()
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        _grant_season12_rewards(ctx, ctx.tx('athena'), PAID[1], 0)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)

    def restart(self, starts=None):
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        self.quests = self.profiles.quests = QuestService(self.profiles, starts)

    def login(self):
        self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})

    def snapshot(self):
        return self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')

    def item(self, kind, name):
        template = (kind + ':' + name).casefold()
        return next((v for v in self.snapshot()['items'].values() if v['templateId'].casefold() == template), None)

    def quest(self, week):
        return self.item('Quest', 'Quest_S12_Donut_' + week)

    def progress(self, event, event_id=None, amount=1):
        return self.quests.progress(LOCAL_ACCOUNT_ID, event, amount, event_id=event_id or event)

    def chain_w1(self):
        self.progress('donut_w1_startquest')
        self.progress('dont_thank_bus_driver')

    def test_chain_gate_multicount_and_replay(self):
        self.chain_w1()
        self.assertIsNotNone(self.item('ChallengeBundle', 'QuestBundle_S12_Donut_W2'))
        self.assertIsNone(self.quest('W2_A'))
        self.progress('donut_w2_startquest', 'early')
        self.now = START + 11 * DAY
        self.restart({TAG: START})
        self.login()
        self.assertEqual(self.quest('W2_A')['attributes']['completion_quest_s12_donut_w2_a'], 0)
        self.assertFalse(self.progress('donut_w2_startquest', 'early'))
        self.progress('donut_w2_startquest', 'on_time')
        for event in ('donut_chimi_1', 'donut_chimi_2'):
            self.progress(event)
        self.assertEqual(self.quest('W2_B')['attributes']['quest_state'], 'Active')
        self.assertIsNone(self.item('ChallengeBundle', 'QuestBundle_S12_Donut_W3'))
        self.progress('donut_chimi_3')
        self.assertEqual(self.quest('W2_B')['attributes']['quest_state'], 'Claimed')
        self.assertIsNotNone(self.item('ChallengeBundle', 'QuestBundle_S12_Donut_W3'))
        self.assertIsNone(self.quest('W3_A'))
        self.now = START + 18 * DAY
        self.login()
        self.assertEqual(self.quest('W3_A')['attributes']['quest_state'], 'Active')
        self.restart({TAG: START})
        self.assertFalse(self.progress('donut_chimi_3'))
        self.assertEqual(self.quest('W2_B')['attributes']['quest_state'], 'Claimed')

    def test_independent_schedule_and_dummy(self):
        self.now = START + 42 * DAY
        self.restart({TAG: START})
        self.login()
        self.assertIsNotNone(self.item('ChallengeBundle', 'QuestBundle_S12_Donut_W7'))
        self.assertIsNone(self.quest('W7_A'))
        self.assertIsNone(self.item('ChallengeBundle', 'QuestBundle_S12_Donut_W7_A_ToastForInGame'))
        # An independently granted W6 reward follows the observed dummy link.
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        self.quests._bundle(ctx.tx('athena'), 'QuestBundle_S12_Donut_W6')
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.progress('donut_w6_startquest')
        self.progress('interact_donut_poster', amount=3)
        self.assertIsNotNone(self.item('ChallengeBundle', 'QuestBundle_S12_Donut_W7_Dummy'))
        self.assertIsNone(self.quest('W7_Dummy'))
        self.now = START + 46 * DAY
        self.login()
        self.assertIsNotNone(self.quest('W7_Dummy'))
        self.assertIsNotNone(self.quest('W7_A'))
        self.assertIsNotNone(self.quest('W7_A_ToastForInGame'))

    def test_multiobjective_fallback_and_large_damage(self):
        self.now = START + 46 * DAY
        self.restart({TAG: START})
        self.login()
        self.progress('donut_pistol_1')
        self.assertEqual(self.quest('W7_A')['attributes']['quest_state'], 'Active')
        self.assertIsNone(self.quest('W7_B'))
        self.progress('donut_pistol_2')
        self.assertIsNotNone(self.quest('W7_B'))
        self.assertIsNone(self.item('AthenaCharacter', 'CID_705_Athena_Commando_M_Donut'))
        self.progress('donut_enter_porta_or_phone')
        self.assertIsNotNone(self.item('AthenaCharacter', 'CID_705_Athena_Commando_M_Donut'))
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        self.quests._bundle(ctx.tx('athena'), 'QuestBundle_S12_Donut_W4')
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.progress('donut_katana_1')
        self.progress('donut_katana_2')
        self.progress('damage_athena_enemy_building', amount=10000)
        self.assertEqual(self.quest('W4_B')['attributes']['quest_state'], 'Claimed')
        self.assertIsNotNone(self.quest('W5_A'))

    def test_reward_failure_rolls_back_and_receipt_can_retry(self):
        before = self.snapshot()
        with patch.object(self.quests, '_quest', side_effect=RuntimeError('reward failure')):
            with self.assertRaises(RuntimeError):
                self.progress('donut_w1_startquest')
        self.assertEqual(before, self.snapshot())
        self.restart()
        self.assertEqual(before, self.snapshot())
        self.assertTrue(self.progress('donut_w1_startquest'))
        self.assertIsNotNone(self.quest('W1_B'))

if __name__ == '__main__':
    unittest.main()
