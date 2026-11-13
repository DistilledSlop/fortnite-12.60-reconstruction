"""Faction mission activation honors the native purchase and season-level gates."""
if __package__:
    from . import _bootstrap
else:
    import _bootstrap

import tempfile
import unittest

from account_service import AccountService
from api_common import ApiError, Context, LOCAL_ACCOUNT_ID
from profile_service import CommandContext, ProfileService
from quest_service import QuestService


# Independently recovered SpatialHenchmenData / AthenaSeason12 schedule facts.
CASES = (
    ('Brutus', 20, 'AthenaBattlePass_SkullDudeBundle_Token', 'quest_AB_skulldude_ego'),
    ('TNTina', 40, 'AthenaBattlePass_TNTinaBundle_Token', 'AB_tntina_ego'),
    ('Meowscles', 60, 'AthenaBattlePass_MeowsclesBundle_Token', 'quest_ab_buffcat_ego'),
    ('Skye', 80, 'AthenaBattlePass_AdventureGirlBundle_Token', 'quest_AB_photographer_ego'),
    ('Midas', 100, 'AthenaBattlePass_MidasBundle_Token', 'quest_ab_midas_ego'),
)


class FactionPrerequisiteTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        ctx = Context(clock=lambda: 1700000000)
        accounts = AccountService(ctx, state_dir=tmp.name)
        self.profiles = ProfileService(ctx, accounts, None, state_dir=tmp.name)
        quests = self.profiles.quests = QuestService(self.profiles)
        self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})
        command = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        for _, _, token, _ in CASES:
            quests._award_token(command.tx('athena'), token, 18)
        self.profiles._store(LOCAL_ACCOUNT_ID, command.txs)

    def snapshot(self):
        return self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')

    def edit(self, **stats):
        command = CommandContext(self.profiles, LOCAL_ACCOUNT_ID, 'athena', {})
        for name, value in stats.items():
            tx = command.tx('athena')
            old = tx.profile['stats']['attributes'].get(name)
            if old == value and type(old) is not type(value):
                # Seed deliberately malformed stored values despite Python's
                # bool/int equality used by the normal change tracker.
                tx.set_stat(name, None)
            tx.set_stat(name, value)
        self.profiles._store(LOCAL_ACCOUNT_ID, command.txs)

    def select(self, quest):
        item_id = next(i for i, item in self.snapshot()['items'].items()
                       if item['templateId'].casefold() == ('Quest:' + quest).casefold())
        return self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ToggleQuestActiveState',
                                 {'questIds': [item_id]})

    def reject_unchanged(self, quest):
        before = self.snapshot()
        with self.assertRaises(ApiError):
            self.select(quest)
        self.assertEqual(before, self.snapshot())

    def test_all_five_level_boundaries_use_season_level(self):
        for name, required, _, quest in CASES:
            with self.subTest(faction=name):
                self.edit(book_purchased=True, level=required - 1, book_level=100)
                self.reject_unchanged(quest)
                # The native getter reads season level even when book_level is
                # lower. A high book_level must not bypass a low season level.
                self.edit(level=required, book_level=1)
                response = self.select(quest)
                self.assertTrue(response['profileChanges'])

    def test_paid_ownership_and_season_are_required_for_every_faction(self):
        for name, _, _, quest in CASES:
            for stats in ({'book_purchased': False}, {'book_purchased': 1},
                          {'book_purchased': 'true'}, {'season_num': 11}):
                with self.subTest(faction=name, stats=stats):
                    self.edit(book_purchased=True, level=100, season_num=12)
                    self.edit(**stats)
                    self.reject_unchanged(quest)

    def test_malformed_level_cannot_activate_an_earned_mission(self):
        quest = CASES[0][3]
        for level in (None, True, '100', 100.0, -1):
            with self.subTest(level=level):
                self.edit(book_purchased=True, level=level)
                self.reject_unchanged(quest)


if __name__ == '__main__':
    unittest.main()
