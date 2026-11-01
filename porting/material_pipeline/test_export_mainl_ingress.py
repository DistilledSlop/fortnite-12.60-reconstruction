#!/usr/bin/env python3
"""Check MainL's source-override retention and graph-use distinction."""
import copy
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('mainl_exporter', Path(__file__).with_name('export_mainl_ingress.py'))
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


def info(name, association=2, index=-1):
    return dict(name=dict(name=name, number=0), association=association, index=index)


def row(name, value):
    return dict(parameter=name, parameterInfo=info(name), value=value)


class MainLIngressTests(unittest.TestCase):
    def test_retains_unused_original_values_without_affecting_graph(self):
        instance = dict(textures=[], scalars=[row('Used', 0.6), row('OptimizedOut', 1.5)], vectors=[])
        original = copy.deepcopy(instance)
        all_values, used, unused = module.partition_owner_expectations(instance, {'Used': ('scalar', info('Used'))})
        self.assertEqual([x['parameter']['name']['name'] for x in all_values], ['Used', 'OptimizedOut'])
        self.assertEqual([x['parameter']['name']['name'] for x in used], ['Used'])
        self.assertEqual([x['parameter']['name']['name'] for x in unused], ['OptimizedOut'])
        self.assertEqual(instance, original)

    def test_rejects_conflicting_or_duplicate_identity(self):
        instance = dict(textures=[], scalars=[row('P', 1)], vectors=[])
        with self.assertRaisesRegex(ValueError, 'conflicts'):
            module.partition_owner_expectations(instance, {'P': ('scalar', info('P', index=3))})
        instance['scalars'].append(row('P', 2))
        with self.assertRaisesRegex(ValueError, 'duplicate'):
            module.partition_owner_expectations(instance, {})

    def test_rejects_texture_and_missing_parameter_identity(self):
        instance = dict(textures=[{}], scalars=[], vectors=[])
        with self.assertRaisesRegex(ValueError, 'textures'):
            module.partition_owner_expectations(instance, {})
        instance = dict(textures=[], scalars=[dict(parameter='P', parameterInfo=None, value=0)], vectors=[])
        with self.assertRaisesRegex(ValueError, 'full identity'):
            module.partition_owner_expectations(instance, {})


if __name__ == '__main__':
    unittest.main()
