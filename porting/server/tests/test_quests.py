import tempfile
import unittest
from account_service import AccountService
from api_common import ApiError, Context, LOCAL_ACCOUNT_ID
from profile_service import ProfileService, ProfileTx, CommandContext, _grant_season12_rewards
from quest_service import QuestService
from season12_rewards import PAID

class QuestTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(); self.addCleanup(self.tmp.cleanup)
        self.ctx = Context(clock=lambda: 1700000000)
        self.accounts = AccountService(self.ctx, state_dir=self.tmp.name)
        self.restart()
    def restart(self):
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        self.profiles.quests = self.quests = QuestService(self.profiles)
    def snapshot(self):
        return self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')
    def login(self, body=None):
        return self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', body or {})
    def grant(self, rewards):
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        _grant_season12_rewards(ctx, ctx.tx('athena'), rewards, 0)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
    def item(self, template):
        return next((v for v in self.snapshot()['items'].values() if v['templateId'].casefold() == template.casefold()), None)
    def test_grants_login_progress_restart(self):
        q = 'Quest:quest_s12_styles1_q01_accolade_ar'
        self.login({'quest_state': 'Claimed'})
        self.assertIsNone(self.item(q))
        self.grant(PAID[1])
        self.assertEqual(self.item(q)['attributes']['quest_state'], 'Active')
        self.assertIsNotNone(self.item('ChallengeBundle:QuestBundle_S12_Donut_W1'))
        before = self.snapshot(); self.login()
        self.assertEqual(before['items'], self.snapshot()['items'])
        self.quests.progress(LOCAL_ACCOUNT_ID, 'accolade_assault_specialist', 4, event_id='one')
        self.assertFalse(self.quests.progress(LOCAL_ACCOUNT_ID, 'accolade_assault_specialist', 4, event_id='one'))
        self.assertIsNone(self.item('Token:AthenaToken_BRS12_HeroSelection_01'))
        self.quests.progress(LOCAL_ACCOUNT_ID, 'accolade_assault_specialist', 1, event_id='two')
        self.assertEqual(self.item(q)['attributes']['quest_state'], 'Claimed')
        self.assertIsNotNone(self.item('Token:AthenaToken_BRS12_HeroSelection_01'))
        self.quests.progress(LOCAL_ACCOUNT_ID, 'donut_w1_startquest', event_id='letter')
        self.assertEqual(self.item('Quest:Quest_S12_Donut_W1_B')['attributes']['quest_state'], 'Active')
        self.restart(); self.login()
        self.assertEqual(self.item(q)['attributes']['quest_state'], 'Claimed')
        self.assertFalse(self.quests.progress(LOCAL_ACCOUNT_ID, 'accolade_assault_specialist', 1, event_id='two'))
    def test_variants_and_rollback(self):
        self.grant(PAID[1])
        char = 'AthenaCharacter:CID_701_Athena_Commando_M_BananaAgent'
        self.assertEqual(self.item(char)['attributes']['variants'], [])
        self.grant(PAID[34]); self.grant(PAID[77])
        self.assertEqual(self.item(char)['attributes']['variants'], [{'channel':'Progressive','active':'Stage1','owned':['Stage1','Stage2','Stage3']}])
        self.assertIsNone(self.item('CosmeticVariantToken:VTID_560_BananaAgent_StyleB'))
        tx = ProfileTx(self.ctx, self.snapshot())
        char_id = next(i for i,v in tx.profile['items'].items() if v['templateId'] == char)
        tx.set_item_attr(char_id, 'variants', [{'channel':'Progressive','active':'Stage1','owned':'broken'}])
        self.profiles._store(LOCAL_ACCOUNT_ID, {'athena': tx})
        before = self.snapshot()
        with self.assertRaises(ApiError): self.grant(PAID[34])
        self.assertEqual(before, self.snapshot()); self.restart(); self.assertEqual(before, self.snapshot())
    def test_invalid_events(self):
        self.grant(PAID[1]); self.quests.progress(LOCAL_ACCOUNT_ID, 'donut_w1_startquest', event_id='letter')
        before = self.snapshot()
        for event,amount,event_id in [('made_up',1,'x'),('donut_w1_startquest',True,'x'),('donut_w1_startquest',2,'letter')]:
            with self.assertRaises(ApiError): self.quests.progress(LOCAL_ACCOUNT_ID,event,amount,event_id=event_id)
            self.assertEqual(before, self.snapshot())
