#!/usr/bin/env python3
"""Validate Apollo landscape ingress and translate its original archive shaders.

This preserves each serialized occurrence and reports explicit compiler bindings;
it does not establish UE runtime binding, draws, or visual equivalence.
"""
import argparse
import json
from pathlib import Path
import subprocess
import sys

import export_landscape_ingress as landscape
import inspect_d3d_code as d3d

HERE = Path(__file__).resolve().parent
GENERATED = landscape.GENERATED


def run(*args):
    result = subprocess.run([sys.executable, *map(str, args)], capture_output=True, text=True, timeout=300)
    if result.returncode:
        raise ValueError(f'{args[0]} failed: {result.stderr[-4000:]}')
    print(result.stdout.strip())


def generate(ingress_path, output, tool_prefix, include):
    output = output.resolve()
    if GENERATED.resolve() not in output.parents:
        raise ValueError('output must be under porting/generated')
    if ingress_path.resolve().parent == output:
        raise ValueError('ingress must not be inside the output directory')
    raw = ingress_path.read_bytes()
    ingress = json.loads(raw)
    if ingress['schema'] != 'fortnite.legacy-landscape-ingress.v1':
        raise ValueError('unexpected ingress schema')
    framing = ingress['framing']
    payload = bytes.fromhex(ingress['resource_raw_hex'])
    if landscape.sha256(payload) != framing['resource_sha256']:
        raise ValueError('resource identity differs')
    maps = landscape.decode_maps(payload, framing)
    landscape.validate_apollo(framing, maps)
    archive_path = Path(ingress['provenance']['shader_library']['path'])
    archive_hash = landscape.library_reader.sha256(archive_path)
    if archive_hash != landscape.ARCHIVE_HASH or archive_hash != ingress['provenance']['shader_library']['sha256']:
        raise ValueError('archive identity differs')
    with landscape.library_reader.ShaderArchive(archive_path, layout='fortnite-12.60') as archive:
        code, occurrences = landscape.associate_code(maps, archive)
    if ingress['decoded'] != dict(schema='fortnite.legacy-landscape-maps.v1', package=framing['package'],
                                  resource_sha256=landscape.sha256(payload), maps=maps):
        raise ValueError('decoded maps differ from original resource')
    if code != ingress['original_shader_code'] or occurrences != ingress['record_code_associations']:
        raise ValueError('archive code or occurrence associations differ')
    if len(code) != 43 or len(occurrences) != 136 or len({x['library_index'] for x in code}) != 43:
        raise ValueError('unexpected original shader population')
    output.mkdir(parents=True, exist_ok=True)
    (output / 'landscape-translation-receipt.json').unlink(missing_ok=True)
    shaders, inspected = [], []
    for original in code:
        data = bytes.fromhex(original['code_raw_hex'])
        if len(data) != original['bytes'] or landscape.sha256(data) != original['sha256']:
            raise ValueError('shader wrapper identity differs')
        containers = landscape.library_reader.dxbc_containers(data)
        parsed = d3d.inspect_wrapper(data, containers)
        container, = containers
        wrapper_path = output / f"{original['library_index']}.ue-shader-code"
        dxbc_path = output / f"{original['library_index']}.dxbc"
        wrapper_path.write_bytes(data)
        dxbc_path.write_bytes(data[container['offset']:container['offset'] + container['size']])
        container['path'] = str(dxbc_path)
        shaders.append(dict(index=original['library_index'], frequency=original['frequency'],
                            library_output_hash=original['output_hash'], decoded_sha256=original['sha256'],
                            wrapper_path=str(wrapper_path), dxbc=containers))
        inspected.append(dict(parsed, shader_index=original['library_index'], frequency=original['frequency'],
                              wrapper_sha256=original['sha256']))
    source = output / 'source-report.json'
    landscape.write_json(source, dict(sha256=archive_hash, shaders=shaders))
    source_hash = landscape.sha256(source.read_bytes())
    d3d_report = output / 'd3d-report.json'
    landscape.write_json(d3d_report, dict(source_report_sha256=source_hash, archive_sha256=archive_hash,
                                          records=inspected))
    mapped_dir = output / 'mapped'
    run(HERE / 'translate_mapped_dxbc.py', '--source-report', source, '--d3d-report', d3d_report,
        '--tool-prefix', tool_prefix, '--include', include, '--output', mapped_dir, '--limit', 43)
    mapped_path = mapped_dir / 'mapped-translation-report.json'
    reflection_path = output / 'reflection.json'
    run(HERE / 'reflect_mapped_spirv.py', '--mapped-report', mapped_path, '--report', reflection_path)
    mapped = json.loads(mapped_path.read_bytes())
    reflection = json.loads(reflection_path.read_bytes())
    if mapped['truncated'] or mapped['validated_shaders'] != 43 or len(reflection['records']) != 43:
        raise ValueError('incomplete translation or reflection')
    by_index = {x['shader_index']: x for x in mapped['records']}
    reflected = {x['shader_index']: x for x in reflection['records']}
    if set(by_index) != {x['library_index'] for x in code} or set(reflected) != set(by_index):
        raise ValueError('translated shader identities differ')
    rows = []
    for occurrence in occurrences:
        item = dict(occurrence)
        shader = by_index[item['library_index']]
        item.update(spirv_path=shader['output'], spirv_sha256=shader['output_sha256'])
        rows.append(item)
    receipt = dict(schema='fortnite.landscape-spirv-translation.v1', ingress_path=str(ingress_path.resolve()),
                   ingress_sha256=landscape.sha256(raw), archive_sha256=archive_hash,
                   source_report=str(source), source_report_sha256=source_hash,
                   d3d_report=str(d3d_report), d3d_report_sha256=landscape.sha256(d3d_report.read_bytes()),
                   mapped_report=str(mapped_path), mapped_report_sha256=landscape.sha256(mapped_path.read_bytes()),
                   reflection_report=str(reflection_path), reflection_report_sha256=landscape.sha256(reflection_path.read_bytes()),
                   unique_shaders=len(code), occurrences=rows,
                   limitations=['Bindings describe declared compiler mappings, not verified UE runtime binding',
                                'Translation and reflection do not establish draws or visual parity'])
    path = output / 'landscape-translation-receipt.json'
    landscape.write_json(path, receipt)
    print(f'{len(code)} shaders; {len(rows)} occurrences; {path}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--ingress', type=Path, default=GENERATED / 'material_pipeline/apollo-d4-landscape-ingress.json')
    parser.add_argument('--output', type=Path, default=GENERATED / 'material_pipeline/apollo-d4-landscape-spirv')
    parser.add_argument('--tool-prefix', type=Path, default=GENERATED / 'material_pipeline/tools/vkd3d2-compiled')
    parser.add_argument('--include', type=Path, default=GENERATED / 'material_pipeline/tools/vkd3d-2.0/include')
    args = parser.parse_args()
    try:
        generate(args.ingress, args.output, args.tool_prefix, args.include)
    except (OSError, ValueError, KeyError, TypeError, OverflowError, subprocess.TimeoutExpired) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
