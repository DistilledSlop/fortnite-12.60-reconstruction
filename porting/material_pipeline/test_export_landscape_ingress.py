#!/usr/bin/env python3
"""Terrain ingress identity, repeated-record and original two-map regressions."""
import copy
import importlib.util
import json
from pathlib import Path
from types import SimpleNamespace
import unittest
import tempfile

spec = importlib.util.spec_from_file_location('terrain_ingress', Path(__file__).with_name('export_landscape_ingress.py'))
exporter = importlib.util.module_from_spec(spec)
spec.loader.exec_module(exporter)


def sample_maps():
    record = dict(offset=80, type=dict(name='FVelocityVS', number=0),
        pipeline=dict(name='VelocityPipeline', number=0),
        vertex_factory=dict(name='FLandscapeVertexFactory', number=0), frequency=0,
        output_hash=dict(hash='ab' * 20), resource=dict(uncompressed_code_bytes=4))
    material = dict(material_shaders=dict(records=[]), mesh_shader_maps=[dict(
        records=[record], pipelines=[dict(stages=[copy.deepcopy(record)])])])
    material['mesh_shader_maps'][0]['pipelines'][0]['stages'][0]['offset'] = 160
    return [dict(offset=0, end=200, decoded=material)]


class LandscapeIngressTests(unittest.TestCase):
    def archive(self, frequency=0, size=4, code=b'DXBC'):
        return SimpleNamespace(shader_lookup={bytes.fromhex('ab' * 20): 0},
            shaders=[(0, size, size, frequency)], shader_code=lambda index: code)

    def test_repeated_shader_code_keeps_distinct_pipeline_occurrences(self):
        maps = sample_maps()
        code, records = exporter.associate_code(maps, self.archive())
        self.assertEqual(len(code), 1)
        self.assertEqual(bytes.fromhex(code[0]['code_raw_hex']), b'DXBC')
        self.assertEqual(code[0]['output_hash'], 'ab' * 20)
        self.assertEqual([r['location']['record_offset'] for r in records], [80, 160])
        self.assertEqual([r['location']['pipeline_index'] for r in records], [None, 0])
        self.assertEqual(records[1]['location']['stage_index'], 0)

    def test_archive_target_size_missing_hash_and_short_code_fail_closed(self):
        for archive in (self.archive(frequency=3), self.archive(size=5),
                        self.archive(code=b'DXB')):
            with self.subTest(archive=archive), self.assertRaises(ValueError):
                exporter.associate_code(sample_maps(), archive)
        archive = self.archive()
        archive.shader_lookup.clear()
        with self.assertRaisesRegex(ValueError, 'absent'):
            exporter.associate_code(sample_maps(), archive)

    def test_output_cannot_escape_generated_or_replace_source(self):
        target = exporter.GENERATED / 'terrain-test.json'
        with self.assertRaisesRegex(ValueError, 'overwrite'):
            exporter.checked_outputs([target], [target])
        with self.assertRaisesRegex(ValueError, 'distinct'):
            exporter.checked_outputs([target, target], [])
        with self.assertRaisesRegex(ValueError, 'generated'):
            exporter.checked_outputs([exporter.HERE / 'terrain-test.json'], [])

    def test_map_locations_must_start_at_resource_zero_and_be_ordered(self):
        for offsets in ([1], [0, 0], [0, 100, 50]):
            with self.subTest(offsets=offsets), self.assertRaisesRegex(ValueError, 'locations'):
                exporter.decode_maps(b'x' * 200, dict(locations=[dict(offset=x) for x in offsets]))

    def test_original_d4_preserves_both_maps_vt_rvt_vf_and_archive_bytes(self):
        framing_path = exporter.GENERATED / 'apollo-material-evidence/d4-resource.json'
        uexp = exporter.EXTRACTED / (exporter.PACKAGE.removeprefix('/Game/') + '.uexp')
        archive = exporter.EXTRACTED / 'ShaderArchive-FortniteGame-PCD3D_SM5.ushaderbytecode'
        if not all(path.is_file() for path in (framing_path, uexp, archive)):
            self.skipTest('original D4 extraction and resource receipt are not present')
        bundle = exporter.create_bundle(SimpleNamespace(framing=framing_path,
            uexp=uexp, package_header=uexp.with_suffix('.umap'), shader_archive=archive, decoded=None))
        maps = bundle['decoded']['maps']
        self.assertEqual([(m['offset'], m['end']) for m in maps], [(0, 50634), (50634, 101268)])
        self.assertEqual(len(bundle['record_code_associations']), 136)
        self.assertEqual(len(bundle['original_shader_code']), 43)
        code = {entry['library_index']: entry for entry in bundle['original_shader_code']}
        for entry in code.values():
            raw = bytes.fromhex(entry['code_raw_hex'])
            self.assertEqual(exporter.sha256(raw), entry['sha256'])
            self.assertEqual(len(raw), entry['bytes'])
        for location, record in exporter.record_locations(maps):
            reference = next(r for r in bundle['record_code_associations'] if r['location'] == location)
            self.assertEqual(reference['output_hash'], record['output_hash']['hash'])
            self.assertEqual(reference['sha256'], code[reference['library_index']]['sha256'])
            vf = record['vertex_factory_parameters']
            if record['frequency'] == 0:
                self.assertEqual(list(vf['landscape_parameters']), ['lod_values', 'forced_lod',
                    'lod_tessellation', 'neighbor_section_lod', 'lod_bias', 'section_lods'])
        for entry in maps:
            material = entry['decoded']
            self.assertEqual(material['shader_record_count'], 68)
            self.assertEqual(sum(len(m.get('pipelines', [])) for m in material['mesh_shader_maps']), 6)
            stack = material['virtual_texture_stacks']['stacks'][0]
            self.assertEqual(stack['layer_uniform_expression_indices'], [0, 1, 2])
            self.assertEqual(stack['preallocated_stack_texture_index'], 164)
            rvt = [e for e in material['uniform_expression_arrays']['vector']['expressions']
                   if e['type'] == 'FMaterialUniformExpressionRuntimeVirtualTextureUniform']
            self.assertEqual([e['vector_index'] for e in rvt], [2, 1, 0])
            self.assertTrue(all(e['texture_index'] == 164 for e in rvt))
        payload = bytes.fromhex(bundle['resource_raw_hex'])
        changed = copy.deepcopy(bundle['framing'])
        changed['locations'][1]['offset'] += 1
        with self.assertRaises(ValueError):
            exporter.decode_maps(payload, changed)
        changed = copy.deepcopy(bundle['framing'])
        changed['locations'][1]['quality_level'] = 0
        with self.assertRaisesRegex(ValueError, 'location'):
            exporter.decode_maps(payload, changed)
        # Reject a plausible decoded receipt that substitutes a live RVT owner.
        changed = copy.deepcopy(bundle['decoded'])
        changed['maps'][1]['decoded']['uniform_expression_arrays']['vector']['expressions'][1]['parameter']['index'] = 0
        with tempfile.TemporaryDirectory(dir=exporter.GENERATED) as directory:
            receipt = Path(directory) / 'substituted-maps.json'
            receipt.write_text(json.dumps(changed))
            with self.assertRaisesRegex(ValueError, 'decoded shader maps differ'):
                exporter.create_bundle(SimpleNamespace(framing=framing_path, uexp=uexp,
                    package_header=uexp.with_suffix('.umap'), shader_archive=archive, decoded=receipt))


if __name__ == '__main__':
    unittest.main()
