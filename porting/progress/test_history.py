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
    def test_event_registration_scope_preserves_lifecycle_budget(self):
        import json
        folder = Path(__file__).parent
        model = json.loads((folder / 'components.json').read_text())
        targets = json.loads((folder / 'frontend_event_targets.json').read_text())
        ledger = json.loads((folder / 'evidence.json').read_text())['checks']
        leaves = {leaf['id']: leaf for leaf in progress.walk_spec(model['tree'])}
        events = leaves['ui.frontend_event_registrations']
        self.assertEqual(events['weight'] + leaves['ui.lifecycle']['weight'] + leaves['ui.context_source_stage']['weight'] + leaves['ui.context_runtime_stage']['weight'], 15)
        self.assertEqual(leaves['ui.lifecycle']['measure'], dict(kind='manual', pct=0))
        self.assertEqual(len(targets['targets']), 41)
        self.assertEqual(len({row['path'] for row in targets['targets']}), 41)
        self.assertEqual(events['measure']['checks'], [row['check_id'] for row in targets['targets']])
        excluded = set(targets['excluded_native_behavior_targets'])
        for target in targets['targets']:
            self.assertNotIn(target['path'], excluded)
            self.assertFalse(int(target['flags'], 16) & 0x400)
            check = ledger[target['check_id']]
            if check['result'] == 'pass':
                self.assertEqual(check['original_signature']['path'], target['path'])
                self.assertEqual(set(check['compiled']), {'client', 'server'})
                for proof in check['compiled'].values():
                    self.assertEqual(proof['flags'], target['flags'])
                    self.assertEqual(proof['property_record_count'], target['serialized_property_count'])
        self.assertTrue(all(not gate['done'] for gate in model['milestones']))
        ctx = progress.Ctx({
            'porting/progress/frontend_event_targets.json': (folder / 'frontend_event_targets.json').read_text(),
            progress.EVIDENCE_REL: (folder / 'evidence.json').read_text(),
        }, False, [])
        units, done = progress._grid_event_registrations(events, ctx)
        self.assertEqual(len(units), 41)
        self.assertEqual(len(done), sum(ledger[cid]['result'] == 'pass' for cid in events['measure']['checks']))

    def test_material_work_has_explicit_remaining_weight_and_no_gates_pass(self):
        import json
        model = json.loads(Path(__file__).with_name('components.json').read_text())
        leaves = list(progress.walk_spec(model['tree']))
        material = [leaf for leaf in leaves if leaf['id'].startswith('ui.material_')]
        self.assertEqual(sum(leaf['weight'] for leaf in material), 20)
        self.assertEqual(len(leaves), 115)
        self.assertEqual(sum(leaf['weight'] for leaf in leaves), 1855)
        self.assertEqual(sum(leaf['weight'] for leaf in material if leaf['measure']['kind'] == 'ledger'), 20)
        by_id = {leaf['id']: leaf for leaf in material}
        self.assertEqual({key: by_id[key]['weight'] for key in (
            'ui.material_named_ingress', 'ui.material_frontend_closure',
            'ui.material_widget_integration', 'ui.material_visual_parity')}, {
            'ui.material_named_ingress': 6, 'ui.material_frontend_closure': 2,
            'ui.material_widget_integration': 1, 'ui.material_visual_parity': 1})
        self.assertEqual(by_id['ui.material_named_ingress']['measure']['checks'], [
            'material_powertoast_asset_ingress', 'material_menubutton_asset_ingress',
            'material_donut_asset_ingress', 'material_keybind_asset_ingress'])
        self.assertEqual(by_id['ui.material_widget_integration']['measure']['checks'], ['material_frontend_widget_integration'])
        self.assertEqual(by_id['ui.material_visual_parity']['measure']['checks'], ['material_original_visual_parity'])
        self.assertEqual(by_id['ui.material_locker_ingress']['measure']['checks'], ['material_locker_asset_ingress'])
        spec = importlib.util.spec_from_file_location("record_frontend_materials", Path(__file__).with_name("record_frontend_materials.py"))
        recorder = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(recorder)
        EXCLUDED, inventory_owners, owner_id = recorder.EXCLUDED, recorder.inventory_owners, recorder.owner_id
        inventory = Path(__file__).parent.parent / 'generated/frontend-probes/frontend-material-inventory/summary.json'
        # Generated inventory is local evidence and may be absent in a clean checkout.
        checks = by_id['ui.material_frontend_closure']['measure']['checks']
        ledger = json.loads(Path(__file__).with_name('evidence.json').read_text())['checks']
        owners = [ledger[check]['owner_path'] for check in checks]
        self.assertEqual(len(owners), 142)
        self.assertEqual(len(set(owners)), 142)
        self.assertEqual(checks, [owner_id(owner) for owner in owners])
        self.assertFalse(EXCLUDED & {owner.rsplit('.', 1)[1] for owner in owners})
        self.assertTrue(any(owner.endswith('.M-Border-MainL') for owner in owners))
        if inventory.exists():
            self.assertEqual(owners, inventory_owners(inventory))
        self.assertFalse(any(m.get('done') for m in model['milestones']))
        self.assertFalse(any('ceiling' in leaf for leaf in leaves))

    def test_commonui_runtime_is_separate_and_pending(self):
        import json
        folder = Path(__file__).parent
        model = json.loads((folder / 'components.json').read_text())
        leaves = {leaf['id']: leaf for leaf in progress.walk_spec(model['tree'])}
        remaining = leaves['ui.commonui_umg.implementation']
        runtime = leaves['ui.commonui_activation_runtime']
        button = leaves['ui.commonui_button_internal_runtime']
        panel = leaves['ui.commonui_panel_intro_runtime']
        self.assertEqual(remaining['weight'] + runtime['weight'] + button['weight'] + panel['weight'], 22)
        self.assertEqual(remaining['measure'], dict(kind='manual', pct=0))
        self.assertEqual(runtime['measure'], dict(kind='ledger', checks=['frontend_commonui_base_activation_runtime']))
        self.assertEqual(button['measure'], dict(kind='ledger', checks=['frontend_commonui_internal_button_runtime']))
        self.assertEqual(panel['measure'], dict(kind='ledger', checks=['frontend_commonui_panel_intro_runtime']))
        self.assertEqual(leaves['ui.panel_source_stage']['weight'], 1)
        self.assertEqual(leaves['ui.input_source_stage']['weight'], 1)
        evidence = json.loads((folder / 'evidence.json').read_text())
        evidence['checks'].pop('frontend_commonui_base_activation_runtime', None)
        evidence['checks'].pop('frontend_commonui_internal_button_runtime', None)
        evidence['checks'].pop('frontend_commonui_panel_intro_runtime', None)
        ctx = progress.Ctx({progress.EVIDENCE_REL: json.dumps(evidence)}, False, [])
        self.assertEqual(progress.evaluate(runtime, ctx)['pct'], 0)
        self.assertEqual(progress.evaluate(button, ctx)['pct'], 0)
        self.assertEqual(progress.evaluate(panel, ctx)['pct'], 0)
        self.assertTrue(all(not gate['done'] for gate in model['milestones']))

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
