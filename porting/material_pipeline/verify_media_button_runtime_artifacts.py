#!/usr/bin/env python3
"""Verify the original Media button texture, sampling controls and shaders."""
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
FIXTURE_SHA256 = 'cf26cd2ac25ef38bda2219590fed359b298b9159dc334c4c118bbc175b9fec5f'
TEXTURE = '/Game/UI/Icons/T_UI_MediaPlayerButton_Layered_A.T_UI_MediaPlayerButton_Layered_A'
ASSET_SHA = '7fed5326ffdbf8e10bcd37f64eee4f3f8be41aca9593708a90f28e7046013c38'
EXPORT_SHA = '364e417cfee78e6e15ff3dc91bd800dd39d3babe3a47085b189c863778e34cd9'
BGRA_SHA = 'b842bc2606ab98a0f5c359e5e8b1b423a9af00d7544eed41900a7c7156b0ffc3'
RGBA_SHA = '2778f9ccfc8816a04839d2c12d595f9bce232e018eca9730e97d2071aed35610'
spec = importlib.util.spec_from_file_location('common', HERE / 'verify_runtime_artifacts.py')
common = importlib.util.module_from_spec(spec)
spec.loader.exec_module(common)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def read_json(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def check_source(fixture, asset, exports):
    header, payload = asset.read_bytes(), exports.read_bytes()
    if len(header) != 854 or sha(header) != ASSET_SHA or len(payload) != 65823 or sha(payload) != EXPORT_SHA:
        raise ValueError('actual staged texture source pair differs')
    source = fixture['texture_contract']['source_platform_data']
    if (source['stored_bulk_offset'] != 1113 or source['pixel_format'] != 'PF_B8G8R8A8'
            or source['mip_count'] != 1 or source['first_mip'] != 0 or source['virtual_texture'] is not False
            or (source['logical_width'], source['logical_height']) != (128, 128)
            or source['raw_bgra_sha256'] != BGRA_SHA or source['rgba_sha256'] != RGBA_SHA):
        raise ValueError('original texture source frame differs')
    start = 1113 - len(header)
    bgra = payload[start:start + 65536]
    if len(bgra) != 65536 or sha(bgra) != BGRA_SHA:
        raise ValueError('original BGRA mip differs')
    rgba = bytearray(len(bgra))
    for i in range(0, len(bgra), 4):
        rgba[i:i+4] = bytes((bgra[i+2], bgra[i+1], bgra[i], bgra[i+3]))
    if sha(rgba) != RGBA_SHA:
        raise ValueError('original canonical RGBA differs')
    return bytes(rgba), dict(uasset_sha256=sha(header), uexp_sha256=sha(payload), stored_bulk_offset=1113,
                             bgra_sha256=sha(bgra), rgba_sha256=sha(rgba))


def check_sampling(fixture, artifacts, profile, source):
    contract = fixture['texture_contract']
    expr = fixture['texture_2d_parameters']
    if (contract['expression_kind'] != 'Texture' or contract['parameter'] is not None
            or contract['named_overrides_allowed'] is not False or contract['default_texture_path'] != TEXTURE
            or fixture['ordered_texture_references'] != [TEXTURE] or len(expr) != 1
            or expr[0] != dict(expression_kind='Texture', parameter=None, texture_index=0,
                               texture_layer_index=-1, page_table_layer_index=-1, sampler_source=2,
                               virtual_texture=False, default_texture_path=TEXTURE)):
        raise ValueError('original indexed texture identity, index or sampler differs')
    default = contract['default']
    if ((default['width'], default['height'], default['pixelFormat'], default['srgb']) != (128, 128, 'PF_B8G8R8A8', True)
            or default['filter'] != 'TF_Default' or default['lodGroup'] != 'TEXTUREGROUP_World'
            or default['addressX'] != 'TA_Wrap' or default['addressY'] != 'TA_Wrap'
            or len(default['mips']) != 1 or default['rgbaSha256'] != RGBA_SHA):
        raise ValueError('original texture asset metadata differs')
    layout = fixture['expected_layout']
    if layout != dict(constant_bytes=80, resource_offsets=[48, 56, 64, 72], resource_types=[5, 8, 8, 8]):
        raise ValueError('original typed layout differs')
    layout_hash = common.uniform_layout_hash(80, layout['resource_offsets'], layout['resource_types'])
    if layout_hash != 89655543:
        raise ValueError('original layout hash differs')
    meta = read_json(artifacts / 'MediaTexture.json')
    resources = [dict(offset=o, type=t) for o,t in zip(layout['resource_offsets'], layout['resource_types'])]
    if (meta['texture_path'] != TEXTURE or (meta['width'],meta['height'],meta['format'],meta['srgb'],meta['mips']) != (128,128,'PF_B8G8R8A8',True,1)
            or meta['named_proxy_calls'] != 0 or meta['sampler_source'] != 2 or meta['world_clamp_binding_match'] is not True
            or meta['constant_bytes'] != 80 or meta['layout_hash'] != layout_hash or meta['resources'] != resources
            or meta['source_uasset_sha256'].lower() != ASSET_SHA or meta['source_uexp_sha256'].lower() != EXPORT_SHA
            or meta['source_bgra_sha256'].lower() != BGRA_SHA):
        raise ValueError('live indexed texture, world-clamp sampler or layout differs')
    gpu = (artifacts / 'MediaTexture.rgba').read_bytes()
    if len(source) != 65536 or sha(source) != RGBA_SHA or gpu != source or meta['rgba_sha256'].lower() != RGBA_SHA:
        raise ValueError('GPU RGBA differs from original source')
    draw = read_json(artifacts / 'MediaDraw.json')
    policy = 'required_positive' if profile == 'combined' else 'observational'
    if (profile not in ('defaults','combined') or draw['numeric_profile'] != profile or draw['checker_delta_policy'] != policy
            or (draw['width'], draw['height'], draw['format']) != (64,64,'RGBA8') or draw['named_proxy_calls'] != 0
            or draw['source_mips_uploaded'] != 1 or draw['source_texture_format'] != 'BGRA8_SRGB'
            or draw['source_copy_equal'] is not True or draw['checker_effect_required'] is not (profile == 'combined')):
        raise ValueError('draw profile, source format or sampling policy differs')
    images=[]
    for label in ('Source','Copy','Checker'):
        data=(artifacts / ('MediaDraw-' + label + '.rgba')).read_bytes()
        if len(data) != 16384 or hashlib.sha1(data).hexdigest() != draw[label.lower() + '_sha1'].lower():
            raise ValueError('draw frame size or hash differs')
        images.append(data)
    if images[0] != images[1]:
        raise ValueError('source and independent copy draws differ')
    changed=sum(images[0][i:i+4] != images[2][i:i+4] for i in range(0,16384,4))
    if (changed != draw['checker_changed_pixels'] or (profile == 'combined' and changed == 0)
            or draw['checker_effect_observed'] is not (changed > 0)):
        raise ValueError('checker change count or required combined effect differs')
    return dict(layout_hash=layout_hash, gpu_rgba_sha256=sha(gpu), numeric_profile=profile,
                source_copy_equal=True, checker_changed_pixels=changed, checker_delta_policy=policy,
                world_clamp_binding_match=True, named_proxy_calls=0)


def check_log(text, fixture, sampling):
    expected = [
        'Legacy material ingress: Media owner complete and attached on both threads; seven shaders and source/copy checks passed',
        'Legacy material probe: %d owner live uniform cases passed' % len(fixture['uniform_cases']),
        'Legacy material probe: %d plain texture owner live uniform cases passed' % len(fixture['owner_uniform_cases']),
        'Legacy Media texture: real indexed source, zero named proxy calls, GPU RGBA SHA256='+RGBA_SHA.upper(),
        'Legacy Media texture: source/copy agree; checker changes %d pixels, profile=%s' % (sampling['checker_changed_pixels'], sampling['numeric_profile']),
    ]
    if any(text.lower().count(line.lower()) != 1 for line in expected):
        raise ValueError('actual owner attachment, texture or numeric runtime log evidence differs')


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ['fixture','artifacts','log','texture-uasset','texture-uexp','report']:
        parser.add_argument('--'+name,required=True,type=Path)
    parser.add_argument('--profile',choices=['defaults','combined'],default='defaults')
    args=parser.parse_args()
    try:
        if GENERATED.resolve() not in args.report.resolve().parents:
            raise ValueError('report must be under porting/generated')
        raw=args.fixture.read_bytes()
        if sha(raw) != FIXTURE_SHA256:
            raise ValueError('fixture differs from independently audited original Media fixture')
        fixture=json.loads(raw)
        source,source_report=check_source(fixture,args.texture_uasset,args.texture_uexp)
        sampling=check_sampling(fixture,args.artifacts,args.profile,source)
        check_log(args.log.read_text(errors='replace'),fixture,sampling)
        projection_path=args.report.with_name(args.report.stem+'-shader-fixture.json')
        shader_report=args.report.with_name(args.report.stem+'-shaders.json')
        if any(p.exists() for p in (args.report,projection_path,shader_report)):
            raise ValueError('choose fresh verification outputs')
        projection=copy.deepcopy(fixture)
        del projection['texture_contract']
        projection_path.write_text(json.dumps(projection,indent=2,allow_nan=False)+'\n')
        command=[sys.executable,'-I',str(HERE/'verify_runtime_artifacts.py'),'--fixture',str(projection_path),
                 '--artifacts',str(args.artifacts),'--report',str(shader_report)]
        run=subprocess.run(command,capture_output=True,text=True,timeout=120)
        if run.returncode:
            raise ValueError('shader verification failed: '+run.stderr.strip())
        shaders=read_json(shader_report)
        if (len(shaders['records']) != 7 or shaders['layout_hash'] != sampling['layout_hash']
                or shaders['fixture_sha256'] != sha(projection_path.read_bytes())
                or shaders['verifier_sha256'] != sha((HERE/'verify_runtime_artifacts.py').read_bytes())
                or {r['shader_type'] for r in shaders['records']} != {s['record']['type']['name'] for s in fixture['shaders']}
                or any(r['instructions_preserved'] is not True or r['validator_exit'] != 0 for r in shaders['records'])):
            raise ValueError('expected seven intact original shader payloads')
        result=dict(passed=True,fixture_sha256=sha(raw),source=source_report,sampling=sampling,verified_shader_payloads=7,
                    runtime_log_sha256=sha(args.log.read_bytes()),shader_report=str(shader_report.resolve()),
                    shader_report_sha256=sha(shader_report.read_bytes()),shader_projection_sha256=sha(projection_path.read_bytes()),
                    scope='Original staged texture bytes, live indexed binding and sampling diagnostics; no original-client pixel or interactive UI parity claim.')
        args.report.write_text(json.dumps(result,indent=2)+'\n')
        print(json.dumps(result))
    except (OSError,ValueError,KeyError,TypeError,subprocess.SubprocessError) as error:
        parser.exit(1,'error: '+str(error)+'\n')


if __name__ == '__main__':
    main()
