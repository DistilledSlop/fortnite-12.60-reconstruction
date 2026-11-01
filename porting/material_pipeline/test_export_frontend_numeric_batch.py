#!/usr/bin/env python3
"""Contract tests independent of recovered game data."""
import importlib.util
import hashlib
import json
import struct
import sys
from pathlib import Path
import unittest
import tempfile
from unittest import mock

BATCH_PATH = None
if '--batch' in sys.argv:
    option = sys.argv.index('--batch')
    BATCH_PATH = Path(sys.argv[option + 1]).resolve()
    del sys.argv[option:option + 2]

spec = importlib.util.spec_from_file_location('batch', Path(__file__).with_name('export_frontend_numeric_batch.py'))
batch = importlib.util.module_from_spec(spec)
spec.loader.exec_module(batch)


def parameter(name='Color', number=0, association=2, index=-1):
    return dict(name=dict(name=name, number=number), association=association, index=index)


def row(path, parent='', values=(), static=False):
    owner = dict(name=path.split('.')[-1], type='UMaterialInstanceConstant' if parent else 'UMaterial', staticPermutation=static)
    scalars = [dict(parameter=batch.display_name(p['name']), parameterInfo=p, value=v) for p, v in values]
    return dict(owner=owner, owner_path=path, parent_path=parent, report=dict(instanceParameters=[dict(name=owner['name'], scalars=scalars, vectors=[], textures=[])]))


