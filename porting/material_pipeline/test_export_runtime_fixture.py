#!/usr/bin/env python3
"""Ensure flat runtime IR preserves graph values, parameter identity and slots."""
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('exporter', Path(__file__).with_name('export_runtime_fixture.py'))
exporter = importlib.util.module_from_spec(spec)
spec.loader.exec_module(exporter)


def scalar(name, default):
    return dict(type=exporter.PREFIX + 'ScalarParameter', default=default,
                parameter=dict(name=dict(name=name, number=2), association=1, index=3))


class RuntimeFixtureTests(unittest.TestCase):
    def test_postorder_graph_preserves_values_and_duplicate_output_slots(self):
        parameter = scalar('Offset', 2.5)
        folded = dict(type=exporter.PREFIX + 'FoldedMath', inputs=[parameter, parameter],
                      operation=2, value_type=15)
        arrays = dict(vector=dict(count=2, expressions=[folded, parameter]),
                      scalar=dict(count=2, expressions=[parameter, folded]))
        nodes, roots = exporter.flatten_expressions(arrays)
        rebuilt = []
        for index, node in enumerate(nodes):
            row = dict(node, type=exporter.PREFIX + node['type'])
            if 'inputs' in node:
                self.assertTrue(all(child < index for child in node['inputs']))
                row['inputs'] = [rebuilt[child] for child in node['inputs']]
            rebuilt.append(row)
        for kind in ('vector', 'scalar'):
            self.assertEqual(len(roots[kind + '_roots']), 2)
            for original, index in zip(arrays[kind]['expressions'], roots[kind + '_roots']):
                self.assertEqual(exporter.evaluator.evaluate(original, {}, set()),
                                 exporter.evaluator.evaluate(rebuilt[index], {}, set()))
        self.assertEqual(nodes[0]['parameter'], parameter['parameter'])
        self.assertNotEqual(roots['scalar_roots'][0], roots['vector_roots'][1])

    def test_max_saturate_flatten_postorder(self):
        p = scalar('X', .2)
        maximum = dict(type=exporter.PREFIX + 'Max', inputs=[p, p])
        saturated = dict(type=exporter.PREFIX + 'Saturate', inputs=[maximum])
        nodes, roots = exporter.flatten_expressions(dict(vector=dict(count=0, expressions=[]),
                                                          scalar=dict(count=1, expressions=[saturated])))
        self.assertEqual([n['type'] for n in nodes], ['ScalarParameter', 'ScalarParameter', 'Max', 'Saturate'])
        self.assertEqual(nodes[2]['inputs'], [0, 1])
        self.assertEqual(nodes[3]['inputs'], [2])
        self.assertEqual(nodes[0]['parameter'], p['parameter'])
        self.assertEqual(roots['scalar_roots'], [3])

    def test_unknown_expression_and_deep_nesting_fail(self):
        unknown = dict(type=exporter.PREFIX + 'TextureParameter')
        with self.assertRaisesRegex(ValueError, 'unsupported'):
            exporter.flatten_expressions(dict(vector=dict(count=1, expressions=[unknown]),
                                             scalar=dict(count=0, expressions=[])))
        node = scalar('Offset', 1)
        for _ in range(65):
            node = dict(type=exporter.PREFIX + 'ComponentSwizzle', inputs=[node],
                        channels=[0, -1, -1, -1], elements=1)
        with self.assertRaisesRegex(ValueError, 'bound exceeded'):
            exporter.flatten_expressions(dict(vector=dict(count=1, expressions=[node]),
                                             scalar=dict(count=0, expressions=[])))

    def test_inconsistent_declared_root_count_fails(self):
        with self.assertRaisesRegex(ValueError, 'count mismatch'):
            exporter.flatten_expressions(dict(vector=dict(count=2, expressions=[scalar('P', 1)]),
                                             scalar=dict(count=0, expressions=[])))


if __name__ == '__main__':
    unittest.main()
