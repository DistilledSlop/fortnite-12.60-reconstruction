#!/usr/bin/env python3
"""Tests for the source-based texture candidate diagnostic."""
import unittest
from unittest import mock
import inspect_frontend_texture_candidates as subject


class CandidateTests(unittest.TestCase):
    def test_inheritance_resource_selection_and_unproven_imports(self):
        base = dict(owner_path='/Game/Base.Base', parent_path='', owner=dict(type='UMaterial', name='Base'), report=dict(imports=[dict(className='Texture2D', root='/Game/T', objectName='T')]))
        child = dict(owner_path='/Game/Child.Child', parent_path=base['owner_path'], owner=dict(type='UMaterialInstanceConstant', name='Child', staticPermutation=False), report=dict(imports=[], instanceParameters=[dict(name='Child', scalars=[], vectors=[], textures=[dict(parameter='Icon')])]))
        owners = {r['owner_path']: r for r in (base, child)}
        parsed = dict(material_path=base['owner_path'], uniform_expression_arrays=dict(texture_2d=dict(count=1, expressions=[dict(texture=dict(texture_index=0))])))
        with mock.patch.object(subject, 'parse_resource', return_value=parsed) as parse:
            cache = {}
            result = subject.describe(child['owner_path'], owners, object(), cache)
            subject.describe(base['owner_path'], owners, object(), cache)
        parse.assert_called_once()
        self.assertEqual(result['resource_owner_path'], base['owner_path'])
        self.assertEqual(result['texture_import_candidates'][0]['object_name'], 'T')
        self.assertEqual(result['serialized_texture_overrides'][0]['owner_path'], child['owner_path'])
        parsed['uniform_expression_arrays']['texture_2d']['count'] = 2
        with self.assertRaisesRegex(ValueError, 'count mismatch'):
            subject.describe(child['owner_path'], owners, object(), cache)

    def test_rejects_wrong_base_identity(self):
        row = dict(owner_path='/Game/Base.Base', parent_path='', owner=dict(type='UMaterial'), report=dict(imports=[]))
        with mock.patch.object(subject, 'parse_resource', return_value=dict(material_path='/Game/Other.Other')):
            with self.assertRaisesRegex(ValueError, 'base identity'):
                subject.describe(row['owner_path'], {row['owner_path']: row}, object(), {})


if __name__ == '__main__':
    unittest.main()
