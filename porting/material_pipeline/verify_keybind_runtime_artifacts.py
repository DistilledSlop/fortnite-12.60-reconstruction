#!/usr/bin/env python3
"""Verify the original regular texture, three sampling controls and seven shaders."""
import argparse
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys

HERE = Path(__file__).resolve().parent
GENERATED = HERE.parent / 'generated'
FIXTURE_SHA256 = '26a3e93fc381a7eb20bc42f0f0bcf6ad453adf06348f199848cd5c8437dd6f02'
spec = importlib.util.spec_from_file_location('common_verifier', HERE / 'verify_runtime_artifacts.py')
common = importlib.util.module_from_spec(spec)
spec.loader.exec_module(common)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def check_sampling(fixture, artifacts, profile):
    contract = fixture['texture_contract']
    expressions = fixture['texture_2d_parameters']
    if (contract.get('expression_kind') != 'Texture' or contract.get('parameter') is not None
            or contract['named_overrides_allowed'] or len(expressions) != 1
            or expressions[0].get('expression_kind') != 'Texture' or expressions[0].get('parameter') is not None
            or fixture['ordered_texture_references'] != [contract['default_texture_path']]):
        raise ValueError('expected original indexed texture without a parameter identity')
    meta = json.loads((artifacts / 'KeybindTexture.json').read_text(encoding='utf-8-sig'))
    layout = fixture['expected_layout']
    layout_hash = common.uniform_layout_hash(layout['constant_bytes'], layout['resource_offsets'], layout['resource_types'])
    expected_resources = [dict(offset=o, type=t) for o, t in zip(layout['resource_offsets'], layout['resource_types'])]
    if (meta['texture_path'] != contract['default_texture_path'] or meta['width'] != 64 or meta['height'] != 64
            or meta['format'] != 'PF_B8G8R8A8' or meta['srgb'] is not True or meta['mips'] != 1
            or meta['named_proxy_calls'] != 0 or meta['constant_bytes'] != layout['constant_bytes']
            or meta['layout_hash'] != layout_hash or meta['resources'] != expected_resources):
        raise ValueError('actual texture source, RHI layout or named lookup differs')
    default = contract['default']
    source = Path(default['rgba']).read_bytes()
    pixels = (artifacts / 'KeybindTexture.rgba').read_bytes()
    if (len(source) != 64 * 64 * 4 or pixels != source or sha(source) != default['rgbaSha256']
            or meta['rgba_sha256'].lower() != sha(pixels)):
        raise ValueError('GPU source pixels differ from the original canonical RGBA')
    draw = json.loads((artifacts / 'KeybindDraw.json').read_text(encoding='utf-8-sig'))
    if (profile not in ('defaults', 'combined') or draw['numeric_profile'] != profile
            or draw['width'] != 64 or draw['height'] != 64 or draw['format'] != 'RGBA8'
            or draw['named_proxy_calls'] != 0):
        raise ValueError('sampling control profile or frame metadata differs')
    images = []
    for label in ['Source', 'Copy', 'Checker']:
        data = (artifacts / ('KeybindDraw-' + label + '.rgba')).read_bytes()
        if len(data) != 64 * 64 * 4 or hashlib.sha1(data).hexdigest() != draw[label.lower() + '_sha1'].lower():
            raise ValueError('sampling control readback size/hash differs')
        images.append(data)
    if images[0] != images[1]:
        raise ValueError('original source and independent exact-copy draws differ')
    changed = sum(images[0][i:i+4] != images[2][i:i+4] for i in range(0, len(images[0]), 4))
    if changed <= 0 or changed != draw['checker_changed_pixels']:
        raise ValueError('checker substitution does not match the recorded pixel change')
    return dict(original_rgba_sha256=sha(pixels), layout_hash=layout_hash, numeric_profile=profile,
                original_and_copy_identical=True, checker_changed_pixels=changed, named_proxy_calls=0)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--fixture', required=True, type=Path)
    parser.add_argument('--artifacts', required=True, type=Path)
    parser.add_argument('--profile', choices=['defaults', 'combined'], default='defaults')
    parser.add_argument('--report', required=True, type=Path)
    args = parser.parse_args()
    try:
        if GENERATED.resolve() not in args.report.resolve().parents:
            raise ValueError('report must stay in porting/generated')
        raw = args.fixture.read_bytes()
        if sha(raw) != FIXTURE_SHA256:
            raise ValueError('fixture differs from the independently audited original Keybind fixture')
        fixture = json.loads(raw)
        fixture = fixture.get('runtime_fixture', fixture)
        sampling = check_sampling(fixture, args.artifacts, args.profile)
        shader_fixture = args.report.with_name(args.report.stem + '-shader-fixture.json')
        shader_report = args.report.with_name(args.report.stem + '-shaders.json')
        if any(p.exists() for p in [args.report, shader_fixture, shader_report]):
            raise ValueError('choose fresh verification outputs')
        # The historical common CLI expects DonutMaterialLayout.json whenever
        # texture_contract exists. Its shader/header checks are reusable as-is.
        # Verify the full original texture contract above, then give that CLI
        # an explicitly shader-only projection, with every shader/layout retained.
        projection = copy.deepcopy(fixture)
        del projection['texture_contract']
        shader_fixture.write_text(json.dumps(projection, indent=2, allow_nan=False) + '\n')
        command = [sys.executable, '-I', str(HERE / 'verify_runtime_artifacts.py'),
                   '--fixture', str(shader_fixture), '--artifacts', str(args.artifacts), '--report', str(shader_report)]
        run = subprocess.run(command, capture_output=True, text=True, timeout=120)
        if run.returncode:
            raise ValueError('shader verification failed: ' + run.stderr.strip())
        shaders = json.loads(shader_report.read_text())
        records = shaders['records']
        if (len(records) != 7 or shaders['layout_hash'] != sampling['layout_hash']
                or shaders['fixture_sha256'] != sha(shader_fixture.read_bytes())
                or shaders['verifier_sha256'] != sha((HERE / 'verify_runtime_artifacts.py').read_bytes())
                or {r['shader_type'] for r in records} != {s['record']['type']['name'] for s in fixture['shaders']}
                or any(r['instructions_preserved'] is not True or r['validator_exit'] != 0 for r in records)):
            raise ValueError('expected seven independently verified original shader payloads')
        result = dict(passed=True, fixture=str(args.fixture.resolve()), fixture_sha256=sha(raw),
                      artifacts=str(args.artifacts.resolve()), sampling=sampling,
                      shader_report=str(shader_report.resolve()), shader_report_sha256=sha(shader_report.read_bytes()),
                      shader_fixture_sha256=sha(shader_fixture.read_bytes()), verified_shader_payloads=7,
                      scope='Original indexed texture GPU bytes, typed layout and diagnostic sampling controls; seven shader instructions/header bindings. Original-client pixel and interactive UI parity remain unverified.')
        args.report.write_text(json.dumps(result, indent=2) + '\n')
        print(json.dumps(result))
    except (OSError, ValueError, KeyError, TypeError, subprocess.TimeoutExpired) as error:
        parser.exit(1, 'error: ' + str(error) + '\n')


if __name__ == '__main__':
    main()
