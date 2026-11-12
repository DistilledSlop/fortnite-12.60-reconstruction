"""Fixed quest reward claims validate server state and commit rewards once."""
if __package__:
    from . import _bootstrap
else:
    import _bootstrap

import tempfile
import unittest
from unittest.mock import patch

from backend import Backend
from api_common import ApiError, LOCAL_ACCOUNT_ID
from profile_service import ProfileTx
from season12_daily_quest_data import DAILY_QUESTS


class QuestClaimTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.restart()
        self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})
        self.quest_id, self.quest = next((i, v) for i, v in self.snapshot()['items'].items()
                                        if v['templateId'].startswith('Quest:Quest_S12_Daily_'))

    def restart(self):
        self.profiles = Backend(state_dir=self.directory.name).profiles

    def snapshot(self):
        return self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')

    def complete(self, *, progress=True):
        tx = ProfileTx(self.profiles.ctx, self.snapshot())
        tx.set_item_attr(self.quest_id, 'quest_state', 'Completed')
        if progress:
            for objective, _, target in DAILY_QUESTS[self.quest['templateId'][6:]]['objectives']:
                tx.set_item_attr(self.quest_id, 'completion_' + objective.lower(), target)
        self.profiles._store(LOCAL_ACCOUNT_ID, {'athena': tx})

    def claim(self, **body):
        return self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClaimQuestReward',
                                 body or {'questId': self.quest_id, 'selectedRewardIndex': -1})

    def test_completed_claim_rewards_once_across_restart(self):
        self.complete()
        self.claim()
        after = self.snapshot()
        self.assertEqual(after['items'][self.quest_id]['attributes']['quest_state'], 'Claimed')
        tokens = [v for v in after['items'].values()
                  if v['templateId'].casefold() == 'token:athena_dailychallenge_token']
        self.assertEqual([v['quantity'] for v in tokens], [1])
        self.assertGreater(after['stats']['attributes']['level'], 1)
        self.restart()
        self.claim()
        again = self.snapshot()
        # MCP command revisions can advance on a no-op; rewards and state cannot.
        self.assertEqual(after['items'], again['items'])
        self.assertEqual(after['stats'], again['stats'])
        self.assertEqual(after['rvn'], again['rvn'])

    def test_active_incomplete_unknown_and_invalid_index_reject(self):
        before = self.snapshot()
        for body in ({'questId': self.quest_id, 'selectedRewardIndex': -1},
                     {'questId': self.quest_id},
                     {'questId': self.quest_id, 'selectedRewardIndex': 0},
                     {'questId': self.quest_id, 'selectedRewardIndex': True},
                     {'questId': 'unowned', 'selectedRewardIndex': -1}):
            with self.subTest(body=body), self.assertRaises(ApiError):
                self.claim(**body)
            self.assertEqual(before, self.snapshot())
        self.complete(progress=False)
        before = self.snapshot()
        with self.assertRaises(ApiError):
            self.claim()
        self.assertEqual(before, self.snapshot())

    def test_award_failure_rolls_back_then_retry_succeeds(self):
        self.complete()
        before = self.snapshot()
        with patch('progression_service.award_profile_xp', side_effect=RuntimeError('award failure')):
            with self.assertRaises(RuntimeError):
                self.claim()
        self.assertEqual(before, self.snapshot())
        self.restart()
        self.assertEqual(before, self.snapshot())
        self.claim()
        self.assertEqual(self.snapshot()['items'][self.quest_id]['attributes']['quest_state'], 'Claimed')
