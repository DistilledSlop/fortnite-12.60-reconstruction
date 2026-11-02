"""Budget conservation and evidence separation for the reviewed effort model."""
import json
from pathlib import Path
import unittest

HERE = Path(__file__).resolve().parent

def leaves(node):
    if isinstance(node, dict):
        if 'measure' in node:
            yield node
        for key, value in node.items():
            if key not in ('measure', 'rationale'):
                yield from leaves(value)
    elif isinstance(node, list):
        for value in node:
            yield from leaves(value)

class EffortReviewModel(unittest.TestCase):
    def setUp(self):
        self.model = json.loads((HERE / 'components.json').read_text())
        self.rows = list(leaves(self.model['tree']))
        self.by_id = {row['id']: row for row in self.rows}

    def test_total_and_explicit_missing_scope(self):
        self.assertEqual(len(self.rows), len(self.by_id))
        self.assertEqual(len(self.rows), 132)
        self.assertEqual(sum(row['weight'] for row in self.rows), 2019)
        self.assertEqual(sum(row['weight'] for row in self.rows if row['id'].startswith('ui.')), 199)
        scopes = ['visuals.world_materials_particles', 'assets.animation_cosmetics',
                  'gameplay.engine_system_integration', 'net.full_match_scale', 'gameplay.replay_spectating']
        self.assertEqual(sum(self.by_id[name]['weight'] for name in scopes), 120)
        for name in scopes:
            self.assertEqual(self.by_id[name]['measure'], {'kind': 'manual', 'pct': 0})
        self.assertTrue(all(not gate['done'] for gate in self.model['milestones']))

    def test_census_split_preserves_declaration_credit_and_exposes_remainder(self):
        total = 0
        for name in ('framework', 'building', 'inventory', 'abilities', 'ai_nav', 'world_volumes', 'other'):
            declaration = self.by_id['gameplay.' + name]
            remainder = self.by_id['gameplay.' + name + '.implementation']
            allocation = declaration['weight'] + remainder['weight']
            total += allocation
            self.assertEqual(declaration['measure']['stub_credit'], 1)
            self.assertAlmostEqual(declaration['weight'], allocation * 0.2)
            self.assertEqual(remainder['measure'], {'kind': 'manual', 'pct': 0})
            for coverage in (0, 0.5, 1):
                self.assertAlmostEqual(declaration['weight'] * coverage, allocation * coverage * 0.2)
        self.assertEqual(total, 265)

    def test_foundations_are_specific_verified_ledgers_not_whole_ui(self):
        ledger = json.loads((HERE / 'evidence.json').read_text())['checks']
        names = ['ui.commonui_button_foundation_runtime', 'ui.commonui_input_foundation_runtime', 'ui.startup_selection_runtime']
        self.assertEqual(sum(self.by_id[name]['weight'] for name in names), 3)
        checks = []
        for name in names:
            row = self.by_id[name]
            self.assertEqual(row['measure']['kind'], 'ledger')
            checks.extend(row['measure']['checks'])
        self.assertEqual(len(checks), 4)
        self.assertEqual(len(set(checks)), 4)
        for check in checks:
            self.assertEqual(ledger[check]['result'], 'pass')
        self.assertEqual(self.by_id['ui.commonui_umg.implementation']['measure'], {'kind': 'manual', 'pct': 0})
        self.assertEqual(self.by_id['ui.lifecycle']['measure'], {'kind': 'manual', 'pct': 0})

if __name__ == '__main__':
    unittest.main()
