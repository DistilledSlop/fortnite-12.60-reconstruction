#!/usr/bin/env python3
"""Bounded original M_TempIcon_Login_PS4 / XBox / Epic nonstatic-instance ingress.

These three UMaterialInstanceConstant owners have no serialized resource of their
own (inventory resource_status nonstatic-instance-inherits-parent); they inherit
the verified M_TempIcon_Login_Facebook shader map.  Each bundle therefore copies
that resource unchanged (resource owner = Facebook) and records only what the
original instance files prove: its complete scalar/texture override identities
and values, the resulting effective numeric cases, and the original override
texture's metadata.  No guessed defaults: anything unproven raises.

The inspector passed with --inspector must emit texture ``parameterInfo`` (the
checked-in cue4inspect/Program.cs does; the older frozen DLLs do not).
"""
import argparse
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

HERE = Path(__file__).resolve().parent
GENERATED = HERE.parent / 'generated'
ROOT = GENERATED / 'material_pipeline'
EXTRACTED = Path('/mnt/vibrato/fortnite-extracted-full')


def sibling(name):
    spec = importlib.util.spec_from_file_location(name, HERE / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


batch = sibling('export_frontend_numeric_batch')
frame = sibling('login_icon_frame')
evaluator = batch.evaluator

FACEBOOK = '/Game/UI/Login/Art/M_TempIcon_Login_Facebook.M_TempIcon_Login_Facebook'
FACEBOOK_BUNDLE_SHA1 = '8c00f790b3f785bb1b30fdd49c2f387dbfdf3da6'
TEXTURE_PARAMETER = dict(name=dict(name='Texture', number=0), association=2, index=-1)
# owner -> (direct parent, override texture package, texture report directory, frame name or None)
OWNERS = {
    '/Game/UI/Login/Art/M_TempIcon_Login_PS4.M_TempIcon_Login_PS4': (
        '/Game/UI/Login/Art/M_TempIcon_Login_Facebook.M_TempIcon_Login_Facebook',
        'FortniteGame/Content/UI/Login/Art/T_TempIcon_Login_Ps4', 'ps4-texture-1', 'T_TempIcon_Login_Ps4'),
    '/Game/UI/Login/Art/M_TempIcon_Login_XBox.M_TempIcon_Login_XBox': (
        '/Game/UI/Login/Art/M_TempIcon_Login_PS4.M_TempIcon_Login_PS4',
        'FortniteGame/Content/UI/Login/Art/T_TempIcon_Login_XBox', 'xbox-texture-1', 'T_TempIcon_Login_XBox'),
    '/Game/UI/Login/Art/M_TempIcon_Login_Epic.M_TempIcon_Login_Epic': (
        '/Game/UI/Login/Art/M_TempIcon_Login_PS4.M_TempIcon_Login_PS4',
        'FortniteGame/Content/UI/Foundation/Textures/Banner/Event_Special/T-Banners-Icons-Epic-L', 'epic-texture-1', None),
}
TEXTURE_FIELDS = ('width', 'height', 'pixelFormat', 'decodedPixelFormat', 'firstMip', 'srgb', 'filter', 'lodGroup',
                  'addressX', 'addressY', 'rgbaSha256', 'pngSha256')


def digest(raw):
    return hashlib.sha256(raw).hexdigest()


def receipt(path):
    raw = Path(path).read_bytes()
    return dict(path=str(Path(path).resolve()), bytes=len(raw), sha256=digest(raw))


def object_path(package):
    mount = '/Game/' + package.removeprefix('FortniteGame/Content/')
    return mount + '.' + mount.rsplit('/', 1)[1]


def fresh_instance_report(inspector, package):
    run = subprocess.run(['dotnet', str(inspector), package, str(EXTRACTED), '--loose', '--material-report'],
                         capture_output=True, text=True, timeout=180, check=True, cwd=HERE.parent.parent)
    return json.loads(run.stdout[run.stdout.index('{'):])


def fresh_texture_identity(inspector, package, owner_name):
    """Complete texture override identity from the original instance bytes."""
    report = fresh_instance_report(inspector, package)
    rows = [x for x in report['instanceParameters'] if x['name'] == owner_name]
    if len(rows) != 1 or len(rows[0]['textures']) != 1:
        raise ValueError('expected exactly one original texture override')
    row = rows[0]['textures'][0]
    if row.get('parameterInfo') != TEXTURE_PARAMETER or row['parameter'] != 'Texture':
        raise ValueError('texture override identity differs from the Facebook graph Texture parameter')
    return report, row


def build(owner_path, facebook, owners, inspector, texture_root):
    parent, texture_package, texture_dir, frame_name = OWNERS[owner_path]
    row = owners[owner_path]
    entry = row['entry']
    if (row['owner']['type'] != 'UMaterialInstanceConstant' or row['owner'].get('staticPermutation') is not False
            or entry['resource_status'] != 'nonstatic-instance-inherits-parent' or row['parent_path'] != parent):
        raise ValueError('owner is not an original nonstatic instance of the expected parent')
    if Path(entry['report']).with_suffix('.inline-resource.json').exists():
        raise ValueError('instance unexpectedly has its own serialized resource')
    chain = batch.owner_chain(owner_path, owners)
    if chain[0]['owner_path'] != FACEBOOK or any(x['owner']['type'] != 'UMaterialInstanceConstant' for x in chain[1:]):
        raise ValueError('unexpected inheritance chain')
    fixture = copy.deepcopy(facebook['runtime_fixture'])
    graph = {evaluator.identity(n['parameter']): 'scalar' if n['type'] == 'ScalarParameter' else 'vector'
             for n in fixture['nodes'] if n['type'] in ('ScalarParameter', 'VectorParameter')}
    effective, serialized, texture_chain = {}, [], []
    for item in chain[1:]:
        values, _ = batch.serialized_overrides(item)
        package = item['entry']['package']
        report, texture_row = fresh_texture_identity(inspector, package, item['owner']['name'])
        fresh = [x for x in report['instanceParameters'] if x['name'] == item['owner']['name']][0]
        fresh_values = [dict(type=k, parameter=v['parameterInfo'], value=v['value'])
                        for k, p in (('scalar', 'scalars'), ('vector', 'vectors')) for v in fresh[p]]
        if fresh_values != values:
            raise ValueError('fresh original override values differ from the inventory report')
        serialized.append(dict(owner_path=item['owner_path'], values=values, textures=[texture_row],
                               serialized_instance_parameters=fresh))
        texture_chain.append(texture_row)
        for value in values:
            key = evaluator.identity(value['parameter'])
            if key not in graph or graph[key] != value['type']:
                raise ValueError('override identity/type is not in the Facebook graph: ' + str(key))
            effective[key] = value
    used = list(effective.values())
    final = texture_chain[-1]
    if object_path(texture_package) != final['package'] + '.' + final['texture']:
        raise ValueError('override texture package differs from the expected original texture')
    meta = json.loads((texture_root / texture_dir / 'texture-report.json').read_bytes())
    if meta['package'] != texture_package:
        raise ValueError('texture report belongs to another package')
    texture = dict(parameter=TEXTURE_PARAMETER, texture_path=object_path(texture_package), texture_package=texture_package,
                   mip_count=len(meta['mips']), **{k: meta[k] for k in TEXTURE_FIELDS})
    source_paths = [texture_root / texture_dir / 'texture-report.json']
    if frame_name:
        base = EXTRACTED / texture_package
        texture['source_platform_data'] = frame.frame(*(base.with_suffix(s).read_bytes() for s in ('.uasset', '.uexp', '.ubulk')), meta, frame_name)
        source_paths += [base.with_suffix(s) for s in ('.uasset', '.uexp', '.ubulk')]
    else:
        texture['source_platform_data'] = None
        texture['source_platform_data_note'] = 'Only the BC1 512-square ten-mip layout is framed; this texture is not framed or draw-verified'
    fixture.update(
        uniform_cases=batch.numeric_cases(facebook['decoded'], fixture['nodes'], []),
        owner_uniform_cases=batch.numeric_cases(facebook['decoded'], fixture['nodes'], used),
        owner_parameter_expectations=used, unused_owner_parameter_expectations=[],
        serialized_override_chain=serialized, graph_effective_parameter_expectations=used,
        owner_texture_expectations=[texture])
    for key in ('draw_parameter',):
        fixture.pop(key, None)
    fixture['schema'] = facebook['runtime_fixture']['schema']
    source_paths += [Path(item['report_path']) for item in chain[1:]] + [Path(item['entry']['export_map']) for item in chain[1:]]
    source_paths += [EXTRACTED / (item['entry']['package'] + s) for item in chain[1:] for s in ('.uasset', '.uexp')]
    data = dict(schema=facebook['schema'], owner_path=owner_path, owner_type='UMaterialInstanceConstant', parent_path=parent,
                shader_identity=FACEBOOK, resource_owner_path=FACEBOOK, framing=facebook['framing'], decoded=facebook['decoded'],
                runtime_fixture=fixture, resource_raw_hex=facebook['resource_raw_hex'],
                original_shader_code=facebook['original_shader_code'],
                provenance=dict(inputs=[receipt(p) for p in dict.fromkeys(source_paths)],
                                resource_sha1=facebook['provenance']['resource_sha1'],
                                shader_library=facebook['provenance']['shader_library'],
                                parent_resource_bundle_sha1=FACEBOOK_BUNDLE_SHA1,
                                inspector_sha256=digest(inspector.read_bytes()),
                                exporter_sha256=digest(Path(__file__).read_bytes())),
                limitations=['Nonstatic instance: no own resource; shader map is the Facebook parent map',
                             'Original override identities/values and texture metadata only; no pixel/draw parity claim'])
    return data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--facebook-bundle', type=Path, default=ROOT / 'facebook-ingress-1/bundle.json')
    parser.add_argument('--inventory', type=Path, default=GENERATED / 'frontend-probes/frontend-material-inventory/summary.json')
    parser.add_argument('--inspector', type=Path, required=True)
    parser.add_argument('--texture-root', type=Path, default=ROOT)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    raw = args.facebook_bundle.read_bytes()
    if hashlib.sha1(raw).hexdigest() != FACEBOOK_BUNDLE_SHA1:
        raise ValueError('Facebook parent bundle changed')
    facebook = json.loads(raw)
    if GENERATED.resolve() not in args.output.resolve().parents or args.output.exists():
        raise ValueError('fresh generated output directory required')
    owners = batch.load_inventory(args.inventory)
    args.output.mkdir(parents=True)
    summary = []
    for owner_path in OWNERS:
        data = build(owner_path, facebook, owners, args.inspector, args.texture_root)
        out = (json.dumps(data, indent=2, allow_nan=False) + '\n').encode()
        target = args.output / (owner_path.rsplit('.', 1)[1] + '.bundle.json')
        target.write_bytes(out)
        summary.append(dict(owner_path=owner_path, parent_path=data['parent_path'], bundle=str(target),
                            bundle_sha1=hashlib.sha1(out).hexdigest(), bundle_sha256=digest(out),
                            resource_sha1=data['provenance']['resource_sha1']))
    print(json.dumps(summary, indent=1))


if __name__ == '__main__':
    main()
