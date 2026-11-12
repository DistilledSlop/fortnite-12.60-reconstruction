"""Maya's all-quests reward is transactional and repairable exactly once."""
if __package__:
    from . import _bootstrap
else:
    import _bootstrap

import tempfile
import unittest
from unittest.mock import patch

from account_service import AccountService
from api_common import ApiError, Context, LOCAL_ACCOUNT_ID
from profile_service import ProfileService, ProfileTx, CommandContext, _grant_season12_rewards
from quest_service import QuestService
from season12_maya_data import MAYA_QUESTS
from season12_rewards import PAID

START = 1700000000
TOKEN = 'token:athenatoken_brs12_heroselection_10'
RECEIPT = 'local_maya_bundle_reward_received'


class MayaBundleRewardTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.ctx = Context(clock=lambda: START + 60 * 86400)
        self.accounts = AccountService(self.ctx, state_dir=self.tmp.name)
        self.restart()
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        _grant_season12_rewards(ctx, ctx.tx('athena'), PAID[1], 0)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.login()

    def restart(self):
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        self.quests = self.profiles.quests = QuestService(self.profiles, {'EventFlag.LobbySeason12': START})

    def login(self):
        self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})

    def snapshot(self):
        return self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')

    def tokens(self, profile=None):
        return [(i, v) for i, v in (profile or self.snapshot())['items'].items()
                if v['templateId'].casefold() == TOKEN]

    def progress(self, index):
        name, definition = list(MAYA_QUESTS.items())[index]
        _, event, target = definition['objectives'][0]
        return self.quests.progress(LOCAL_ACCOUNT_ID, event, target, event_id='maya-' + name)

    def complete_eight(self):
        for i in range(8):
            self.progress(i)

    def test_ninth_reward_retry_and_consumption(self):
        self.complete_eight()
        self.assertEqual(self.tokens(), [])
        self.progress(8)
        self.assertEqual([v['quantity'] for _, v in self.tokens()], [1])
        self.assertTrue(any(v['templateId'].casefold() == 'token:athenatoken_brs12_heroselection_09'
                            for v in self.snapshot()['items'].values()))
        self.assertIs(self.snapshot()['stats']['attributes'][RECEIPT], True)
        self.restart()
        self.assertFalse(self.progress(8))
        tx = ProfileTx(self.ctx, self.snapshot())
        tx.remove_item(self.tokens()[0][0])
        self.profiles._store(LOCAL_ACCOUNT_ID, {'athena': tx})
        self.restart()
        self.login()
        self.assertEqual(self.tokens(), [])

    def test_old_completed_profile_repair_and_counter_is_insufficient(self):
        self.complete_eight()
        tx = ProfileTx(self.ctx, self.snapshot())
        bundle_id = next(i for i, v in tx.profile['items'].items() if
                         v['templateId'].casefold() == 'challengebundle:questbundle_styles_paperdoll')
        tx.set_item_attr(bundle_id, 'num_quests_completed', 9)
        self.profiles._store(LOCAL_ACCOUNT_ID, {'athena': tx})
        self.login()
        self.assertEqual(self.tokens(), [])
        # Simulate a legacy completion with all quest instances but no receipt.
        tx = ProfileTx(self.ctx, self.snapshot())
        name, definition = list(MAYA_QUESTS.items())[8]
        quest_id = next(i for i, v in tx.profile['items'].items() if v['templateId'].casefold() == ('quest:' + name).casefold())
        tx.set_item_attr(quest_id, 'quest_state', 'Claimed')
        objective, _, target = definition['objectives'][0]
        tx.set_item_attr(quest_id, 'completion_' + objective.lower(), target)
        self.profiles._store(LOCAL_ACCOUNT_ID, {'athena': tx})
        self.login()
        self.assertEqual([v['quantity'] for _, v in self.tokens()], [1])
        self.restart()
        self.login()
        self.assertEqual([v['quantity'] for _, v in self.tokens()], [1])

    def test_failure_rolls_back_ninth_and_login_rejects_bad_receipt(self):
        self.complete_eight()
        before = self.snapshot()
        original = self.quests._award_token
        def fail_final(tx, reward, quantity):
            if reward.casefold().endswith('_10'):
                raise RuntimeError('injected reward failure')
            return original(tx, reward, quantity)
        with patch.object(self.quests, '_award_token', side_effect=fail_final):
            with self.assertRaises(RuntimeError):
                self.progress(8)
        self.assertEqual(before, self.snapshot())
        self.restart()
        self.assertEqual(before, self.snapshot())
        self.progress(8)
        tx = ProfileTx(self.ctx, self.snapshot())
        tx.set_stat(RECEIPT, 'true')
        self.profiles._store(LOCAL_ACCOUNT_ID, {'athena': tx})
        before = self.snapshot()
        with self.assertRaises(ApiError):
            self.login()
        self.assertEqual(before, self.snapshot())


if __name__ == '__main__':
    unittest.main()
