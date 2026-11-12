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
from season12_maya_data import MAYA_BUNDLE, MAYA_QUESTS
from season12_rewards import PAID

TAG = 'EventFlag.LobbySeason12'
START = 1700000000
DAY = 86400


class MayaQuestTests(unittest.TestCase):
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

    def item(self, template):
        profile = self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')
        return next((v for v in profile['items'].values()
                     if v['templateId'].casefold() == template.casefold()), None)

    def test_unlock_boundaries_and_restart(self):
        self.now = START + 100 * DAY
        self.login()  # No configured calendar start: only GrantWithBundle is due.
        for name, _, _, _ in MAYA_BUNDLE['quest_infos'][1:]:
            self.assertIsNone(self.item('Quest:' + name))
        self.restart({TAG: START})
        self.now = START
        self.assertTrue(self.quests.progress(LOCAL_ACCOUNT_ID, 'complete_athena_top_20', 5, event_id='early'))
        for name, _, days, _ in MAYA_BUNDLE['quest_infos'][1:]:
            self.now = START + days * DAY - 1
            self.login()
            self.assertIsNone(self.item('Quest:' + name))
            self.now += 1
            self.login()
            self.assertEqual(self.item('Quest:' + name)['attributes']['quest_state'], 'Active')
        self.assertFalse(self.quests.progress(LOCAL_ACCOUNT_ID, 'complete_athena_top_20', 5, event_id='early'))
        self.assertEqual(self.item('Quest:quest_s12_styles1_q04_athenarank')['attributes']['completion_quest_s12_styles1_q04_athenarank_obj'], 0)
        self.restart({TAG: START})
        for name in MAYA_QUESTS:
            self.assertIsNotNone(self.item('Quest:' + name))

    def test_all_rewards_replay_and_atomic_failure(self):
        self.now = START + 54 * DAY
        self.restart({TAG: START})
        self.login()
        name = 'quest_s12_styles1_q10_athenaoutlive'
        before = self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')
        with patch.object(self.quests, '_grant', side_effect=RuntimeError('reward failure')):
            with self.assertRaises(RuntimeError):
                self.quests.progress(LOCAL_ACCOUNT_ID, 'outlive_athena', 500, event_id='outlive')
        self.assertEqual(before, self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena'))
        self.restart({TAG: START})
        for index, (quest, definition) in enumerate(MAYA_QUESTS.items()):
            objective, event, target = definition['objectives'][0]
            event_id = 'outlive' if quest == name else str(index)
            self.assertTrue(self.quests.progress(LOCAL_ACCOUNT_ID, event, target, event_id=event_id))
            self.assertFalse(self.quests.progress(LOCAL_ACCOUNT_ID, event, target, event_id=event_id))
            attrs = self.item('Quest:' + quest)['attributes']
            self.assertEqual(attrs['quest_state'], 'Claimed')
            self.assertEqual(attrs['completion_' + objective.lower()], target)
            for kind, reward, _ in definition['rewards']:
                self.assertIsNotNone(self.item(kind + ':' + reward))
        self.restart({TAG: START})
        self.assertFalse(self.quests.progress(LOCAL_ACCOUNT_ID, 'outlive_athena', 500, event_id='outlive'))
        self.assertEqual(self.item('ChallengeBundle:QuestBundle_Styles_PaperDoll')['attributes']['num_quests_completed'], 9)
        self.assertEqual(self.item('Token:AthenaToken_BRS12_HeroSelection_10')['quantity'], 1)


if __name__ == '__main__':
    unittest.main()
