#!/usr/bin/env python3
"""Bind complete original material bytes and records to the runtime adapter input.

The bundle is generated evidence, never a substitute for parsing the live asset.
The native reader must parse its bounded archive independently before conversion.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
GENERATED = HERE.parent / 'generated'
DEFAULT = GENERATED / 'material_pipeline'


def sibling(name):
    spec = importlib.util.spec_from_file_location(name, HERE / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


resource_reader = sibling('inspect_material_resource')
decoder = sibling('legacy_shader_map')
library_reader = sibling('shader_archive')
fixture_exporter = sibling('export_runtime_fixture')


def sha256(raw):
    return hashlib.sha256(raw).hexdigest()


def validate_source(raw, framing):
    if len(raw) > decoder.MAX_BYTES or sha256(raw) != framing['uexp_sha256']:
        raise ValueError('original uexp differs from resource receipt')
    start, end = framing['export_start'], framing['export_end']
    if not 0 <= start <= framing['wrapper_start'] < end <= len(raw):
        raise ValueError('invalid original export range')
    actual = resource_reader.parse_resource(raw, framing['wrapper_start'], end)
    for key, value in actual.items():
        if framing[key] != value:
            raise ValueError('original wrapper differs from receipt: ' + key)
    header = resource_reader.inspect_legacy_header(
        raw, actual['resource_start'], end, framing['export_name'])
    if header != framing['observed_inline_header']:
        raise ValueError('original inline header differs from receipt')
    return raw[actual['resource_start']:end]


def all_records(material):
    return decoder.all_records(material)


def verify_fixture(material, fixture):
    if fixture['schema'] != 'fortnite.legacy-slate.v1':
        raise ValueError('unsupported runtime fixture schema')
    if fixture['provenance']['resource_sha256'] != material['resource_sha256']:
        raise ValueError('runtime fixture belongs to a different original resource')
    if fixture['provenance']['shader_library_sha256'] != material['shader_library']['sha256']:
        raise ValueError('runtime fixture library differs from original library')
    if any(material[key] != value for key, value in fixture['material'].items()):
        raise ValueError('runtime fixture material identity differs from original')
    nodes, roots = fixture_exporter.flatten_expressions(material['uniform_expression_arrays'])
    if nodes != fixture['nodes'] or any(fixture[key] != value for key, value in roots.items()):
        raise ValueError('runtime fixture graph differs from original expressions')
    records = material['material_shaders']['records']
    if len(records) != 7 or len(fixture['shaders']) != len(records):
        raise ValueError('expected seven original Slate adapter records')
    if fixture['numeric_comparison_bytes'] != 312:
        raise ValueError('unexpected Locker numeric comparison range')
    for record, shader in zip(records, fixture['shaders']):
        if record != shader['record'] or record['debug_uniform_buffer_layout'] != fixture['expected_layout']:
            raise ValueError('runtime shader record differs from original')
        mapped, reflected = shader['mapped'], shader['reflection']
        index = record['library_association']['index']
        code_path = Path(reflected['output']).resolve()
        if GENERATED.resolve() not in code_path.parents:
            raise ValueError('mapped shader must stay under generated')
        code = code_path.read_bytes()
        if (not mapped['validated'] or mapped['shader_index'] != index
                or reflected['shader_index'] != index
                or mapped['frequency'] != record['frequency']
                or reflected['frequency'] != record['frequency']
                or mapped['output_sha256'] != sha256(code)
                or reflected['spirv_sha256'] != sha256(code)
                or shader['spirv_sha1'] != hashlib.sha1(code).hexdigest()):
            raise ValueError('mapped runtime code differs from verified original association')


def create_bundle(args):
    paths = [args.framing, args.decoded, args.fixture]
    data = [path.read_bytes() for path in paths]
    framing, decoded, fixture = [json.loads(raw) for raw in data]
    raw = args.uexp.read_bytes()
    payload = validate_source(raw, framing)
    parsed = decoder.parse_prefix(payload, framing['names'])
    archive_path = Path(decoded['shader_library']['path'])
    archive_hash = library_reader.sha256(archive_path)
    if archive_hash != decoded['shader_library']['sha256']:
        raise ValueError('original shader library differs from decoded receipt')
    code_receipts = []
    with library_reader.ShaderArchive(archive_path, layout='fortnite-12.60') as archive:
        decoder.associate_library(parsed, archive)
        for record in all_records(parsed):
            index = record['library_association']['index']
            code = archive.shader_code(index)
            code_receipts.append(dict(type=record['type'], output_hash=record['output_hash']['hash'],
                library_index=index, frequency=record['frequency'], bytes=len(code), sha256=sha256(code)))
    for key, value in parsed.items():
        if key == 'original_serializer_evidence':
            if any(decoded[key].get(k) != v for k, v in value.items()):
                raise ValueError('original serializer evidence differs from receipt')
        elif decoded.get(key) != value:
            raise ValueError('complete original parse differs from receipt: ' + key)
    if (not parsed['complete_serialized_shader_map'] or parsed['undecoded_bytes']
            or parsed['shader_record_count'] != 19 or len(code_receipts) != 19
            or framing['package'] != decoded['package']
            or decoded['resource_start'] != framing['resource_start']
            or decoded['resource_end'] != framing['resource_end']):
        raise ValueError('incomplete or inconsistent original material identity')
    verify_fixture(decoded, fixture)
    asset_path = args.uexp.with_suffix('.uasset')
    asset_bytes = asset_path.read_bytes()
    sources = [dict(path=str(path.resolve()), bytes=len(content), sha256=sha256(content))
               for path, content in zip(paths + [args.uexp, asset_path], data + [raw, asset_bytes])]
    return dict(schema='fortnite.legacy-material-ingress.v1', framing=framing, decoded=decoded,
        runtime_fixture=fixture, resource_raw_hex=payload.hex(), original_shader_code=code_receipts,
        provenance=dict(inputs=sources, resource_sha1=hashlib.sha1(payload).hexdigest(),
            fixture_sha1=hashlib.sha1(data[2]).hexdigest(), shader_library=dict(
                path=str(archive_path.resolve()), sha256=archive_hash, bytes=archive_path.stat().st_size),
            tools=[dict(path=str((HERE / name).resolve()), sha256=sha256((HERE / name).read_bytes()))
                   for name in ('export_ingress_bundle.py', 'legacy_shader_map.py',
                                'legacy_shader_records.py', 'inspect_material_resource.py', 'shader_archive.py')]),
        limitations=['Native loading must independently parse the live bounded original resource',
                    'All original records are retained; only seven Slate records have runtime adapter inputs',
                    'Actual-owner completeness, attachment and rendered original-client parity require runtime evidence'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--framing', type=Path, default=DEFAULT / 'locker-inline-resource.json')
    parser.add_argument('--decoded', type=Path, default=DEFAULT / 'locker-legacy-prefix.json')
    parser.add_argument('--fixture', type=Path, default=DEFAULT / 'locker-runtime-fixture.json')
    parser.add_argument('--uexp', type=Path, default=Path('/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/UI/Foundation/Materials/Athena_LockerTabMaster.uexp'))
    parser.add_argument('--output', type=Path, default=DEFAULT / 'locker-ingress-bundle.json')
    args = parser.parse_args()
    try:
        target = args.output.resolve()
        if GENERATED.resolve() not in target.parents:
            raise ValueError('ingress bundle must stay under porting/generated')
        if target in {path.resolve() for path in (args.framing, args.decoded, args.fixture, args.uexp, args.uexp.with_suffix('.uasset'))}:
            raise ValueError('ingress bundle cannot overwrite an input')
        bundle = create_bundle(args)
        content = (json.dumps(bundle, indent=2, allow_nan=False) + '\n').encode()
        target.parent.mkdir(parents=True, exist_ok=True)
        temporary = target.with_suffix('.tmp')
        temporary.write_bytes(content)
        temporary.replace(target)
        print(f'19 original records; {len(bytes.fromhex(bundle["resource_raw_hex"]))} resource bytes; '
              f'SHA1 {hashlib.sha1(content).hexdigest()}; {target}')
    except (OSError, ValueError, KeyError, TypeError, OverflowError) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
