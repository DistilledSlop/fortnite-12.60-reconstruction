"""TNTina selection uses cooked entitlement and native instance-ID batches."""
if __package__:
    from . import _bootstrap, test_faction_quest_selection as brutus_tests
else:
    import _bootstrap
    import test_faction_quest_selection as brutus_tests

from api_common import LOCAL_ACCOUNT_ID
from profile_service import CommandContext
from season12_tntina_faction_data import TOKEN
from season12_brutus_faction_data import TOKEN as BRUTUS_TOKEN


class TNTinaSelectionTests(brutus_tests.FactionSelectionTests):
    def earn(self):
        self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        self.quests._award_token(ctx.tx('athena'), TOKEN, 18)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        self.ids = {side: next(item_id for item_id, item in self.snapshot()['items'].items()
                              if item['templateId'].casefold() == 'quest:ab_tntina_' + side)
                    for side in ('ego', 'alter')}

    def test_threshold_and_cross_faction_batches(self):
        self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})
        ctx = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        self.quests._award_token(ctx.tx('athena'), TOKEN, 17)
        self.quests._award_token(ctx.tx('athena'), BRUTUS_TOKEN, 18)
        self.profiles._store(LOCAL_ACCOUNT_ID, ctx.txs)
        brutus = next(i for i, v in self.snapshot()['items'].items()
                      if v['templateId'].casefold() == 'quest:quest_ab_skulldude_ego')
        self.reject_unchanged(['Quest:AB_tntina_ego'])
        self.earn()
        self.reject_unchanged([self.ids['ego'], brutus])
        self.select([brutus])
        self.select([self.ids['ego']])
        self.assertEqual(self.states(), {'ego': 'Active', 'alter': 'Inactive'})

    def test_completion_locks_other_branch(self):
        self.earn()
        self.select([self.ids['ego']])
        self.quests.progress(LOCAL_ACCOUNT_ID, 'AB_tntina_alter', 2, event_id='inactive')
        self.assertEqual(self.snapshot()['items'][self.ids['alter']]['attributes']['completion_ab_tntina_alter_obj'], 0)
        self.quests.progress(LOCAL_ACCOUNT_ID, 'AB_tntina_ego', 2, event_id='selected')
        self.assertEqual(self.states(), {'ego': 'Claimed', 'alter': 'Inactive'})
        self.reject_unchanged([self.ids['alter']])
        self.reject_unchanged([self.ids['ego'], self.ids['alter']])
