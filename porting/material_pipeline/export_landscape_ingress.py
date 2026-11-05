#!/usr/bin/env python3
"""Prepare both original Apollo D4 shader maps and archive code for terrain ingress.

This lossless transport IR deliberately does not evaluate uniforms or fabricate
Linux bindings. Original record offsets remain relative to the whole resource,
including in the second map. Output belongs under ignored porting/generated.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('landscape_base_ingress', HERE / 'export_ingress_bundle.py')
base = importlib.util.module_from_spec(spec)
spec.loader.exec_module(base)
sibling, validate_source, sha256, GENERATED = base.sibling, base.validate_source, base.sha256, base.GENERATED

decoder = sibling('legacy_shader_map')
library_reader = sibling('shader_archive')
EXTRACTED = Path('/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content')
PACKAGE = '/Game/Athena/Apollo/Maps/Landscape/Apollo_Terrain_LS_D4'
RESOURCE_HASH = '73bd93d83493b0cd3ca7e99fbabe2907a0a7ac579679e664d6257512f9b300fa'
ARCHIVE_HASH = '2e795bd21c5e9bd69a6fdb520ef73f6fd70822c011c4232ce1ba08ccf10aef21'


def decode_maps(payload, framing):
    """Reparse every bounded map, without rebasing declared record offsets."""
    locations = framing['locations']
    starts = [location['offset'] for location in locations]
    if not starts or starts[0] != 0 or starts != sorted(set(starts)):
        raise ValueError('invalid shader-map locations')
    ends = starts[1:] + [len(payload)]
    maps = []
    for location, end in zip(locations, ends):
        start = location['offset']
        decoded = decoder.parse_prefix(payload, framing['names'], start=start, end=end)
        if (decoded['quality_level'] != location['quality_level']
                or decoded['feature_level'] != location['feature_level']
                or not decoded['complete_serialized_shader_map']
                or decoded['undecoded_bytes']
                or decoded['decoded_prefix_bytes'] != end - start):
            raise ValueError('incomplete or inconsistent shader-map location')
        maps.append(dict(offset=start, end=end, decoded=decoded))
    return maps


def record_locations(maps):
    """Keep each serialized occurrence, even when archive code is shared."""
    for map_index, entry in enumerate(maps):
        material = entry['decoded']
        for record in material['material_shaders']['records']:
            yield dict(map_index=map_index, mesh_index=None, pipeline_index=None,
                       stage_index=None, record_offset=record['offset']), record
        for mesh_index, mesh in enumerate(material['mesh_shader_maps']):
            for record in mesh['records']:
                yield dict(map_index=map_index, mesh_index=mesh_index, pipeline_index=None,
                           stage_index=None, record_offset=record['offset']), record
            for pipeline_index, pipeline in enumerate(mesh.get('pipelines', [])):
                for stage_index, record in enumerate(pipeline['stages']):
                    yield dict(map_index=map_index, mesh_index=mesh_index,
                               pipeline_index=pipeline_index, stage_index=stage_index,
                               record_offset=record['offset']), record


def associate_code(maps, archive):
    """Validate hash/target/size and retain exact expanded archive code bytes."""
    for entry in maps:
        decoder.associate_library(entry['decoded'], archive)
    code_by_index, occurrences = {}, []
    for location, record in record_locations(maps):
        index = record['library_association']['index']
        output_hash = record['output_hash']['hash']
        if index not in code_by_index:
            code = archive.shader_code(index)
            if len(code) != record['resource']['uncompressed_code_bytes']:
                raise ValueError('archive expanded code size mismatch')
            code_by_index[index] = dict(library_index=index, output_hash=output_hash,
                frequency=record['frequency'], bytes=len(code), sha256=sha256(code),
                code_raw_hex=code.hex())
        code = code_by_index[index]
        if code['output_hash'] != output_hash or code['frequency'] != record['frequency']:
            raise ValueError('archive code identity mismatch')
        occurrences.append(dict(location=location, type=record['type'],
            pipeline=record['pipeline'], vertex_factory=record['vertex_factory'],
            output_hash=output_hash, library_index=index, frequency=record['frequency'],
            bytes=code['bytes'], sha256=code['sha256']))
    return list(code_by_index.values()), occurrences


def validate_apollo(framing, maps):
    if (framing['package'] != PACKAGE or framing['resource_sha256'] != RESOURCE_HASH
            or [(m['offset'], m['end']) for m in maps] != [(0, 50634), (50634, 101268)]
            or [m['decoded']['quality_level'] for m in maps] != [0, 1]
            or any(m['decoded']['feature_level'] != 3 or
                   m['decoded']['shader_record_count'] != 68 for m in maps)):
        raise ValueError('expected complete original Apollo D4 resource and both quality maps')


def source_receipt(path):
    return dict(path=str(path.resolve()), bytes=path.stat().st_size,
                sha256=library_reader.sha256(path))


def create_bundle(args):
    framing = json.loads(args.framing.read_text())
    payload = validate_source(args.uexp.read_bytes(), framing)
    maps = decode_maps(payload, framing)
    validate_apollo(framing, maps)
    archive_receipt = source_receipt(args.shader_archive)
    if archive_receipt['sha256'] != ARCHIVE_HASH:
        raise ValueError('expected original Fortnite PCD3D_SM5 shader archive')
    with library_reader.ShaderArchive(args.shader_archive, layout='fortnite-12.60') as archive:
        code, occurrences = associate_code(maps, archive)
    decoded_receipt = dict(schema='fortnite.legacy-landscape-maps.v1',
        package=framing['package'], resource_sha256=sha256(payload), maps=maps)
    if args.decoded is not None and json.loads(args.decoded.read_text()) != decoded_receipt:
        raise ValueError('decoded shader maps differ from independently parsed originals')
    inputs = [args.framing, args.uexp, args.package_header]
    if args.decoded is not None:
        inputs.append(args.decoded)
    return dict(schema='fortnite.legacy-landscape-ingress.v1', framing=framing,
        decoded=decoded_receipt, resource_raw_hex=payload.hex(),
        original_shader_code=code, record_code_associations=occurrences,
        provenance=dict(inputs=[source_receipt(path) for path in inputs],
            resource_sha1=hashlib.sha1(payload).hexdigest(), shader_library=archive_receipt,
            tools=[source_receipt(HERE / name) for name in (
                'export_landscape_ingress.py', 'export_ingress_bundle.py',
                'legacy_shader_map.py', 'legacy_shader_records.py',
                'inspect_material_resource.py', 'shader_archive.py')]),
        runtime_loading_verified=False,
        limitations=[
            'Native ingress must independently parse the live bounded resource and validate both maps',
            'Uniform expressions, VT stacks, compilation metadata and shader parameters are serialized facts; evaluation and live texture/RVT owners remain runtime work',
            'Original PCD3D_SM5 code requires verified Linux translation and reflection before shader creation',
            'Landscape vertex-factory, material, pass, view and VT/RVT binding contracts require runtime implementation',
            'Archive association and byte preservation do not establish shader semantics, draws or original-client visual parity'])


def checked_outputs(outputs, inputs):
    resolved = [path.resolve() for path in outputs]
    if len(set(resolved)) != len(resolved):
        raise ValueError('output paths must be distinct')
    for path in resolved:
        if GENERATED.resolve() not in path.parents:
            raise ValueError('landscape artifacts must stay under porting/generated')
        if path in {source.resolve() for source in inputs if source is not None}:
            raise ValueError('landscape artifacts cannot overwrite an input')


def write_json(path, value):
    content = (json.dumps(value, indent=2, allow_nan=False) + '\n').encode()
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + '.tmp')
    temporary.write_bytes(content)
    temporary.replace(path)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--framing', type=Path, default=GENERATED / 'apollo-material-evidence/d4-resource.json')
    parser.add_argument('--uexp', type=Path, default=EXTRACTED / (PACKAGE.removeprefix('/Game/') + '.uexp'))
    parser.add_argument('--package-header', type=Path, default=EXTRACTED / (PACKAGE.removeprefix('/Game/') + '.umap'))
    parser.add_argument('--shader-archive', type=Path, default=EXTRACTED / 'ShaderArchive-FortniteGame-PCD3D_SM5.ushaderbytecode')
    parser.add_argument('--decoded', type=Path, help='optional existing decoded-maps receipt to revalidate')
    parser.add_argument('--decoded-output', type=Path, default=GENERATED / 'material_pipeline/apollo-d4-landscape-maps.json')
    parser.add_argument('--output', type=Path, default=GENERATED / 'material_pipeline/apollo-d4-landscape-ingress.json')
    args = parser.parse_args()
    try:
        checked_outputs([args.output, args.decoded_output], [args.framing, args.uexp,
            args.package_header, args.shader_archive, args.decoded])
        bundle = create_bundle(args)
        write_json(args.decoded_output, bundle['decoded'])
        write_json(args.output, bundle)
        print(f'{len(bundle["decoded"]["maps"])} maps; '
              f'{len(bundle["record_code_associations"])} original records; '
              f'{len(bundle["original_shader_code"])} unique archive shaders; {args.output}')
    except (OSError, ValueError, KeyError, TypeError, OverflowError) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
