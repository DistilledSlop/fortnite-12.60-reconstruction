#!/usr/bin/env python3
"""Contract tests independent of recovered game data."""
import importlib.util
import copy
import hashlib
import json
import struct
import sys
from pathlib import Path
import unittest
import tempfile
from unittest import mock

BATCH_PATH = None
BASELINE_PATH = None
if '--batch' in sys.argv:
    option = sys.argv.index('--batch')
    BATCH_PATH = Path(sys.argv[option + 1]).resolve()
    del sys.argv[option:option + 2]

if '--baseline' in sys.argv:
    option = sys.argv.index('--baseline')
    BASELINE_PATH = Path(sys.argv[option + 1]).resolve()
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
    def test_visible_time_delta_policy_is_master_only(self):
        master = '/Game/UI/Foundation/Buttons/Materials/Feature/M_FeatureButton_Master.M_FeatureButton_Master'
        policy = batch.feature_view_draw_policy(master)
        self.assertIs(policy['require_time_delta'],True)
        self.assertEqual(policy['different_image_pairs'],[[0,1],[2,3]])
        for name in ('M_Button_Feature_Base','M_Button_Feature_Disabled','M_Button_Feature_Hovered','M_Button_Feature_Pressed','M_Button_Feature_Text_Base'):
            child = '/Game/UI/Foundation/Buttons/Materials/Feature/' + name + '.' + name
            policy = batch.feature_view_draw_policy(child)
            self.assertIs(policy['require_time_delta'],False)
            self.assertEqual(policy['different_image_pairs'],[])
            self.assertEqual(policy['equal_image_pairs'],[[0,2],[1,3]])
            self.assertIn('mask shine',policy['time_delta_rationale'])

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

    def test_trimmed_angled_material_spans_and_original_binding(self):
        record = dict(frequency=3, uniform_parameters=[], material_uniform_buffer=dict(base_index=1, bound=True),
                      debug_uniform_buffer_layout=dict(resource_offsets=[736, 744], resource_types=[8, 8], constant_bytes=752))
        def descriptor(index, span):
            return dict(d3d_register=dict(space=0, kind='cbv', index=index), set=0,
                        resource_table_entries=[], vulkan_descriptor_type=6, block_span_bytes=span)
        descriptors = [descriptor(0, 64), descriptor(1, 608)]
        for span in (608, 720, 736):
            descriptors[1]['block_span_bytes'] = span
            self.assertEqual(batch.typed_contract(record, dict(descriptors=descriptors))[1]['semantic'], 'Material')
        for span in (0, 607, 752):
            descriptors[1]['block_span_bytes'] = span
            with self.assertRaisesRegex(ValueError, 'span'):
                batch.typed_contract(record, dict(descriptors=descriptors))
        descriptors[1]['block_span_bytes'] = 608
        for binding in (dict(base_index=2, bound=True), dict(base_index=1, bound=False)):
            record['material_uniform_buffer'] = binding
            with self.assertRaisesRegex(ValueError, 'span'):
                batch.typed_contract(record, dict(descriptors=descriptors))
        record['material_uniform_buffer'] = dict(base_index=1, bound=True)
        descriptors[1]['d3d_register']['index'] = 2
        with self.assertRaisesRegex(ValueError, 'span'):
            batch.typed_contract(record, dict(descriptors=descriptors))

    def test_real_angled_source_records_and_reflected_spans(self):
        generated = Path(__file__).resolve().parent.parent / 'generated'
        decoded_path = generated / 'frontend-probes/frontend-material-inventory/FortniteGame_Content_UI_Art_ToolKit_AngledBoxes_M_UI_AngledBox_Master.decoded.json'
        reflected_path = generated / 'material_pipeline/frontend-numeric-batch-10/shaders-000/reflection.json'
        if not decoded_path.exists() or not reflected_path.exists():
            self.skipTest('local original AngledBox evidence is unavailable')
        source = json.loads(decoded_path.read_bytes())['material_shaders']['records']
        reflected = {r['shader_index']: r for r in json.loads(reflected_path.read_bytes())['records']}
        pixel = [r for r in source if r['type']['name'].startswith('TSlateMaterialShaderPS')]
        self.assertEqual(len(pixel), 5)
        for record, index, span in zip(pixel, range(3186, 3191), (608, 720, 720, 608, 720)):
            self.assertEqual(record['debug_uniform_buffer_layout']['resource_offsets'], [736, 744])
            self.assertEqual(record['material_uniform_buffer'], dict(base_index=1, bound=True))
            self.assertEqual(next(d['block_span_bytes'] for d in reflected[index]['descriptors']
                                  if d['d3d_register'] == dict(kind='cbv', space=0, index=1)), span)
            self.assertIn('Material', [b['semantic'] for b in batch.typed_contract(record, reflected[index])])

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

    def test_original_fourteen_record_shape_and_rejections(self):
        path = (Path(__file__).resolve().parent.parent / 'generated/frontend-probes/frontend-material-inventory/'
                'FortniteGame_Content_UI_Foundation_Lobby_materials_M_UI_Border_HGrad.decoded.json')
        if not path.exists():
            self.skipTest('local original HGrad evidence is unavailable')
        original = json.loads(path.read_bytes())
        self.assertEqual(original['resource_sha256'], '8568ae19af007ab8b5b734bb53ffb786d815b6e80f1a3a0cc262afd1fd701b35')
        self.assertEqual(original['shader_record_count'], 14)
        records = batch.bundle.all_records(original)
        self.assertEqual(len(records), 14)
        self.assertTrue(all('index' in record['library_association'] for record in records))
        self.assertEqual(original['resource_end'] - original['resource_start'], 6163)
        self.assertEqual(original['decoded_prefix_bytes'], 6163)
        batch.numeric_contract(original)
        for mutation in ('missing', 'extra', 'count', 'group_count', 'slate_missing', 'family', 'incomplete', 'texture'):
            parsed = copy.deepcopy(original)
            mesh = parsed['mesh_shader_maps'][0]
            if mutation == 'missing':
                mesh['records'].pop()
            elif mutation == 'extra':
                mesh['records'].append(copy.deepcopy(mesh['records'][0]))
            elif mutation == 'count':
                parsed['shader_record_count'] += 1
            elif mutation == 'group_count':
                mesh['count'] += 1
            elif mutation == 'slate_missing':
                parsed['material_shaders']['records'].pop()
                parsed['material_shaders']['count'] -= 1
                parsed['shader_record_count'] -= 1
            elif mutation == 'family':
                mesh['records'][0]['type']['name'] = 'UnsupportedShader'
            elif mutation == 'incomplete':
                parsed['undecoded_bytes'] = 1
            else:
                parsed['uniform_expression_arrays']['texture_2d']['count'] = 1
            with self.subTest(mutation=mutation), self.assertRaises(ValueError):
                batch.numeric_contract(parsed)

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
            records = batch.bundle.all_records(bundle['decoded'])
            self.assertEqual(len(records), bundle['decoded']['shader_record_count'])
            self.assertGreaterEqual(len(records), 7)
            self.assertLessEqual(len(records), 256)
            self.assertEqual(len(bundle['original_shader_code']), len(records))
            for record, receipt in zip(records, bundle['original_shader_code']):
                self.assertEqual(receipt['type'], record['type'])
                self.assertEqual(receipt['library_index'], record['library_association']['index'])
                self.assertEqual(receipt['output_hash'], record['output_hash']['hash'])
                self.assertEqual(receipt['frequency'], record['frequency'])
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
                    expected_policy = batch.feature_view_draw_policy(entry['owner_path'])
                    for field in ('require_time_delta','different_image_pairs','equal_image_pairs'):
                        self.assertEqual(fixture['view_draw_expectations'][field],expected_policy[field])
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


