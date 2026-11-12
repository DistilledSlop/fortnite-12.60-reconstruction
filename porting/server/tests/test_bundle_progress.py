"""The client's displayed bundle progress is distinct from reward counters."""
if __package__:
    from . import _bootstrap, test_weekly_quests as weekly
else:
    import _bootstrap
    import test_weekly_quests as weekly

import unittest

from profile_service import ProfileService, ProfileTx
from quest_service import QuestService


class BundleProgressTests(unittest.TestCase):
    setUp = weekly.WeeklyQuestTests.setUp
    items = weekly.WeeklyQuestTests.items
    get = weekly.WeeklyQuestTests.get
    login = weekly.WeeklyQuestTests.login
    grant_schedule = weekly.WeeklyQuestTests.grant_schedule
    progress = weekly.WeeklyQuestTests.progress

    def test_claim_retry_and_login_repair_do_not_repeat_rewards(self):
        self.grant_schedule()
        self.login()
        bundle = 'ChallengeBundle:MissionBundle_S12_Week_01'
        self.assertEqual(self.get(bundle)['attributes']['num_progress_quests_completed'], 0)
        self.progress('damage_athena_henchmen', 'henchmen', 2000)
        attrs = self.get(bundle)['attributes']
        self.assertEqual((attrs['num_quests_completed'], attrs['num_progress_quests_completed']), (1, 1))
        self.assertFalse(self.progress('damage_athena_henchmen', 'henchmen', 2000))
        profile = self.profiles.profile(weekly.LOCAL_ACCOUNT_ID, 'athena')
        bundle_id = next(i for i, v in profile['items'].items() if v['templateId'] == bundle)
        tx = ProfileTx(self.ctx, profile)
        # Older profiles have claimed quests but no maintained display progress.
        tx.set_item_attr(bundle_id, 'num_progress_quests_completed', 0)
        self.profiles._store(weekly.LOCAL_ACCOUNT_ID, {'athena': tx})
        self.profiles = ProfileService(self.ctx, self.accounts, None, state_dir=self.tmp.name)
        self.quests = self.profiles.quests = QuestService(self.profiles, {weekly.TAG: weekly.START})
        self.login()
        self.assertEqual(self.get(bundle)['attributes']['num_progress_quests_completed'], 1)
        self.assertEqual(self.get('Token:AthenaBattlePass_WeeklyChallenge_Token')['quantity'], 1)
        self.assertEqual(self.get('Token:AthenaBattlePass_SkullDudeBundle_Token')['quantity'], 1)


if __name__ == '__main__':
    unittest.main()
