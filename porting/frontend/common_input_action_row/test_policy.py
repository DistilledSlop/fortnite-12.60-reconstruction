import copy
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('policy', Path(__file__).with_name('policy.py'))
p = importlib.util.module_from_spec(spec)
spec.loader.exec_module(p)

class Info:
    def __init__(self, fn, kind, cpp, record):
        self.fn, self.kind, self.cpp_name = fn, kind, cpp
        self.package = '/Script/CommonUI'
        self.rec = copy.deepcopy(p.EXPECTED[record])

class Gate:
    def __init__(self):
        self.types = {fn: Info(fn, kind, cpp, record) for fn, kind, cpp, record in p.TYPES}
        self.failure = {}
    def info(self, fn): return self.types.get(fn)
    def reason(self, info): return self.failure.get(info.fn)

class PolicyTests(unittest.TestCase):
    def test_seed_and_defaults(self):
        g = Gate(); seeds = {'existing': {'Enum'}}
        p.add_roots(g, seeds)
        self.assertEqual(seeds, {'existing': {'Enum'}, p.ROOT: {'Struct'}})
        m = p.apply_struct(g.info(p.INFO), {})
        text = '\n'.join(p.declarations(m))
        for expected in ('OverrrideState(EInputActionState::Enabled)', 'bActionRequiresHold(false)',
                         'HoldTime(0.5f)', 'OverrideBrush.DrawAs = ESlateBrushDrawType::NoDrawType;'):
            self.assertIn(expected, text)
        self.assertEqual(p.declarations(p.apply_struct(g.info(p.ROOT), {})), [])
    def test_metadata_and_missing_dependency_fail_before_seeding(self):
        for fn, _, _, _ in p.TYPES:
            for mutation in ('missing', 'record', 'package', 'unresolved'):
                with self.subTest(fn=fn, mutation=mutation):
                    g = Gate(); seeds = {}
                    if mutation == 'missing': del g.types[fn]
                    elif mutation == 'record': g.types[fn].rec['name'] += 'changed'
                    elif mutation == 'package': g.types[fn].package = '/Script/Other'
                    else: g.failure[fn] = 'unresolved'
                    with self.assertRaises(ValueError): p.add_roots(g, seeds)
                    self.assertEqual(seeds, {})
    def test_constructor_rejects_identity_drift(self):
        g = Gate(); g.info(p.INFO).rec['size'] += 1
        with self.assertRaises(ValueError): p.apply_struct(g.info(p.INFO), {})

if __name__ == '__main__': unittest.main()
