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


class ProgressModelTests(unittest.TestCase):
    def test_material_work_has_explicit_remaining_weight_and_no_gates_pass(self):
        import json
        model = json.loads(Path(__file__).with_name('components.json').read_text())
        leaves = list(progress.walk_spec(model['tree']))
        material = [leaf for leaf in leaves if leaf['id'].startswith('ui.material_')]
        self.assertEqual(sum(leaf['weight'] for leaf in material), 20)
        self.assertEqual(len(leaves), 104)
        self.assertEqual(sum(leaf['weight'] for leaf in leaves), 1855)
        self.assertEqual(sum(leaf['weight'] for leaf in material if leaf['measure']['kind'] == 'ledger'), 20)
        integration = next(leaf for leaf in material if leaf['id'] == 'ui.material_integration')
        self.assertEqual(integration['weight'], 10)
        self.assertEqual(integration['measure'], {'kind': 'ledger', 'checks': [
            'material_powertoast_asset_ingress', 'material_menubutton_asset_ingress',
            'material_donut_asset_ingress', 'material_frontend_widget_integration',
            'material_original_visual_parity',
        ]})
        self.assertEqual(next(leaf for leaf in material if leaf['id'] == 'ui.material_locker_ingress')['measure']['checks'], ['material_locker_asset_ingress'])
        self.assertFalse(any(m.get('done') for m in model['milestones']))
        self.assertFalse(any('ceiling' in leaf for leaf in leaves))

    def test_weighted_coverage_cannot_claim_unfinished_implementation(self):
        class Context:
            files = {'some/tracked/file': ''}
            replay = False
            used_local = False
            def reset_leaf(self): pass
        coverage = {'id': 'coverage', 'name': 'Coverage', 'weight': 2,
                    'measure': {'kind': 'fraction', 'done': 1, 'total': 2}}
        remaining = {'id': 'remaining', 'name': 'Remaining', 'weight': 8,
                     'measure': {'kind': 'manual', 'pct': 0}}
        # Supply the same total lookup normally provided by Ctx.
        ctx = Context()
        ctx.total_value = lambda spec: (spec['total'], 'test')
        result = progress.evaluate({'id': 'group', 'name': 'Group', 'children': [coverage, remaining]}, ctx)
        self.assertEqual(result['pct'], 10)


if __name__ == '__main__':
    unittest.main()
