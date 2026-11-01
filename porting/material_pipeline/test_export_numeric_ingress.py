#!/usr/bin/env python3
"""Keep real-owner inheritance separate from diagnostic proxy overrides."""
import copy
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('numeric_exporter', Path(__file__).with_name('export_numeric_ingress.py'))
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


def identity(name):
    return dict(name=dict(name=name, number=0), association=2, index=-1)


def sample():
    defaults = dict(Rotation=0.0, Scale=1.0, RGBA0=[0.2, 0.3, 0.4, 0.5], RGBA1=[1.0, 1.0, 1.0, 1.0])
    parameters = {name: ('vector' if isinstance(value, list) else 'scalar', identity(name))
                  for name, value in defaults.items()}
    arrays = {name: dict(count=0, expressions=[]) for name in module.bundle.decoder.ARRAYS}
    for name, value in defaults.items():
        kind, parameter = parameters[name]
        arrays[kind]['expressions'].append(dict(type='FMaterialUniformExpression' + kind.title() + 'Parameter',
                                               parameter=parameter, default=value))
        arrays[kind]['count'] += 1
    return dict(uniform_expression_arrays=arrays, virtual_texture_stacks=dict(count=0)), parameters


class NumericIngressTests(unittest.TestCase):
    def test_instance_defaults_and_proxy_mutations_are_independent(self):
        material, parameters = sample()
        inherited = [dict(type='scalar', parameter=identity('Scale'), value=1.5),
                     dict(type='vector', parameter=identity('RGBA1'), value=[0, 0, 1, 1])]
        original = copy.deepcopy(inherited)
        owner = {case['name']: case for case in module.numeric_cases(material, parameters, inherited)}
        base = {case['name']: case for case in module.numeric_cases(material, parameters, [])}
        self.assertEqual(owner['defaults']['overrides'], [])
        self.assertEqual(owner['defaults']['numeric_values'][4:8], [0, 0, 1, 1])
        self.assertEqual(owner['defaults']['numeric_values'][9], 1.5)
        self.assertEqual(owner['Scale']['numeric_values'][9], 0.75)
        self.assertEqual(owner['Scale']['numeric_values'][4:8], [0, 0, 1, 1])
        self.assertEqual(base['defaults']['numeric_values'][9], 1.0)
        self.assertEqual(inherited, original)

    def test_unmatched_full_identities_leave_real_owner_values_unchanged(self):
        material, parameters = sample()
        inherited = [dict(type='scalar', parameter=identity('Rotation'), value=1.0)]
        cases = {case['name']: case for case in module.numeric_cases(material, parameters, inherited)}
        for name in ('unmatched_association', 'unmatched_index'):
            self.assertEqual(cases[name]['numeric_values'], cases['defaults']['numeric_values'])
            self.assertNotEqual(cases[name]['overrides'][0]['parameter'], identity('Rotation'))

    def test_only_explicit_supported_package_mounts_are_normalized(self):
        self.assertEqual(module.package_path('FortniteGame/Content/UI/M_Test'), '/Game/UI/M_Test')
        self.assertEqual(module.package_path('/Game/UI/M_Test'), '/Game/UI/M_Test')
        with self.assertRaisesRegex(ValueError, 'unsupported'):
            module.package_path('/Engine/Content/UI/M_Test')


if __name__ == '__main__':
    unittest.main()