class BatchTests(unittest.TestCase):
    def test_chain_retains_direct_parent_and_base_separately(self):
        base = row('/Game/Base.Base')
        parent = row('/Game/Parent.Parent', base['owner_path'], static=True)
        child = row('/Game/Child.Child', parent['owner_path'])
        chain = batch.owner_chain(child['owner_path'], {x['owner_path']: x for x in (base, parent, child)})
        self.assertEqual([x['owner_path'] for x in chain], [base['owner_path'], parent['owner_path'], child['owner_path']])
        self.assertNotEqual(chain[0]['owner_path'], child['parent_path'])
        with self.assertRaisesRegex(ValueError, 'outside bounded'):
            batch.owner_chain(child['owner_path'], {child['owner_path']: child})
        parent['parent_path'] = child['owner_path']
        with self.assertRaisesRegex(ValueError, 'cycle'):
            batch.owner_chain(child['owner_path'], {x['owner_path']: x for x in (base, parent, child)})

    def test_inheritance_full_identity_and_optimized_out_retention(self):
        p, numbered, wrong_scope, optimized = parameter(), parameter(number=2), parameter(association=1), parameter('Unused')
        base = row('/Game/Base.Base')
        parent = row('/Game/Parent.Parent', base['owner_path'], [(p, 1), (numbered, 2), (optimized, 7)])
        child = row('/Game/Child.Child', parent['owner_path'], [(p, 3), (wrong_scope, 4)])
        nodes = [dict(type='ScalarParameter', parameter=p), dict(type='ScalarParameter', parameter=numbered)]
        source, effective, used, unused = batch.inherited_values([base, parent, child], nodes)
        self.assertEqual([v['value'] for v in effective], [3, 2, 7, 4])
        self.assertEqual([v['value'] for v in used], [3, 2])
        self.assertEqual([v['value'] for v in unused], [7, 4])
        self.assertEqual(source[1]['values'][0]['value'], 1)
        self.assertEqual(source[1]['serialized_instance_parameters']['scalars'][1]['parameter'], 'Color_1')

    def test_duplicate_and_missing_identity_rejected(self):
        p = parameter()
        bad = row('/Game/A.A', '/Game/B.B', [(p, 1), (p, 2)])
        with self.assertRaisesRegex(ValueError, 'duplicate'):
            batch.serialized_overrides(bad)
        bad['report']['instanceParameters'][0]['scalars'][0]['parameterInfo'] = None
        with self.assertRaisesRegex(ValueError, 'full parameter identity'):
            batch.serialized_overrides(bad)

    def test_textures_in_parent_chain_rejected(self):
        parent = row('/Game/Parent.Parent', '/Game/Base.Base')
        parent['report']['instanceParameters'][0]['textures'] = [dict(parameter='Icon')]
        with self.assertRaisesRegex(ValueError, 'texture overrides'):
            batch.inherited_values([parent], [])

    def test_view_and_unknown_descriptors_fail_closed(self):
        rec = dict(frequency=3, uniform_parameters=[dict(struct_name=dict(name='FViewUniformShaderParameters'), binding=dict(base_index=1, bound=True))], material_uniform_buffer=dict(base_index=2, bound=True))
        desc = dict(d3d_register=dict(space=0, kind='cbv', index=1), set=0, resource_table_entries=[], vulkan_descriptor_type=6, block_span_bytes=2272)
        with self.assertRaisesRegex(ValueError, 'View'):
            batch.typed_contract(rec, dict(descriptors=[desc]))
        desc['d3d_register']['kind'] = 'uav'
        rec['uniform_parameters'] = []
        with self.assertRaisesRegex(ValueError, 'unsupported typed'):
            batch.typed_contract(rec, dict(descriptors=[desc]))

    def test_numeric_material_element_texture_pair_and_spans(self):
        rec = dict(frequency=3, uniform_parameters=[], material_uniform_buffer=dict(base_index=1, bound=True), debug_uniform_buffer_layout=dict(resource_offsets=[32, 40]))
        def desc(kind, index, dtype, span=None):
            return dict(d3d_register=dict(space=0, kind=kind, index=index), set=0, resource_table_entries=[], vulkan_descriptor_type=dtype, block_span_bytes=span)
        descriptors = [desc('cbv', 0, 6, 64), desc('cbv', 1, 6, 32), desc('srv', 0, 2), desc('sampler', 0, 0)]
        self.assertEqual(len(batch.typed_contract(rec, dict(descriptors=descriptors))), 4)
        with self.assertRaisesRegex(ValueError, 'incomplete'):
            batch.typed_contract(rec, dict(descriptors=descriptors[:-1]))
        descriptors[1]['block_span_bytes'] = 48
        with self.assertRaisesRegex(ValueError, 'span'):
            batch.typed_contract(rec, dict(descriptors=descriptors))

    def test_view_proof_requires_exact_module_and_fresh_inspection(self):
        proof = dict(shader_index=7, spirv_sha256='a'*64, field_name='GameTime', byte_offset=2264, output='/old/path')
        inspector = mock.Mock()
        inspector.verify_record.return_value = dict(proof, output='/fresh/path')
        evidence = dict(records={7:proof}, inspector=inspector, grammar=object())
        reflected = dict(shader_index=7, spirv_sha256='b'*64)
        with self.assertRaisesRegex(ValueError, 'matching module'):
            batch.verify_view_shader(reflected, evidence)
        inspector.verify_record.assert_not_called()
        reflected['spirv_sha256'] = 'a'*64
        self.assertEqual(batch.verify_view_shader(reflected, evidence), proof)
        inspector.verify_record.return_value = dict(proof, byte_offset=2268)
        with self.assertRaisesRegex(ValueError, 'fresh module'):
            batch.verify_view_shader(reflected, evidence)

    def test_proven_view_typed_roles(self):
        rec = dict(frequency=3, uniform_parameters=[dict(struct_name=dict(name='FViewUniformShaderParameters'), binding=dict(base_index=1,bound=True))],
                   material_uniform_buffer=dict(base_index=2,bound=True), debug_uniform_buffer_layout=dict(resource_offsets=[416,424]))
        def descriptor(index, span):
            return dict(d3d_register=dict(kind='cbv',space=0,index=index), set=0, vulkan_descriptor_type=6, resource_table_entries=[], block_span_bytes=span)
        reflected = dict(descriptors=[descriptor(0,64),descriptor(1,2272),descriptor(2,416)])
        with mock.patch.object(batch, 'verify_view_shader') as verify:
            result = batch.typed_contract(rec,reflected,object())
            verify.assert_called_once()
            self.assertEqual([x['semantic'] for x in result], ['SlateLooseConstants','View.GameTime','Material'])
            reflected['descriptors'][1]['block_span_bytes'] = 3952
            with self.assertRaisesRegex(ValueError,'View descriptor'):
                batch.typed_contract(rec,reflected,object())

    def test_existing_output_rejected_without_touching_it(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            output = root / 'existing'
            output.mkdir()
            sentinel = output / 'original'
            sentinel.write_bytes(b'preserve')
            with mock.patch.object(batch, 'GENERATED', root):
                with self.assertRaisesRegex(ValueError, 'fresh batch'):
                    batch.export_batch(root / 'absent-inventory', output)
            self.assertEqual(sentinel.read_bytes(), b'preserve')
            self.assertEqual(list(output.iterdir()), [sentinel])

    def test_unsupported_resource_shape_and_textures(self):
        with self.assertRaisesRegex(ValueError, 'record shape'):
            batch.numeric_contract(dict(complete_serialized_shader_map=True, undecoded_bytes=0, shader_record_count=14))
        material = dict(complete_serialized_shader_map=True, undecoded_bytes=0, shader_record_count=19,
                        uniform_expression_arrays=dict(texture_2d=dict(count=1)))
        with self.assertRaisesRegex(ValueError, 'texture expressions'):
            batch.numeric_contract(material)

    def test_numeric_case_identity_isolation_and_owner_defaults(self):
        p = parameter('Amount')
        expression = dict(type='FMaterialUniformExpressionScalarParameter', parameter=p, default=0.25)
        material = dict(virtual_texture_stacks=dict(count=0), uniform_expression_arrays=dict(texture_virtual=dict(count=0), vector=dict(count=0, expressions=[]), scalar=dict(count=1, expressions=[expression])))
        nodes = [dict(type='ScalarParameter', parameter=p)]
        cases = batch.numeric_cases(material, nodes, [dict(type='scalar', parameter=p, value=0.75)])
        self.assertEqual(cases[0]['numeric_values'][0], 0.75)
        self.assertEqual(cases[1]['numeric_values'][0], 0.375)
        self.assertIn(cases[-3]['overrides'][0]['parameter']['association'], range(3))
        for case in cases[-3:]:
            self.assertEqual(case['numeric_sha256'], cases[0]['numeric_sha256'])


@unittest.skipUnless(BATCH_PATH is not None, 'pass --batch generated/batch-directory to audit emitted artifacts')
class GeneratedBatchAudit(unittest.TestCase):
    def test_all_manifest_bundles_source_bytes_shader_receipts_and_numeric_cases(self):
        manifest_raw = (BATCH_PATH / 'manifest.json').read_bytes()
        manifest = json.loads(manifest_raw)
        report = json.loads((BATCH_PATH / 'report.json').read_bytes())
        self.assertEqual(batch.digest(manifest_raw), report['manifest_sha256'])
        self.assertEqual(sum(report['counts'].values()), report['owner_count'])
        self.assertEqual(len(manifest['entries']), report['counts']['accepted'])
        self.assertGreater(len(manifest['entries']), 0)
        view_evidence = None
        case_count = 0
        for entry in manifest['entries']:
            raw = Path(entry['bundle_path']).read_bytes()
            self.assertEqual(hashlib.sha1(raw).hexdigest(), entry['bundle_sha1'])
            self.assertEqual(batch.digest(raw), entry['bundle_sha256'])
            bundle = json.loads(raw)
            self.assertNotIn(entry['owner_path'], batch.FROZEN_OWNERS)
            for key in ('owner_path', 'owner_type', 'parent_path', 'resource_owner_path', 'shader_identity'):
                self.assertEqual(entry[key], bundle[key])
            self.assertEqual(bundle['provenance']['exporter_sha256'], batch.digest(Path(batch.__file__).read_bytes()))
            payload = bytes.fromhex(bundle['resource_raw_hex'])
            self.assertEqual(hashlib.sha1(payload).hexdigest(), entry['resource_sha1'])
            self.assertEqual(batch.digest(payload), entry['resource_sha256'])
            source = batch.EXTRACTION / (bundle['framing']['package'] + '.uexp')
            self.assertEqual(batch.bundle.validate_source(source.read_bytes(), bundle['framing']), payload)
            self.assertEqual(len(bundle['original_shader_code']), 19)
            self.assertEqual(len(batch.bundle.all_records(bundle['decoded'])), 19)
            self.assertEqual(bundle['decoded']['material_path'], entry['shader_identity'])
            fixture = bundle['runtime_fixture']
            graph = {batch.evaluator.identity(x['parameter']) for x in fixture['nodes'] if x['type'] in ('ScalarParameter','VectorParameter')}
            for key, inherited in [('uniform_cases', []), ('owner_uniform_cases', fixture['graph_effective_parameter_expectations'])]:
                for case in fixture[key]:
                    effective = {batch.evaluator.identity(x['parameter']):x for x in inherited}
                    effective.update({batch.evaluator.identity(x['parameter']):x for x in case['overrides'] if batch.evaluator.identity(x['parameter']) in graph})
                    numeric = batch.evaluator.evaluate_report(bundle['decoded'], batch.evaluator.overrides_from_json(list(effective.values())))[0]
                    self.assertEqual(batch.digest(numeric),case['numeric_sha256'])
                    self.assertEqual(list(struct.unpack('<'+'f'*(len(numeric)//4),numeric)),case['numeric_values'])
                    case_count += 1
            if entry['requires_view']:
                contract = fixture['view_contract']
                self.assertEqual(batch.digest(Path(contract['proof_path']).read_bytes()), contract['proof_sha256'])
                self.assertEqual(batch.digest(Path(contract['access_proof_path']).read_bytes()), contract['access_proof_sha256'])
                view_evidence = batch.load_view_evidence(Path(contract['access_proof_path']),Path(contract['proof_path']))
                if fixture.get('view_draw_profile_status',{}).get('status') == 'source_derived':
                    cases = fixture['view_cases']
                    self.assertEqual(len(cases),4)
                    self.assertEqual(fixture['view_draw_expectations']['equal_image_pairs'],[[0,2],[1,3]])
                    self.assertEqual(fixture['view_draw_expectations']['different_image_pairs'],[[0,1],[2,3]])
                    for case in cases:
                        numeric = batch.evaluator.evaluate_report(bundle['decoded'],batch.evaluator.overrides_from_json(case['overrides']))[0]
                        self.assertEqual(batch.digest(numeric),case['numeric_sha256'])
                        self.assertEqual(list(struct.unpack('<'+'f'*(len(numeric)//4),numeric)),case['numeric_values'])
                        self.assertTrue(any(x['type']=='scalar' and x['parameter']==dict(name=dict(name='UseShineSwipe',number=0),association=2,index=-1) and x['value']==1 for x in case['overrides']))
                        self.assertEqual(case['numeric_values'][99],1.0)
                        case_count += 1
                    threshold,period = cases[0]['numeric_values'][97:99]
                    self.assertTrue(0 <= threshold < period and period-threshold>0.25)
                    self.assertEqual([x['game_time'] for x in cases],[0,threshold+0.25,0,threshold+0.25])
                    self.assertEqual([x['real_time'] for x in cases],[10,10,20,20])

            for shader in fixture['shaders']:
                code = Path(shader['reflection']['output']).read_bytes()
                self.assertEqual(batch.digest(code),shader['mapped']['output_sha256'])
                self.assertEqual(batch.digest(code),shader['reflection']['spirv_sha256'])
                self.assertEqual(hashlib.sha1(code).hexdigest(),shader['spirv_sha1'])
                batch.typed_contract(shader['record'],shader['reflection'],view_evidence if entry['requires_view'] else None)
        print('Audited %d manifest bundles and %d numeric cases' % (len(manifest['entries']),case_count))


if __name__ == '__main__':
    unittest.main()
