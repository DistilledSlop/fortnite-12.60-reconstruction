#!/usr/bin/env python3
"""Verify real Social atlas/size output, separate VM controls and seven shaders."""
import argparse
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import sys

HERE = Path(__file__).resolve().parent
GENERATED = HERE.parent / 'generated'
FIXTURE_SHA256 = 'db3c8a1bf00347d86e9d47b9ea1aff778a21cf9d5dff0e0f6601eaec61e45cc6'
ATLAS = '/Game/UI/SocialPanel/Art/CA_SocialPanel.CA_SocialPanel'
spec = importlib.util.spec_from_file_location('common', HERE / 'verify_runtime_artifacts.py')
common = importlib.util.module_from_spec(spec)
spec.loader.exec_module(common)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def read_json(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def check_source(fixture, asset, exports):
    header, payload = asset.read_bytes(), exports.read_bytes()
    if (len(header) != 2802 or sha(header) != '39d404e1c6b2af2cd07a4786890f47b7c7fa72757ffbad3ef65cb7e517972e23'
            or len(payload) != 8549 or sha(payload) != '45a48f299e3cf56282695ae000376f3105dbaaf8ab9cf83d829a97a73bc21037'):
        raise ValueError('actual staged atlas pair differs from original source')
    source = fixture['texture_contract']['source_platform_data']
    start = source['stored_bulk_offset'] - len(header)
    if (start != 329 or source['raw_bytes'] != 8192 or source['format'] != 'PF_FloatRGBA'
            or source['mip_count'] != 1 or source['first_mip'] != 0
            or (source['logical_width'], source['logical_height']) != (32, 32)):
        raise ValueError('original atlas platform frame differs')
    raw = payload[start:start + 8192]
    if sha(raw) != source['raw_sha256']:
        raise ValueError('original atlas half-float mip differs')
    return raw, dict(uasset_sha256=sha(header), uexp_sha256=sha(payload), raw_half_sha256=sha(raw))


def check_vm_controls(artifacts):
    scratch = read_json(artifacts / 'TextureSize2DScratchBits.json')
    if scratch['passed'] is not True or scratch['opcode'] != 42:
        raise ValueError('compiled scratch VM diagnostic failed')
    expected = {
        'real_source_in_scratch_context': ['42000000', '42000000', '00000000', '00000000'],
        'absent_index': ['00000000'] * 4,
        'null_resource': ['00000000'] * 4,
        'alternate_dimensions': ['40e00000', '41500000', '00000000', '00000000'],
        'restored_source': ['42000000', '42000000', '00000000', '00000000'],
    }
    cases = scratch['cases']
    if len(cases) != 5 or len({row['name'] for row in cases}) != 5:
        raise ValueError('scratch VM cases missing or duplicated')
    actual = {row['name']: [word.lower() for word in row['bits']] for row in cases}
    if actual != expected:
        raise ValueError('scratch dynamic, missing-resource or positive-zero bits differ')
    negative = read_json(artifacts / 'TextureSize2DRejectedForms.json')
    names = {'raw:' + name for name in ['property1', 'named source', 'scalar root', 'nested property', 'child source slot']}
    names |= {kind + ':' + name for kind in ['json', 'factory']
              for name in ['property1', 'bad normalized slot', 'scalar root', 'nested property', 'named source']}
    if (negative['passed'] is not True or len(negative['cases']) != 15
            or {row['name'] for row in negative['cases']} != names
            or any(row['rejected_at_expected_gate'] is not True for row in negative['cases'])):
        raise ValueError('unsupported original forms were not rejected at all three layers')
    return dict(scratch_cases=actual, rejected_form_count=15,
                scope='Compiled VM scratch tests; not a dynamic update of the original owner')


def check_sampling(fixture, artifacts, profile, source_half):
    contract = fixture['texture_contract']
    expression = fixture['texture_2d_parameters']
    root = fixture['nodes'][fixture['vector_roots'][2]]
    if (contract['expression_kind'] != 'Texture' or contract['parameter'] is not None
            or contract['named_overrides_allowed'] is not False or contract['default_texture_path'] != ATLAS
            or fixture['ordered_texture_references'] != [ATLAS] or len(expression) != 1
            or expression[0]['parameter'] is not None or expression[0]['expression_kind'] != 'Texture'
            or expression[0]['texture_index'] != 0 or expression[0]['sampler_source'] != 2
            or root != dict(type='TextureSize2D', texture_parameter_index=0)):
        raise ValueError('original indexed atlas/dynamic root contract differs')
    meta = read_json(artifacts / 'SocialTexture.json')
    layout = fixture['expected_layout']
    resources = [dict(offset=offset, type=kind) for offset, kind in zip(layout['resource_offsets'], layout['resource_types'])]
    layout_hash = common.uniform_layout_hash(layout['constant_bytes'], layout['resource_offsets'], layout['resource_types'])
    if (meta['texture_path'] != ATLAS or (meta['width'], meta['height']) != (32, 32)
            or meta['format'] != 'PF_FloatRGBA' or meta['srgb'] is not False or meta['mips'] != 1
            or meta['filter'] != 'TF_Bilinear' or meta['addressX'] != 'TA_Clamp' or meta['addressY'] != 'TA_Clamp'
            or meta['compressionSettings'] != 'TC_HDR'
            or meta['named_proxy_calls'] != 0 or meta['sampler_source'] != 2 or meta['bound_sampler_resource'] != 3
            or meta['root'] != 2 or [word.lower() for word in meta['root_bits']] != ['42000000', '42000000', '00000000', '00000000']
            or meta['constant_bytes'] != 160 or meta['layout_hash'] != layout_hash or meta['resources'] != resources):
        raise ValueError('real owner size bits, indexed binding or world-clamp layout differs')
    gpu_half = (artifacts / 'SocialTexture.rgba16f').read_bytes()
    if (len(source_half) != 8192 or gpu_half != source_half or meta['gpu_half_bytes'] != 8192
            or sha(gpu_half) != meta['gpu_half_sha256'].lower()
            or sha(gpu_half) != contract['source_platform_data']['raw_sha256']):
        raise ValueError('GPU atlas half-float bytes differ from original source')
    draw = read_json(artifacts / 'SocialDraw.json')
    if (profile not in ('defaults', 'combined') or draw['numeric_profile'] != profile
            or (draw['width'], draw['height'], draw['format']) != (64, 64, 'RGBA8')
            or draw['source_mips_uploaded'] != 1 or draw['source_texture_format'] != 'PF_FloatRGBA'
            or draw['bound_sampler_resource'] != 3 or draw['named_proxy_calls'] != 0
            or draw['source_copy_equal'] is not True or draw['checker_effect_required'] is not (profile == 'combined')):
        raise ValueError('source sampling controls or diagnostic policy differs')
    images = []
    for label in ['Source', 'Copy', 'Checker']:
        data = (artifacts / ('SocialDraw-' + label + '.rgba')).read_bytes()
        if len(data) != 16384 or hashlib.sha1(data).hexdigest() != draw[label.lower() + '_sha1'].lower():
            raise ValueError('source/control frame size or hash differs')
        images.append(data)
    if images[0] != images[1]:
        raise ValueError('real source and independent exact-copy draws differ')
    changed = sum(images[0][i:i + 4] != images[2][i:i + 4] for i in range(0, 16384, 4))
    if changed != draw['checker_changed_pixels'] or (profile == 'combined' and changed == 0):
        raise ValueError('observed checker effect or required combined sampling differs')
    return dict(layout_hash=layout_hash, gpu_half_sha256=sha(gpu_half), root_bits=meta['root_bits'],
                bound_sampler_resource=3, source_copy_equal=True, checker_changed_pixels=changed,
                numeric_profile=profile, default_delta_is_observational=True)


def check_log(text, fixture):
    expected = [
        'Legacy Social: actual indexed atlas, live root2 dimensions/+0 lanes and world-clamp resource3 match; scratch absent-index/null-resource/dimension controls passed',
        'Legacy material ingress: Social owner complete and attached on both threads; seven shaders, dynamic size root and original half-float sampling passed',
        'Legacy material probe: %d owner live uniform cases passed' % len(fixture['uniform_cases']),
        'Legacy material probe: %d plain texture owner live uniform cases passed' % len(fixture['owner_uniform_cases']),
    ]
    if any(text.count(line) != 1 for line in expected):
        raise ValueError('actual-owner attachment/size/numeric runtime log evidence differs')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ['fixture', 'artifacts', 'log', 'texture-uasset', 'texture-uexp', 'report']:
        parser.add_argument('--' + name, required=True, type=Path)
    parser.add_argument('--profile', choices=['defaults', 'combined'], default='defaults')
    args = parser.parse_args()
    try:
        if GENERATED.resolve() not in args.report.resolve().parents:
            raise ValueError('report must be under porting/generated')
        raw = args.fixture.read_bytes()
        if sha(raw) != FIXTURE_SHA256:
            raise ValueError('fixture differs from independently audited original Social fixture')
        fixture = json.loads(raw)
        check_log(args.log.read_text(errors='replace'), fixture)
        source, source_report = check_source(fixture, args.texture_uasset, args.texture_uexp)
        sampling = check_sampling(fixture, args.artifacts, args.profile, source)
        vm = check_vm_controls(args.artifacts)
        projection_path = args.report.with_name(args.report.stem + '-shader-fixture.json')
        shader_report = args.report.with_name(args.report.stem + '-shaders.json')
        if any(path.exists() for path in [args.report, projection_path, shader_report]):
            raise ValueError('choose fresh verification outputs')
        # The common CLI's texture path is Donut-specific. Check the complete
        # Social contract first; reuse its unmodified seven-payload checks on
        # a projection that removes only that already checked texture contract.
        projection = copy.deepcopy(fixture)
        del projection['texture_contract']
        projection_path.write_text(json.dumps(projection, indent=2, allow_nan=False) + '\n')
        command = [sys.executable, '-I', str(HERE / 'verify_runtime_artifacts.py'), '--fixture', str(projection_path),
                   '--artifacts', str(args.artifacts), '--report', str(shader_report)]
        run = subprocess.run(command, capture_output=True, text=True, timeout=120)
        if run.returncode:
            raise ValueError('shader verification failed: ' + run.stderr.strip())
        shaders = read_json(shader_report)
        if (len(shaders['records']) != 7 or shaders['layout_hash'] != sampling['layout_hash']
                or shaders['fixture_sha256'] != sha(projection_path.read_bytes())
                or shaders['verifier_sha256'] != sha((HERE / 'verify_runtime_artifacts.py').read_bytes())
                or {row['shader_type'] for row in shaders['records']} != {row['record']['type']['name'] for row in fixture['shaders']}
                or any(row['instructions_preserved'] is not True or row['validator_exit'] != 0 for row in shaders['records'])):
            raise ValueError('expected seven intact original shader payloads')
        result = dict(passed=True, fixture_sha256=sha(raw), source=source_report, sampling=sampling,
                      compiled_vm_controls=vm, verified_shader_payloads=7,
                      runtime_log=str(args.log.resolve()), runtime_log_sha256=sha(args.log.read_bytes()),
                      shader_report=str(shader_report.resolve()), shader_report_sha256=sha(shader_report.read_bytes()),
                      shader_projection_sha256=sha(projection_path.read_bytes()),
                      scope='Real atlas GPU/source bytes, original-owner indexed binding and size bits; separate compiled scratch VM diagnostics and sampling controls. No original-owner dynamic update, original-client pixel or interactive UI parity claim.')
        args.report.write_text(json.dumps(result, indent=2) + '\n')
        print(json.dumps(result))
    except (OSError, ValueError, KeyError, TypeError, subprocess.SubprocessError) as error:
        parser.exit(1, 'error: ' + str(error) + '\n')


if __name__ == '__main__':
    main()
