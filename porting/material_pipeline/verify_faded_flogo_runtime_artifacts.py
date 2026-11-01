#!/usr/bin/env python3
"""Verify the original named DXT5 texture, three sampling controls and seven shaders."""
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
FIXTURE_SHA256 = 'a17dc9d8b8adddbe95c953695804a56620096681406a5a7d3cf2a3fd98f396f9'
spec = importlib.util.spec_from_file_location('common_verifier', HERE / 'verify_runtime_artifacts.py')
common = importlib.util.module_from_spec(spec)
spec.loader.exec_module(common)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def check_compressed_source(fixture, uasset, uexp):
    header=uasset.read_bytes(); data=uexp.read_bytes()
    if (len(header)!=904 or len(data)!=262509 or sha(header)!='407da51def5b04af628df90f7e301476976344644bf6edba2cf2e37edc5ac293' or
            sha(data)!='93d7fb119a7a9f153c3410c23f761ab51335c8385bdf716a5e6fcec43a8b8a17'):
        raise ValueError('actual staged texture package hashes differ from original')
    source=fixture['texture_contract']['source_platform_data']; start=source['stored_bulk_offset']-len(header); count=source['compressed_bytes']
    if start<0 or count!=262144 or start+count>len(data): raise ValueError('compressed original texture range invalid')
    blocks=data[start:start+count]
    if sha(blocks)!=source['compressed_sha256']: raise ValueError('staged original BC3 mip differs from source receipt')
    return dict(uasset_sha256=sha(header),uexp_sha256=sha(data),compressed_sha256=sha(blocks),compressed_bytes=len(blocks))


def check_log(text, fixture):
    if len(fixture['owner_parameter_expectations']) != 14:
        raise ValueError('unexpected source instance override count')
    expected = [
        'Legacy material ingress: verified 14 source override identities and values',
        'Legacy material ingress: real parent map complete and attached on game/render threads; instance overrides verified',
        'Legacy material probe: %d owner live uniform cases passed' % len(fixture['uniform_cases']),
        'Legacy material probe: %d plain texture owner live uniform cases passed' % len(fixture['owner_uniform_cases']),
        'Legacy material ingress: Flogo owner complete and attached on both threads; seven shaders and controlled source sampling passed',
    ]
    if any(text.count(line)!=1 for line in expected):
        raise ValueError('actual parent/child attachment or full numeric override log receipt differs')
    return dict(verified_source_overrides=14,graph_cases=len(fixture['uniform_cases']),owner_cases=len(fixture['owner_uniform_cases']))


