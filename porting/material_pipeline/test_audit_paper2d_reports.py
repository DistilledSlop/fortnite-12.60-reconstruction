import unittest
from audit_paper2d_reports import audit


def report(count, exports=(), properties=()):
    return {'materials': [{'expressionCount': count, 'blendMode': 'masked', 'shadingModel': 'unlit', 'twoSided': True, 'opacityMaskClipValue': 0.333}], 'materialProperties': [{'properties': list(properties)}], 'expressionExports': [{'name': x, 'exportType': 'Other', 'properties': []} for x in exports], 'instanceParameters': [{}]}


class Paper2dAuditTest(unittest.TestCase):
    def test_stripped_graph_does_not_imply_parity(self):
        result = audit(report(5, ['sampler']), report(5, ['sampler', 'a', 'b', 'c', 'd']), report(None), report(None))
        self.assertFalse(result['original_graph_complete'])
        self.assertEqual(result['parity'], 'unproven')
        self.assertTrue(result['parent_material_settings_match'])

    def test_even_complete_export_count_does_not_prove_semantics(self):
        result = audit(report(1, ['x']), report(1, ['y']), report(None), report(None))
        self.assertTrue(result['original_graph_complete'])
        self.assertEqual(result['parity'], 'unproven')
        self.assertNotEqual(result['original_exported_expressions'], result['current_exported_expressions'])


if __name__ == '__main__':
    unittest.main()