@unittest.skipUnless(BATCH_PATH is not None and BASELINE_PATH is not None,'pass --batch and --baseline to compare preserved numeric cases')
class PriorBatchCasesAudit(unittest.TestCase):
    def test_all_previous_cases_preserved_under_new_draw_policy(self):
        old = json.loads((BASELINE_PATH / 'manifest.json').read_bytes())
        new = json.loads((BATCH_PATH / 'manifest.json').read_bytes())
        current = {e['owner_path']:e for e in new['entries']}
        cases = 0
        for entry in old['entries']:
            prior = json.loads(Path(entry['bundle_path']).read_bytes())['runtime_fixture']
            fresh = json.loads(Path(current[entry['owner_path']]['bundle_path']).read_bytes())['runtime_fixture']
            for field in ('uniform_cases','owner_uniform_cases','view_cases'):
                self.assertEqual(prior.get(field),fresh.get(field))
                cases += len(prior.get(field,[]))
            if entry['owner_path'].endswith(('.M_Button_Feature_Hovered','.M_Button_Feature_Pressed')):
                self.assertAlmostEqual(fresh['view_cases'][1]['game_time'],1.5833333730697632)
        print('Preserved %d prior numeric cases from %s' % (cases,BASELINE_PATH.name))


if __name__ == '__main__':
    unittest.main()