def check_sampling(fixture, artifacts, profile):
    contract = fixture['texture_contract']; expressions = fixture['texture_2d_parameters']
    identity = dict(name=dict(name='TextureMask',number=0),association=2,index=-1)
    if (contract.get('expression_kind') != 'TextureParameter' or contract.get('parameter') != identity or
            not contract['named_overrides_allowed'] or len(expressions)!=1 or
            expressions[0].get('expression_kind')!='TextureParameter' or expressions[0].get('parameter')!=identity or
            expressions[0].get('texture_index')!=0 or fixture['ordered_texture_references'] != [contract['default_texture_path']] or
            contract['override_texture_path']!=contract['default_texture_path']):
        raise ValueError('expected original named texture identity and source references')
    meta = json.loads((artifacts/'FlogoTexture.json').read_text(encoding='utf-8-sig'))
    layout=fixture['expected_layout']; layout_hash=common.uniform_layout_hash(layout['constant_bytes'],layout['resource_offsets'],layout['resource_types'])
    resources=[dict(offset=o,type=t) for o,t in zip(layout['resource_offsets'],layout['resource_types'])]
    if (meta['texture_path']!=contract['default_texture_path'] or meta['parent_path']!='/Game/Athena/UI/Season/Art/M_UI_AthenaSeason_Stylized_Font.M_UI_AthenaSeason_Stylized_Font' or
            meta['width']!=1024 or meta['height']!=256 or meta['format']!='PF_DXT5' or meta['srgb'] is not True or meta['mips']!=1 or
            meta['indexed_default_match'] is not True or meta['named_override_match'] is not True or meta['named_proxy_calls']<1 or
            meta['constant_bytes']!=layout['constant_bytes'] or meta['layout_hash']!=layout_hash or meta['resources']!=resources):
        raise ValueError('actual parent/default/named texture or compiled layout differs')
    source = contract['source_platform_data']; texture=contract['default']
    if (source['compressed_sha256']!='660f15379061b43dcf835dcde3b2b1f141361e32c4f78bd578013d596c28a4d3' or
            source['compressed_bytes']!=262144 or source['stored_bulk_offset']!=1241 or source['mip_count']!=1 or
            texture['rgbaSha256']!='cdab3ce448c5555adebdf2de8a3423f234107a22d2636806b9aca58b6dffe842'):
        raise ValueError('pinned original BC3 source receipt differs')
    draw=json.loads((artifacts/'FlogoDraw.json').read_text(encoding='utf-8-sig'))
    if (profile not in ('defaults','combined') or draw['numeric_profile']!=profile or
            draw['width']!=64 or draw['height']!=64 or draw['format']!='RGBA8' or
            draw['source_mips_uploaded']!=1 or draw['source_texture_format']!='BC3_SRGB' or draw['named_proxy_calls']<1):
        raise ValueError('sampling control profile or frame metadata differs')
    images=[]
    for label in ['Source','Copy','Checker']:
        data=(artifacts/('FlogoDraw-'+label+'.rgba')).read_bytes()
        if len(data)!=64*64*4 or hashlib.sha1(data).hexdigest()!=draw[label.lower()+'_sha1'].lower():
            raise ValueError('sampling control readback size/hash differs')
        images.append(data)
    if images[0]!=images[1]: raise ValueError('actual source and exact BC3 source-copy draws differ')
    changed=sum(images[0][i:i+4]!=images[2][i:i+4] for i in range(0,len(images[0]),4))
    if changed<=0 or changed!=draw['checker_changed_pixels']: raise ValueError('checker substitution does not match recorded pixel delta')
    return dict(layout_hash=layout_hash, named_proxy_calls=meta['named_proxy_calls'],checker_changed_pixels=changed,
                source_sha1=draw['source_sha1'],copy_sha1=draw['copy_sha1'],checker_sha1=draw['checker_sha1'],numeric_profile=profile)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--fixture', required=True, type=Path)
    parser.add_argument('--artifacts', required=True, type=Path)
    parser.add_argument('--log', required=True, type=Path)
    parser.add_argument('--texture-uasset', required=True, type=Path)
    parser.add_argument('--texture-uexp', required=True, type=Path)
    parser.add_argument('--profile', choices=['defaults', 'combined'], default='defaults')
    parser.add_argument('--report', required=True, type=Path)
    args = parser.parse_args()
    try:
        if GENERATED.resolve() not in args.report.resolve().parents:
            raise ValueError('report must stay in porting/generated')
        raw = args.fixture.read_bytes()
        if sha(raw) != FIXTURE_SHA256:
            raise ValueError('fixture differs from the independently audited original FadedFlogo fixture')
        fixture = json.loads(raw)
        fixture = fixture.get('runtime_fixture', fixture)
        owner_receipt = check_log(args.log.read_text(errors='replace'), fixture)
        source_receipt = check_compressed_source(fixture, args.texture_uasset, args.texture_uexp)
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
                      artifacts=str(args.artifacts.resolve()), runtime_log=str(args.log.resolve()), runtime_log_sha256=sha(args.log.read_bytes()), owner=owner_receipt, source=source_receipt, sampling=sampling,
                      shader_report=str(shader_report.resolve()), shader_report_sha256=sha(shader_report.read_bytes()),
                      shader_fixture_sha256=sha(shader_fixture.read_bytes()), verified_shader_payloads=7,
                      scope='Original named texture binding and diagnostic BC3 sampling controls; seven shader instructions/header bindings. Raw GPU-compressed bytes, original-client pixel and interactive UI parity remain unverified.')
        args.report.write_text(json.dumps(result, indent=2) + '\n')
        print(json.dumps(result))
    except (OSError, ValueError, KeyError, TypeError, subprocess.TimeoutExpired) as error:
        parser.exit(1, 'error: ' + str(error) + '\n')


if __name__ == '__main__':
    main()
