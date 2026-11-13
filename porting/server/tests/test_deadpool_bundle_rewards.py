if __package__:
    from . import test_deadpool_quests as deadpool
else:
    import test_deadpool_quests as deadpool
from profile_service import CommandContext
from api_common import LOCAL_ACCOUNT_ID
from unittest.mock import patch


TAG, START, DAY = deadpool.TAG, deadpool.START, deadpool.DAY


class DeadpoolBundleRewardTests(deadpool.DeadpoolQuestTests):
    def grant_bundle(self, week):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        self.quests._bundle(ctx.tx('athena'), 'QuestBundle_S12_Donut_' + week)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)

    def test_threshold_retry_and_restart(self):
        self.progress('donut_w1_startquest')
        self.assertIsNone(self.item('BannerToken', 'BannerToken_043_S12_Donut'))
        self.assertIsNone(self.item('Token', 'AthenaBattlePass_Donut_Challenge_Token'))
        self.progress('dont_thank_bus_driver')
        self.assertIsNotNone(self.item('BannerToken', 'BannerToken_043_S12_Donut'))
        token = self.item('Token', 'AthenaBattlePass_Donut_Challenge_Token')
        self.assertEqual(token['quantity'], 1)
        self.restart()
        self.login()
        self.assertFalse(self.progress('donut_w1_startquest'))
        self.assertEqual(self.item('Token', 'AthenaBattlePass_Donut_Challenge_Token')['quantity'], 1)

    def test_repair_old_claim_does_not_replay_token(self):
        with patch.object(self.quests, '_award_donut_bundle'):
            self.progress('donut_w1_startquest')
            self.progress('dont_thank_bus_driver')
        self.assertIsNone(self.item('BannerToken', 'BannerToken_043_S12_Donut'))
        self.restart()
        self.login()
        self.assertIsNotNone(self.item('BannerToken', 'BannerToken_043_S12_Donut'))
        self.assertIsNone(self.item('Token', 'AthenaBattlePass_Donut_Challenge_Token'))
        self.restart()
        self.login()
        self.assertIsNone(self.item('Token', 'AthenaBattlePass_Donut_Challenge_Token'))

    def test_incomplete_old_claim_is_not_repaired(self):
        with patch.object(self.quests, '_award_donut_bundle'):
            self.progress('donut_w1_startquest')
        self.restart()
        self.login()
        self.assertIsNone(self.item('BannerToken', 'BannerToken_043_S12_Donut'))
        self.progress('dont_thank_bus_driver')
        self.assertIsNotNone(self.item('BannerToken', 'BannerToken_043_S12_Donut'))
        self.assertEqual(self.item('Token', 'AthenaBattlePass_Donut_Challenge_Token')['quantity'], 1)

    def test_suppressed_w2_chain_waits_for_all_objectives(self):
        self.now = START + 11 * DAY
        self.restart({TAG: START})
        self.grant_bundle('W2')
        self.progress('donut_w2_startquest')
        self.progress('donut_chimi_1')
        self.progress('donut_chimi_2')
        self.assertIsNone(self.item('AthenaDance', 'SPID_201_DonutA'))
        self.progress('donut_chimi_3')
        self.assertIsNotNone(self.item('AthenaDance', 'SPID_201_DonutA'))
        self.assertEqual(self.item('Token', 'AthenaBattlePass_Donut_Challenge_Token')['quantity'], 1)

    def test_yacht_single_stage_and_repair_incomplete_w7(self):
        self.now = START + 46 * DAY
        self.restart({TAG: START})
        self.grant_bundle('YachtChallenge')
        self.grant_bundle('W7')
        with patch.object(self.quests, '_award_donut_bundle'):
            self.progress('donut_pistol_1')
            self.progress('donut_pistol_2')
        self.restart({TAG: START})
        self.login()
        self.assertIsNone(self.item('AthenaCharacter', 'CID_705_Athena_Commando_M_Donut'))
        self.progress('visit_namedpoi_spybase_yacht')
        self.assertIsNotNone(self.item('AthenaMusicPack', 'MusicPack_047_FreestylinClubRemix'))
        self.progress('donut_enter_porta_or_phone')
        self.assertIsNotNone(self.item('AthenaCharacter', 'CID_705_Athena_Commando_M_Donut'))

    def test_variant_waits_for_owned_target_and_preserves_active(self):
        self.now = START + 60 * DAY
        self.restart({TAG: START})
        self.grant_bundle('W8')
        self.grant_bundle('W9')
        self.progress('donut_w8_startquest')
        self.progress('donut_w9_startquest')
        self.assertIsNone(self.item('CosmeticVariantToken', 'VTID_587_Donut_StyleB'))
        self.progress('donut_dance_pool_party')
        self.progress('donut_salute_shorts')
        self.assertIsNotNone(self.item('CosmeticVariantToken', 'VTID_587_Donut_StyleB'))
        self.assertIsNotNone(self.item('CosmeticVariantToken', 'VTID_588_Donut_ColorB'))
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        ctx.tx('athena').add_item('deadpool', 'AthenaCharacter:CID_705_Athena_Commando_M_Donut', {'item_seen': True})
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.restart({TAG: START})
        self.login()
        attrs = self.item('AthenaCharacter', 'CID_705_Athena_Commando_M_Donut')['attributes']
        self.assertTrue(attrs['item_seen'])
        self.assertEqual(attrs['variants'], [
            {'channel': 'Parts', 'active': 'Stage1', 'owned': ['Stage1', 'Stage2']},
            {'channel': 'Material', 'active': 'Mat1', 'owned': ['Mat1', 'Mat2']},
        ])
        self.assertIsNone(self.item('CosmeticVariantToken', 'VTID_587_Donut_StyleB'))
        self.assertIsNone(self.item('CosmeticVariantToken', 'VTID_588_Donut_ColorB'))

    def test_reward_failure_is_atomic(self):
        self.progress('donut_w1_startquest')
        before = self.snapshot()
        with patch.object(self.quests, '_award_token', side_effect=RuntimeError('failure')):
            with self.assertRaises(RuntimeError):
                self.progress('dont_thank_bus_driver')
        self.assertEqual(before, self.snapshot())
        self.restart()
        self.assertTrue(self.progress('dont_thank_bus_driver'))
