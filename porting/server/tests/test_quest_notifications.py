"""Quest notification acknowledgement is persisted atomically."""
if __package__:
    from . import _bootstrap
else:
    import _bootstrap

import tempfile
import unittest

from api_common import ApiError, LOCAL_ACCOUNT_ID
from backend import Backend


class QuestNotificationTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.restart()
        self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'ClientQuestLogin', {})
        self.before = self.snapshot()
        self.quest_ids = [i for i, v in self.before['items'].items()
                          if v['templateId'].startswith('Quest:')]
        self.nonquest_id = next(i for i, v in self.before['items'].items()
                                if not v['templateId'].startswith('Quest:'))
        self.assertTrue(self.quest_ids)

    def restart(self):
        self.profiles = Backend(state_dir=self.directory.name).profiles

    def snapshot(self):
        return self.profiles.profile(LOCAL_ACCOUNT_ID, 'athena')

    def send(self, body):
        return self.profiles.run(LOCAL_ACCOUNT_ID, 'athena', 'MarkNewQuestNotificationSent', body)

    def test_persists_and_is_idempotent_without_changing_progress_or_rewards(self):
        quest_id = self.quest_ids[0]
        self.send({'itemIds': [quest_id, quest_id]})
        after = self.snapshot()
        expected = self.before['items'].copy()
        expected[quest_id] = dict(expected[quest_id], attributes={
            **expected[quest_id]['attributes'], 'sent_new_notification': True})
        self.assertEqual(after['items'], expected)
        self.assertEqual(after['stats'], self.before['stats'])
        self.restart()
        self.assertEqual(self.snapshot(), after)
        self.send({'itemIds': [quest_id]})
        again = self.snapshot()
        self.assertEqual(again['items'], after['items'])
        self.assertEqual(again['stats'], after['stats'])
        self.assertEqual(again['rvn'], after['rvn'])

    def test_invalid_batch_rolls_back_including_across_restart(self):
        quest_id = self.quest_ids[0]
        for body in ({}, {'itemIds': None}, {'itemIds': 'x'},
                     {'itemIds': [quest_id, 1]}, {'itemIds': [quest_id, '']},
                     {'itemIds': [quest_id, 'unowned']},
                     {'itemIds': [quest_id, self.nonquest_id]}):
            with self.subTest(body=body), self.assertRaises(ApiError):
                self.send(body)
            self.assertEqual(self.snapshot(), self.before)
        self.restart()
        self.assertEqual(self.snapshot(), self.before)


if __name__ == '__main__':
    unittest.main()
