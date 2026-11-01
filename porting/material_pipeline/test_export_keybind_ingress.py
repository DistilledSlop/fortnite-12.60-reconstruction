#!/usr/bin/env python3
"""Fail closed on unsupported texture semantics and altered original owner evidence."""
import copy
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('keybind', Path(__file__).with_name('export_keybind_ingress.py'))
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
INVENTORY = module.ROOT / 'keybind-inventory-2'
FUNCTIONS = module.ROOT / 'keybind-function-properties-1'
TEXTURE = module.ROOT / 'keybind-texture-1/texture-report.json'
SHADERS = module.ROOT / 'keybind-shaders-1'
STEM = module.PACKAGE.replace('/', '_')


class PortableRejectionTests(unittest.TestCase):
    """Synthetic malformed contracts test rejection only, without original assets or runtime claims."""

    def texture_arrays(self, expression):
        arrays = {name: dict(count=0, expressions=[]) for name in module.bundle.decoder.ARRAYS}
        arrays['texture_2d'] = dict(count=1, expressions=[expression])
        return dict(uniform_expression_arrays=arrays, virtual_texture_stacks=dict(count=0),
                    parameter_collections=dict(count=0))

    def test_plain_texture_cannot_be_relabelled_as_a_parameter(self):
        expression = dict(offset=879, end=903, type='FMaterialUniformExpressionTextureParameter',
                          texture=copy.deepcopy(module.BASE_TEXTURE))
        with self.assertRaisesRegex(ValueError, 'regular Texture'):
            module.validate_arrays(self.texture_arrays(expression))

    def test_plain_texture_rejects_invented_parameter_identity(self):
        expression = dict(offset=879, end=903, type='FMaterialUniformExpressionTexture',
            texture=copy.deepcopy(module.BASE_TEXTURE),
            parameter=dict(name=dict(name='Invented', number=0), association=2, index=-1))
        with self.assertRaisesRegex(ValueError, 'regular Texture'):
            module.validate_arrays(self.texture_arrays(expression))

    def record(self):
        return dict(frequency=3, material_uniform_buffer=dict(base_index=1, bound=True), uniform_parameters=[])

    def descriptor(self, material=True):
        return dict(set=0, binding=0, d3d_register=dict(kind='srv', space=0, index=0 if material else 1),
                    vulkan_descriptor_type=2, image_type=copy.deepcopy(module.SAMPLED_IMAGE),
                    resource_table_entries=[dict(uniform_buffer=1, resource=0, bind=0, table='texture')] if material else [])

    def test_resource_table_rejects_unknown_register_kinds_without_negative_index_fallback(self):
        for kind in ['uav', 'cbv', 'unknown']:
            descriptor = self.descriptor()
            descriptor['d3d_register']['kind'] = kind
            with self.assertRaisesRegex(ValueError, 'resource-table register kind'):
                module.typed_contract(self.record(), dict(descriptors=[descriptor]))

    def test_material_and_element_images_reject_unsupported_sampled_image_shapes(self):
        mutations = [('sampled_type', dict(kind='float', bits=16)), ('dimension', 2), ('depth', 0),
                     ('arrayed', 1), ('multisampled', 1), ('sampled', 2), ('format', 1)]
        for material in [True, False]:
            for field, value in mutations:
                with self.subTest(material=material, field=field):
                    descriptor = self.descriptor(material)
                    descriptor['image_type'][field] = value
                    with self.assertRaisesRegex(ValueError, 'sampled-image shape'):
                        module.typed_contract(self.record(), dict(descriptors=[descriptor]))

    def test_unknown_descriptor_role_is_rejected(self):
        descriptor = self.descriptor(False)
        descriptor['d3d_register'] = dict(kind='unknown', space=0, index=4)
        with self.assertRaisesRegex(ValueError, 'unclassified texture/sampler'):
            module.typed_contract(self.record(), dict(descriptors=[descriptor]))


@unittest.skipUnless((INVENTORY / (STEM + '.decoded.json')).exists() and TEXTURE.exists() and
                     (SHADERS / 'reflection.json').exists(), 'local original Keybind evidence unavailable')
class KeybindIngressTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.parsed = json.loads((INVENTORY / (STEM + '.decoded.json')).read_text())
        cls.owner = json.loads((INVENTORY / (STEM + '.json')).read_text())
        cls.exports = json.loads((INVENTORY / (STEM + '.export-map.json')).read_text())
        cls.functions = [json.loads(p.read_text()) for p in sorted(FUNCTIONS.glob('*.json'))]
        cls.texture = json.loads(TEXTURE.read_text())
        cls.source = module.EXTRACTED / (module.TEXTURE_PACKAGE + '.uexp')
        cls.header_bytes = cls.source.with_suffix('.uasset').stat().st_size
        cls.reflection = json.loads((SHADERS / 'reflection.json').read_text())

    def context(self, functions=None, owner=None, exports=None):
        return module.prove_context(owner or self.owner, exports or self.exports,
                                    self.functions if functions is None else functions)

    def test_original_complete_bundle_has_regular_texture_without_parameter_identity(self):
        data = module.create_bundle(INVENTORY, FUNCTIONS, TEXTURE, SHADERS)
        fixture = data['runtime_fixture']
        self.assertEqual(len(data['original_shader_code']), 19)
        self.assertEqual(len(fixture['shaders']), 7)
        self.assertEqual((len(fixture['vector_roots']), len(fixture['scalar_roots'])), (6, 10))
        self.assertEqual(fixture['expected_layout'], module.LAYOUT)
        self.assertEqual(fixture['numeric_comparison_bytes'], 136)
        self.assertEqual(fixture['ordered_texture_references'], [module.TEXTURE_PATH])
        self.assertEqual(fixture['texture_2d_parameters'][0]['expression_kind'], 'Texture')
        self.assertIsNone(fixture['texture_2d_parameters'][0]['parameter'])
        self.assertFalse(fixture['texture_contract']['named_overrides_allowed'])
        controls = fixture['diagnostic_sampling_control']
        self.assertEqual(controls['source_rgba_sha256'], fixture['texture_contract']['default']['rgbaSha256'])
        self.assertFalse(controls['owner_cache_mutation_allowed'])
        self.assertFalse(controls['named_texture_overrides_allowed'])
        self.assertEqual(data['shader_identity'], module.OWNER)
        self.assertEqual(data['resource_owner_path'], module.OWNER)

    def test_context_preserves_stored_expression_order_and_function_postorder(self):
        context = self.context()
        self.assertEqual(context['trace'][0]['expression_position'], 0)
        self.assertEqual(context['trace'][0]['reference_index'], 0)
        self.assertEqual(context['trace'][1]['function_order'], [
            'FortniteGame/Content/UI/Materials/MF_Smoothstep', 'FortniteGame/Content/UI/Materials/AAThreshold'])
        self.assertTrue(all(not row.get('texture_appends') for row in context['trace'][1:]))

    def test_missing_and_duplicate_function_reports_are_rejected(self):
        with self.assertRaisesRegex(ValueError, 'dependency'):
            self.context([r for r in self.functions if not r['path'].endswith('/MF_Smoothstep')])
        with self.assertRaisesRegex(ValueError, 'duplicate'):
            self.context(self.functions + [self.functions[0]])

    def test_texture_bearing_function_is_rejected(self):
        reports = copy.deepcopy(self.functions)
        report = next(r for r in reports if r['path'].endswith('/AAThreshold'))
        report['objects'][0]['Type'] = 'MaterialExpressionTextureSample'
        with self.assertRaisesRegex(ValueError, 'texture-bearing'):
            self.context(reports)

    def test_dependency_target_export_index_is_checked(self):
        reports = copy.deepcopy(self.functions)
        report = next(r for r in reports if r['path'].endswith('/AAThreshold'))
        report['objects'][0]['Properties']['MaterialFunction']['ObjectPath'] += '9'
        with self.assertRaisesRegex(ValueError, 'root index/name'):
            self.context(reports)

    def test_parent_expression_export_identity_is_checked(self):
        owner = copy.deepcopy(self.owner)
        owner['materialProperties'][0]['expressionRefs'][0]['index'] = 2
        with self.assertRaisesRegex(ValueError, 'index/name'):
            self.context(owner=owner)

    def test_regular_texture_cannot_acquire_parameter_identity_or_virtual_flag(self):
        for field, value in [('parameter', dict(name=dict(name='Invented', number=0), association=2, index=-1)),
                             ('type', 'FMaterialUniformExpressionTextureParameter')]:
            parsed = copy.deepcopy(self.parsed)
            parsed['uniform_expression_arrays']['texture_2d']['expressions'][0][field] = value
            with self.assertRaisesRegex(ValueError, 'regular Texture'):
                module.validate_arrays(parsed)
        parsed = copy.deepcopy(self.parsed)
        parsed['uniform_expression_arrays']['texture_2d']['expressions'][0]['texture']['virtual_texture'] = True
        with self.assertRaisesRegex(ValueError, 'regular Texture'):
            module.validate_arrays(parsed)

    def test_texture_property_and_changed_layout_are_rejected(self):
        parsed = copy.deepcopy(self.parsed)
        parsed['uniform_expression_arrays']['vector']['expressions'][0]['type'] = 'FMaterialUniformExpressionTextureProperty'
        with self.assertRaisesRegex(ValueError, 'unsupported runtime expression'):
            module.validate_arrays(parsed)
        parsed = copy.deepcopy(self.parsed)
        parsed['material_shaders']['records'][0]['debug_uniform_buffer_layout']['resource_types'][0] = 8
        with self.assertRaisesRegex(ValueError, 'typed uniform layout'):
            module.validate_arrays(parsed)

    def test_native_texture_frame_and_canonical_pixels_match_without_resize(self):
        result = module.validate_texture(self.texture, self.source.read_bytes(), self.header_bytes)
        self.assertEqual((result['logical_width'], result['logical_height'], result['mip_count']), (64, 64, 1))
        self.assertFalse(result['virtual_texture'])
        self.assertEqual(result['stored_bulk_offset'], self.header_bytes + result['inline_pixels_offset'])
        self.assertEqual(result['rgba_sha256'], '804c4bcf49353b3fe938fe3dd9bb96b0b6d3b826937b3754c0785250b906fad4')

    def test_changed_texture_dimensions_and_virtual_payload_are_rejected(self):
        raw = bytearray(self.source.read_bytes())
        position = raw.index(b'PF_B8G8R8A8\0') - 16
        raw[position:position + 4] = (128).to_bytes(4, 'little')
        with self.assertRaisesRegex(ValueError, 'logical platform'):
            module.validate_texture(self.texture, bytes(raw), self.header_bytes)
        raw = bytearray(self.source.read_bytes())
        raw[16696:16700] = (1).to_bytes(4, 'little')
        with self.assertRaisesRegex(ValueError, 'virtual flag'):
            module.validate_texture(self.texture, bytes(raw), self.header_bytes)

    def test_source_resource_table_material_links_are_typed(self):
        record = self.parsed['material_shaders']['records'][3]
        reflected = next(r for r in self.reflection['records'] if r['shader_index'] == record['library_association']['index'])
        result = module.typed_contract(record, reflected)
        self.assertEqual(sorted(r['resource_index'] for r in result['material_resources']), [0, 1])
        changed = copy.deepcopy(reflected)
        descriptor = next(d for d in changed['descriptors'] if d['resource_table_entries'])
        descriptor['resource_table_entries'][0]['resource'] = 3
        with self.assertRaisesRegex(ValueError, 'resource-table'):
            module.typed_contract(record, changed)

    def test_unmatched_numeric_parameter_identities_leave_defaults(self):
        nodes, _ = module.validate_arrays(self.parsed)
        cases = {r['name']: r for r in module.uniform_cases(self.parsed, nodes)}
        for key in ['unmatched_association', 'unmatched_index', 'unmatched_number']:
            self.assertEqual(cases[key]['numeric_sha256'], cases['defaults']['numeric_sha256'])
            self.assertTrue(cases[key]['overrides'])
        self.assertNotEqual(cases['BorderColor']['numeric_sha256'], cases['defaults']['numeric_sha256'])

    def test_material_frame_hash_tampering_is_rejected_before_conversion(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            for suffix in ['.json', '.export-map.json', '.inline-resource.json', '.decoded.json']:
                (directory / (STEM + suffix)).write_bytes((INVENTORY / (STEM + suffix)).read_bytes())
            path = directory / (STEM + '.inline-resource.json')
            data = json.loads(path.read_text()); data['uexp_sha256'] = '0' * 64
            path.write_text(json.dumps(data))
            with self.assertRaisesRegex(ValueError, 'uexp differs'):
                module.create_bundle(directory, FUNCTIONS, TEXTURE, SHADERS)


if __name__ == '__main__':
    unittest.main()
