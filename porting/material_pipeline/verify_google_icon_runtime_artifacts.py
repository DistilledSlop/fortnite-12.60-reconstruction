#!/usr/bin/env python3
"""Verify the original named ten-mip BC1 texture, three sampling controls and seven shaders."""
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
FIXTURE_SHA256 = '21a60f0dc74358cc8801bd16875af8a4b1c2e47ee7355e7533476c1e6fc87325'
spec = importlib.util.spec_from_file_location('common_verifier', HERE / 'verify_runtime_artifacts.py')
common = importlib.util.module_from_spec(spec)
spec.loader.exec_module(common)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def check_compressed_source(fixture,uasset,uexp,ubulk):
    spec=importlib.util.spec_from_file_location('google_frame',Path(__file__).with_name('google_frame.py'));frame=importlib.util.module_from_spec(spec);spec.loader.exec_module(frame)
    native=frame.frame(uasset.read_bytes(),uexp.read_bytes(),ubulk.read_bytes(),fixture['texture_contract']['default'])
    if native!=fixture['texture_contract']['source_platform_data']:raise ValueError('original signed mip framing differs')
    return dict(uasset_sha256=sha(uasset.read_bytes()),uexp_sha256=sha(uexp.read_bytes()),ubulk_sha256=sha(ubulk.read_bytes()),mip_count=10,mip_receipts=native['mip_receipts'])

def check_log(text,fixture):
    expected=['Legacy material probe: %d owner live uniform cases passed'%len(fixture['uniform_cases']),
              'Legacy material probe: %d plain texture owner live uniform cases passed'%len(fixture['owner_uniform_cases']),
              'Legacy material ingress: Google owner complete and attached on both threads; seven shaders and original ten-mip source/copy controls passed']
    if any(text.count(x)!=1 for x in expected):raise ValueError('actual owner/GT RT/shader/numeric cases log differs')
    return dict(graph_cases=len(fixture['uniform_cases']),owner_cases=len(fixture['owner_uniform_cases']))

def check_sampling(fixture,artifacts,profile):
    identity=dict(name=dict(name='Texture',number=0),association=2,index=-1)
    expression=dict(expression_kind='TextureParameter',parameter=identity,texture_index=0,texture_layer_index=-1,page_table_layer_index=-1,sampler_source=0,virtual_texture=False,default_texture_path='/Game/UI/Login/Art/T_TempIcon_Login_Google.T_TempIcon_Login_Google')
    c=fixture['texture_contract'];layout=dict(constant_bytes=80,resource_offsets=[48,56,64,72],resource_types=[5,8,8,8]);h=common.uniform_layout_hash(80,[48,56,64,72],[5,8,8,8])
    if fixture['texture_2d_parameters']!=[expression] or fixture['ordered_texture_references']!=[expression['default_texture_path']] or c['expression_kind']!='TextureParameter' or c['parameter']!=identity or c['named_overrides_allowed'] is not True or fixture['expected_layout']!=layout:raise ValueError('exact original named texture/asset sampler fixture differs')
    meta=json.loads((artifacts/'GoogleTexture.json').read_text(encoding='utf-8-sig'))
    required=dict(texture_path=expression['default_texture_path'],parameter_name='Texture',parameter_number=0,association=2,index=-1,width=512,height=512,format='PF_DXT1',srgb=True,mips=10,resident_mips_after=10,sampler_source=0,asset_sampler_binding_match=True,indexed_default_match=True,raw_bc1_gpu_readback_measured=False,layout_hash=h,constant_bytes=80,resource_offsets=[48,56,64,72],resource_types=[5,8,8,8])
    if any(meta.get(k)!=v or (isinstance(v,bool) and meta.get(k) is not v) for k,v in required.items()) or meta['named_proxy_calls']<1 or not 0<=meta['resident_mips_before']<=10:raise ValueError('actual named/indexed/default/RHI/sampler/layout/residency receipt differs')
    draw=json.loads((artifacts/'GoogleDraw.json').read_text(encoding='utf-8-sig'))
    required=dict(width=64,height=64,format='RGBA8',numeric_profile=profile,source_mips_uploaded=10,source_texture_format='BC1_SRGB',source_copy_equal=True,checker_delta_policy='required_positive' if profile=='combined' else 'observational',raw_bc1_gpu_readback_measured=False)
    if profile not in ('defaults','combined') or any(draw.get(k)!=v or (isinstance(v,bool) and draw.get(k) is not v) for k,v in required.items()) or draw['named_proxy_calls']<1:raise ValueError('sampling profile/residency/copy receipt differs')
    images=[]
    for label in ['Source','Copy','Checker']:
        raw=(artifacts/('GoogleDraw-'+label+'.rgba')).read_bytes()
        if len(raw)!=16384 or hashlib.sha1(raw).hexdigest()!=draw[label.lower()+'_sha1'].lower():raise ValueError('sampling frame size/hash differs')
        images.append(raw)
    if images[0]!=images[1]:raise ValueError('actual source/exact original all-mip BC1 copy draws differ')
    changed=sum(images[0][i:i+4]!=images[2][i:i+4] for i in range(0,16384,4))
    if changed!=draw['checker_changed_pixels'] or (profile=='combined' and changed==0):raise ValueError('actual checker delta/policy differs')
    return dict(layout_hash=h,named_proxy_calls=meta['named_proxy_calls'],numeric_profile=profile,checker_changed_pixels=changed,source_sha1=draw['source_sha1'],copy_sha1=draw['copy_sha1'],checker_sha1=draw['checker_sha1'],raw_bc1_gpu_readback_measured=False)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--fixture', required=True, type=Path)
    parser.add_argument('--artifacts', required=True, type=Path)
    parser.add_argument('--log', required=True, type=Path)
    parser.add_argument('--texture-uasset', required=True, type=Path)
    parser.add_argument('--texture-uexp', required=True, type=Path)
    parser.add_argument('--texture-ubulk',required=True,type=Path)
    parser.add_argument('--profile', choices=['defaults', 'combined'], default='defaults')
    parser.add_argument('--report', required=True, type=Path)
    args = parser.parse_args()
    try:
        if GENERATED.resolve() not in args.report.resolve().parents:
            raise ValueError('report must stay in porting/generated')
        raw = args.fixture.read_bytes()
        if sha(raw) != FIXTURE_SHA256:
            raise ValueError('fixture differs from the independently audited original Google fixture')
        fixture = json.loads(raw)
        fixture = fixture.get('runtime_fixture', fixture)
        owner_receipt = check_log(args.log.read_text(errors='replace'), fixture)
        source_receipt = check_compressed_source(fixture,args.texture_uasset,args.texture_uexp,args.texture_ubulk)
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
                      scope='Original named texture binding and diagnostic BC1 all-mip sampling controls; seven shader instructions/header bindings. Raw GPU-compressed bytes, original-client pixel and interactive UI parity remain unverified.')
        args.report.write_text(json.dumps(result, indent=2) + '\n')
        print(json.dumps(result))
    except (OSError, ValueError, KeyError, TypeError, subprocess.TimeoutExpired) as error:
        parser.exit(1, 'error: ' + str(error) + '\n')


if __name__ == '__main__':
    main()
