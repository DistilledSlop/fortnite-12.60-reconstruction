"""Season 12 superlevel source gates, snapshots, and variant rewards."""
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import copy
import tempfile
import unittest
from unittest.mock import patch

from account_service import AccountService
from api_common import ApiError, Context, LOCAL_ACCOUNT_ID
from profile_service import CommandContext, ProfileService
from progression_service import ProgressionService
from quest_service import QuestService
from season12_progression_data import XP_TO_NEXT
from season12_superlevel_data import SUPERLEVEL_QUESTS
from superlevel_service import SUPERLEVEL_VARIANTS

BUNDLE = 'ChallengeBundle:MissionBundle_S12_Superlevel'
MIDAS = 'quest_style_superlevel_midas'


class SuperlevelTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.clock = Context(clock=lambda: 1700000000)
        self.accounts = AccountService(self.clock, state_dir=self.directory.name)
        self.restart()

    def restart(self):
        self.profiles = ProfileService(self.clock, self.accounts, None, state_dir=self.directory.name)
        self.quests = self.profiles.quests = QuestService(self.profiles)
        self.progression = ProgressionService(self.clock, self.accounts, self.profiles, state_dir=self.directory.name)

    def snapshot(self):
        return self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')

    def item(self, template):
        return next((v for v in self.snapshot()['items'].values() if v['templateId'].casefold() == template.casefold()), None)

    def change(self, action):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        action(ctx.tx('athena'))
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)

    def stats(self, **values):
        self.change(lambda tx: [tx.set_stat(k, v) for k, v in values.items()])

    def login(self):
        self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})

    def quest(self, name):
        return self.item('Quest:' + name)

    def test_free_schedule_and_paid_quest_gate(self):
        self.login()
        self.assertIsNotNone(self.item('ChallengeBundleSchedule:Season12_ABChallenges_Schedule'))
        self.assertIsNotNone(self.item(BUNDLE))
        self.assertTrue(all(self.quest(name) is None for name in SUPERLEVEL_QUESTS))
        self.stats(book_purchased=True)
        self.login()
        self.assertTrue(all(self.quest(name)['attributes']['quest_state'] == 'Active' for name in SUPERLEVEL_QUESTS))

    def test_existing_level_reconciles_but_book_level_does_not(self):
        self.stats(book_purchased=True, level=1, book_level=100)
        self.login()
        self.assertEqual(self.quest(MIDAS)['attributes']['completion_quest_style_superlevel_midas_obj'], 1)
        self.assertEqual(self.quest(MIDAS)['attributes']['quest_state'], 'Active')
        self.stats(level=140)
        self.login()
        self.assertEqual(self.quest(MIDAS)['attributes']['quest_state'], 'Claimed')
        self.assertEqual(self.quest('quest_style_superlevel_skulldude')['attributes']['quest_state'], 'Claimed')
        self.assertEqual(self.quest('quest_style_superlevel_buffcat')['attributes']['completion_quest_style_superlevel_buffcat_obj'], 140)

    def test_xp_crosses_level_100_at_book_cap(self):
        self.stats(book_purchased=True, level=99, book_level=100, xp=0, book_xp=0, accountLevel=99)
        self.login()
        self.progression.record_result('gold-level', {'accountId': LOCAL_ACCOUNT_ID, 'placement': 1, 'kills': 0,
            'xp': XP_TO_NEXT[98], 'bookXp': 0, 'endedAt': '2023-11-14T22:13:20Z'})
        self.assertEqual(self.snapshot()['stats']['attributes']['level'], 100)
        self.assertEqual(self.quest(MIDAS)['attributes']['quest_state'], 'Claimed')
        self.assertIsNotNone(self.item('CosmeticVariantToken:VTID_565_CatBurglar_StyleD'))

    def test_level_300_variants_pending_target_and_active_preserved(self):
        self.stats(book_purchased=True, level=300)
        token = 'cosmeticvarianttoken:vtid_573_tntina_styled'
        target, channel, stage, _ = SUPERLEVEL_VARIANTS[token]
        self.change(lambda tx: self.quests._grant(tx, target, {'variants': [
            {'channel': channel, 'active': 'Stage3', 'owned': ['Stage1', 'Stage3']},
            {'channel': 'Material', 'active': 'Ghost', 'owned': ['Ghost', 'Shadow']}]}))
        self.login()
        self.assertTrue(all(self.quest(name)['attributes']['quest_state'] == 'Claimed' for name in SUPERLEVEL_QUESTS))
        variants = self.item(target)['attributes']['variants']
        self.assertEqual(variants[0], {'channel': channel, 'active': 'Stage3', 'owned': ['Stage1', 'Stage3', stage]})
        self.assertEqual(variants[1]['active'], 'Ghost')
        self.assertEqual(stage, 'Stage7')
        pending = 'cosmeticvarianttoken:vtid_575_bananaagent_backbling_styleb'
        self.assertIsNotNone(self.item(pending))
        pending_target, _, pending_stage, _ = SUPERLEVEL_VARIANTS[pending]
        self.assertEqual(pending_stage, 'Stage2')
        self.change(lambda tx: self.quests._grant(tx, pending_target, {'variants': []}))
        self.login()
        self.assertIsNone(self.item(pending))
        self.assertEqual(self.item(pending_target)['attributes']['variants'][0]['owned'], ['Stage1', 'Stage2'])
        self.login()
        self.assertEqual(self.item(pending_target)['attributes']['variants'][0]['owned'], ['Stage1', 'Stage2'])
        self.assertEqual(sum(1 for n in SUPERLEVEL_VARIANTS if self.item(n)), 10)

    def test_untrusted_level_event_rejected(self):
        self.stats(book_purchased=True)
        self.login()
        before = self.snapshot()
        for event in ('HasLeveledUpAthenaSeason', 'EFortQuestObjectiveItemEvent::HasLeveledUpAthenaSeason',
                      'objective:quest_style_superlevel_midas_obj'):
            with self.subTest(event=event), self.assertRaises(ApiError):
                self.quests.progress(LOCAL_ACCOUNT_ID, event, event_id=event)
            self.assertEqual(before, self.snapshot())

    def test_completed_legacy_and_malformed_progress(self):
        self.stats(book_purchased=True, level=99)
        self.login()
        def legacy(tx):
            item_id = next(i for i, v in tx.profile['items'].items()
                           if v['templateId'].casefold() == 'quest:' + MIDAS)
            tx.set_item_attr(item_id, 'quest_state', 'Completed')
            tx.set_item_attr(item_id, 'completion_quest_style_superlevel_midas_obj', 100)
        self.change(legacy)
        self.login()
        self.assertEqual(self.quest(MIDAS)['attributes']['quest_state'], 'Completed')
        self.stats(level=100)
        self.login()
        self.assertEqual(self.quest(MIDAS)['attributes']['quest_state'], 'Claimed')
        def corrupt(tx):
            item_id = next(i for i, v in tx.profile['items'].items()
                           if v['templateId'].casefold() == 'quest:quest_style_superlevel_buffcat')
            tx.set_item_attr(item_id, 'completion_quest_style_superlevel_buffcat_obj', True)
        self.change(corrupt)
        before = self.snapshot()
        with self.assertRaises(ApiError):
            self.login()
        self.assertEqual(before, self.snapshot())

    def test_retry_restart_and_atomic_failure(self):
        self.stats(book_purchased=True, level=99, book_level=100, xp=0, book_xp=0, accountLevel=99)
        self.login()
        before = copy.deepcopy(self.snapshot())
        original = self.quests._grant
        def fail(tx, template, attrs):
            if template.casefold() == 'cosmeticvarianttoken:vtid_565_catburglar_styled':
                raise RuntimeError('reward failure')
            return original(tx, template, attrs)
        payload = {'accountId': LOCAL_ACCOUNT_ID, 'placement': 1, 'kills': 0,
                   'xp': XP_TO_NEXT[98], 'bookXp': 0, 'endedAt': '2023-11-14T22:13:20Z'}
        with patch.object(self.quests, '_grant', side_effect=fail), self.assertRaises(RuntimeError):
            self.progression.record_result('atomic', payload)
        self.assertEqual(before, self.snapshot())
        self.progression.record_result('atomic', payload)
        self.restart()
        self.login()
        self.assertEqual(self.quest(MIDAS)['attributes']['quest_state'], 'Claimed')
        self.assertEqual(sum(v['templateId'].casefold() == 'cosmeticvarianttoken:vtid_565_catburglar_styled'
                             for v in self.snapshot()['items'].values()), 1)


if __name__ == '__main__':
    unittest.main()
