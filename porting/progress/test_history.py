#!/usr/bin/env python3
"""Protect commit history when hooks are missed, repeated or finish out of order."""
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('progress', Path(__file__).with_name('progress.py'))
progress = importlib.util.module_from_spec(spec)
spec.loader.exec_module(progress)


class HistoryRepairTests(unittest.TestCase):
    def test_missing_point_replayed_without_changing_recorded_measurements(self):
        first = dict(commit='a', overall_pct=7, seeded=False)
        prior_head = dict(commit='c', overall_pct=9, seeded=False)
        new_head = dict(commit='d', overall_pct=12, seeded=False)
        calls = []
        def replay(sha):
            calls.append(sha)
            return dict(commit=sha, overall_pct=99, seeded=True)
        repaired = progress.reconcile_history([prior_head, first, prior_head],
                                              ['a', 'b', 'c', 'd'], new_head, replay)
        self.assertEqual(calls, ['b'])
        self.assertEqual([row['commit'] for row in repaired], ['a', 'b', 'c', 'd'])
        self.assertIs(repaired[0], first)
        self.assertIs(repaired[2], prior_head)
        self.assertIs(repaired[3], new_head)

    def test_repeated_head_refresh_keeps_one_current_point(self):
        current = dict(commit='a', overall_pct=12)
        repaired = progress.reconcile_history([dict(commit='a', overall_pct=1)], ['a'], current,
                                              lambda sha: self.fail('recorded commit replayed'))
        self.assertEqual(repaired, [current])


if __name__ == '__main__':
    unittest.main()
