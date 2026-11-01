#!/usr/bin/env python3
"""Recheck the bounded material deliverables counted by the effort dashboard."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

HERE = Path(__file__).resolve().parent
GENERATED = HERE.parent / 'generated'
ROOT = GENERATED / 'material_pipeline'


def sibling(name):
    spec = importlib.util.spec_from_file_location(name, HERE / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


bundle = sibling('export_ingress_bundle')
reflection = sibling('reflect_mapped_spirv')
evaluator = sibling('evaluate_uniforms')


def digest(raw):
    return hashlib.sha256(raw).hexdigest()


def read(name):
    return json.loads((ROOT / name).read_bytes())


def check_maps():
    identities, records = set(), 0
    archive_path = Path(read('locker-legacy-prefix.json')['shader_library']['path'])
    archive_hash = bundle.library_reader.sha256(archive_path)
    with bundle.library_reader.ShaderArchive(archive_path, layout='fortnite-12.60') as archive:
        for name in ('donut', 'locker', 'menu-button', 'power-toast'):
            framing, recorded = read(name + '-inline-resource.json'), read(name + '-legacy-prefix.json')
            source = archive_path.parent / (framing['package'].removeprefix('/Game/') + '.uexp')
            parsed = bundle.decoder.parse_prefix(bundle.validate_source(source.read_bytes(), framing), framing['names'])
            bundle.decoder.associate_library(parsed, archive)
            for key, value in parsed.items():
                if key == 'original_serializer_evidence':
                    assert all(recorded[key].get(k) == v for k, v in value.items()), key
                else:
                    assert recorded.get(key) == value, (name, key)
            assert archive_hash == recorded['shader_library']['sha256']
            assert parsed['complete_serialized_shader_map'] and parsed['undecoded_bytes'] == 0
            rows = bundle.all_records(parsed)
            assert len(rows) == 19
            records += len(rows)
            identities.update(row['library_association']['index'] for row in rows)
    assert records == 76 and len(identities) == 44
    return dict(maps=4, shader_records=records, unique_library_shaders=len(identities), exact_resource_eof=True)


def check_translation():
    mapped = read('frontend-mapped/mapped-translation-report.json')
    assert mapped['source_report_sha256'] == digest((ROOT / 'frontend-game-library.json').read_bytes())
    assert mapped['d3d_report_sha256'] == digest((ROOT / 'd3d-wrapper-inspection.json').read_bytes())
    assert mapped['selected_shaders'] == mapped['validated_shaders'] == 44 and not mapped['truncated']
    descriptors, entries = 0, 0
    validator = ROOT / 'tools/root/usr/bin/spirv-val'
    for row in mapped['records']:
        raw = Path(row['output']).read_bytes()
        assert digest(raw) == row['output_sha256']
        assert digest(Path(row['input']).read_bytes()) == row['input_sha256']
        assert row['validated'] and not row['compiler_diagnostics'] and not row['unmatched_resource_table_entries']
        subprocess.run([str(validator), '--target-env', 'vulkan1.0', row['output']],
                       check=True, capture_output=True, timeout=30)
        actual = reflection.Module(raw).descriptors
        assert len(actual) == len(row['bindings'])
        for binding in row['bindings']:
            expected = binding['spirv_descriptor']
            assert any(all(value.get(k) == v for k, v in expected.items()) for value in actual)
            entries += len(binding['resource_table_entries'])
        descriptors += len(actual)
    assert descriptors == 180 and entries == 64
    return dict(shaders=44, descriptors=descriptors, matched_resource_entries=entries,
                unmatched_resource_entries=0, spirv_validator='vulkan1.0', compiler_diagnostics=0)


def check_uniforms():
    sizes = dict(donut=96, locker=320, **{'menu-button': 288, 'power-toast': 192})
    for name, size in sizes.items():
        material, receipt = read(name + '-legacy-prefix.json'), read(name + '-uniform-defaults.json')
        raw = evaluator.evaluate_report(material)[0]
        assert raw == (ROOT / (name + '-uniform-defaults.bin')).read_bytes()
        assert len(raw) == size == receipt['numeric_bytes']
        assert digest(raw) == receipt['numeric_bytes_sha256']
        assert receipt['resource_sha256'] == material['resource_sha256']
        assert all(record['debug_uniform_buffer_layout']['resource_offsets'][0] == size
                   for record in bundle.all_records(material))
    return dict(layouts=4, numeric_bytes=sizes, scope='Serialized defaults; original floating-point parity and texture values unverified')


def check_draw():
    artifacts = GENERATED / 'frontend-probes/legacy-material-artifacts-2'
    receipt = json.loads((artifacts / 'LockerQuad.json').read_text(encoding='utf-8-sig'))
    assert (receipt['width'], receipt['height'], receipt['format']) == (64, 64, 'RGBA8')
    red, green = [(artifacts / ('LockerQuad-' + color + '.rgba')).read_bytes() for color in ('Red', 'Green')]
    assert len(red) == len(green) == 64 * 64 * 4
    assert hashlib.sha1(red).hexdigest() == receipt['red_sha1'].lower()
    assert hashlib.sha1(green).hexdigest() == receipt['green_sha1'].lower()
    changed = sum(red[i:i+4] != green[i:i+4] for i in range(0, len(red), 4))
    assert changed == receipt['changed_pixels'] == 2832
    log_path = GENERATED / 'frontend-probes/legacy-material-runtime-draw-1.log'
    log = log_path.read_text(errors='replace')
    for marker in ('eight pre-freeze live uniform cases passed', 'eight post-freeze live uniform cases passed',
                   'detached map and seven RHI shaders constructed', 'original Slate quad readback changed 2832 pixels'):
        assert marker in log, marker
    subprocess.run(['python3', '-I', str(HERE / 'verify_runtime_artifacts.py'),
        '--fixture', str(ROOT / 'locker-runtime-fixture.json'), '--artifacts', str(artifacts),
        '--report', str(GENERATED / 'frontend-probes/dashboard-locker-artifact-validation.json')],
        check=True, capture_output=True, timeout=60)
    return dict(pre_freeze_cases=8, post_freeze_cases=8, changed_pixels=changed,
        log_sha256=digest(log_path.read_bytes()), scope='Detached Locker controlled draw; asset attachment and original-client visual parity are separate gates')


CHECKS = dict(material_original_maps=check_maps, material_shader_translation=check_translation,
              material_uniform_evaluation=check_uniforms, material_locker_draw=check_draw)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('check', choices=(*CHECKS, 'all'))
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    try:
        if args.report and GENERATED.resolve() not in args.report.resolve().parents:
            raise ValueError('verification receipt must stay under generated')
        names = CHECKS if args.check == 'all' else (args.check,)
        results = {name: CHECKS[name]() for name in names}
        output = dict(verifier_sha256=digest(Path(__file__).read_bytes()), checks=results,
                      scope='Bounded engineering deliverables; no product milestone or full parity claim')
        if args.report:
            args.report.parent.mkdir(parents=True, exist_ok=True)
            args.report.write_text(json.dumps(output, indent=2) + '\n')
        print(json.dumps(output, sort_keys=True))
    except (AssertionError, OSError, ValueError, KeyError, TypeError, subprocess.SubprocessError) as error:
        parser.exit(1, f'evidence verification failed: {error}\n')


if __name__ == '__main__':
    main()
