#!/usr/bin/env python3
"""Reject source, name-table and full parameter-identity substitutions."""
import copy
import hashlib
import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location('ingress_exporter', Path(__file__).with_name('export_ingress_bundle.py'))
exporter = importlib.util.module_from_spec(spec)
spec.loader.exec_module(exporter)


def string(value):
    raw = value.encode() + b'\0'
    return struct.pack('<i', len(raw)) + raw


def source():
    payload = (struct.pack('<4I', 1, 1, 1, 3) + bytes(range(20)) + struct.pack('<I', 0)
               + string('M_Test') + string('/Game/UI/M_Test.M_Test') + struct.pack('<I', 0))
    raw = (struct.pack('<i', 1) + string('Material') + bytes(4)
           + struct.pack('<iIBBi', 1, 0, 3, 1, len(payload)) + payload)
    framing = exporter.resource_reader.parse_resource(raw, 0, len(raw))
    framing.update(export_start=0, export_end=len(raw), export_name='M_Test',
                   uexp_sha256=hashlib.sha256(raw).hexdigest())
    framing['observed_inline_header'] = exporter.resource_reader.inspect_legacy_header(
        raw, framing['resource_start'], len(raw), 'M_Test')
    return raw, payload, framing


class IngressBundleTests(unittest.TestCase):
    def test_checked_wrapper_returns_exact_original_resource(self):
        raw, payload, framing = source()
        self.assertEqual(exporter.validate_source(raw, framing), payload)

    def test_changed_source_and_export_bounds_are_rejected(self):
        raw, _, framing = source()
        with self.assertRaisesRegex(ValueError, 'original uexp'):
            exporter.validate_source(raw[:-1] + b'\x01', framing)
        for end in (-1, len(raw) + 1):
            changed = dict(framing, export_end=end)
            with self.subTest(end=end), self.assertRaisesRegex(ValueError, 'export range'):
                exporter.validate_source(raw, changed)

    def test_name_table_and_resource_identity_receipts_are_recomputed(self):
        raw, _, framing = source()
        for field, value in [('names', ['Unrelated']), ('resource_sha256', '0' * 64)]:
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, field):
                exporter.validate_source(raw, dict(framing, **{field: value}))

    def test_fixture_must_preserve_parameter_number_association_and_index(self):
        parameter = dict(name=dict(name='Offset', number=2), association=1, index=3)
        arrays = dict(vector=dict(count=0, expressions=[]), scalar=dict(count=1,
            expressions=[dict(type='FMaterialUniformExpressionScalarParameter',
                              parameter=parameter, default=0.5)]))
        material = dict(resource_sha256='a' * 64, shader_library=dict(sha256='b' * 64),
                        uniform_expression_arrays=arrays)
        nodes, roots = exporter.fixture_exporter.flatten_expressions(arrays)
        fixture = dict(schema='fortnite.legacy-slate.v1', material={},
            provenance=dict(resource_sha256='a' * 64, shader_library_sha256='b' * 64), nodes=nodes, **roots)
        for field in ('number', 'association', 'index'):
            altered = copy.deepcopy(fixture)
            info = altered['nodes'][0]['parameter']
            if field == 'number':
                info['name']['number'] += 1
            else:
                info[field] += 1
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, 'graph differs'):
                exporter.verify_fixture(material, altered)


if __name__ == '__main__':
    unittest.main()
