import copy
import importlib.util
from pathlib import Path
import sys
import unittest
from unittest import mock

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[2]
spec = importlib.util.spec_from_file_location('commited_policy', HERE / 'policy.py')
p = importlib.util.module_from_spec(spec); spec.loader.exec_module(p)
sys.path.insert(0, str(REPO / 'porting'))
import gen_native_classes as g

class Tests(unittest.TestCase):
    def setUp(self):
        tables = g.TypeTables(str(REPO / 'porting/generated/reflection'))
        self.gate = g.TypeGate(tables, g.EngineTypeIndex('/dev/null'), g.Resolver({}))
        self.info = self.gate.info(p.ROOT)

    def test_real_metadata_gate_and_seeded_render(self):
        with mock.patch.object(g._action_commited_policy, 'build_delegate', return_value=None):
            self.assertEqual(self.gate.reason(self.info), 'return, out or non-const reference parameter')
        self.gate.models.clear(); self.gate.reasons.clear()
        original = self.gate.build_delegate
        self.gate.build_delegate = lambda info: p.build_delegate(info) or original(info)
        seeds = {}; p.add_roots(self.gate, seeds)
        self.assertEqual(seeds, {p.ROOT: {'Delegate'}})
        model = self.gate.model(self.info)
        text = g.render_delegate_header(model, 'single', 'test')
        self.assertIn('UDELEGATE()\nDECLARE_DYNAMIC_DELEGATE_OneParam(FCommonActionCommited, bool&, bPassThrough);', text)
        self.assertEqual(model['used'], {})

    def test_drift_fails_closed_before_seeding(self):
        for field in ('function_flags', 'outer_fn', 'params', 'construct_fns'):
            with self.subTest(field=field):
                info = copy.copy(self.info); info.rec = copy.deepcopy(info.rec)
                info.rec[field] = None
                with self.assertRaises(ValueError): p.build_delegate(info)
        seeds = {}
        with mock.patch.object(g._action_commited_policy, 'build_delegate', return_value=None):
            with self.assertRaises(ValueError): p.add_roots(self.gate, seeds)
        self.assertEqual(seeds, {})

if __name__ == '__main__': unittest.main()
