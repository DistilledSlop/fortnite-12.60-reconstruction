#!/usr/bin/env python3
"""Export original Paper2D shader occurrences and verified conversion evidence.

This is an offline manifest, not a runtime shader-map or texture-binding adapter.
"""
import argparse
import hashlib
import json
from pathlib import Path

import paper2d_shader_prefix as prefix
import inspect_material_resource as framing_parser
import inspect_d3d_code as d3d_parser
import reflect_mapped_spirv as reflector
import inspect_spirv_bindings as inventory
import translate_mapped_dxbc as translator
import shader_archive

HERE = Path(__file__).resolve().parents[1]
GENERATED = HERE / 'generated'
EVIDENCE = GENERATED / 'paper2d-shader-semantics'
ASSETS = Path('/mnt/vibrato/fortnite-extracted-full/Engine/Plugins/2D/Paper2D/Content')
ARCHIVE = Path('/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/ShaderArchive-FortniteGame-PCD3D_SM5.ushaderbytecode')
OWNERS = [('DefaultSpriteMaterial', 'parent', 'parent-pipelines-prefix.json'),
          ('TranslucentUnlitSpriteMaterial', 'mi', 'mi-complete-prefix.json')]


def digest(data):
    return hashlib.sha256(data).hexdigest()


def load(path):
    return json.loads(path.read_bytes())


def unique(rows, key):
    result = {}
    for row in rows:
        value = row[key]
        if value in result:
            raise ValueError(f'duplicate {key}: {value}')
        result[value] = row
    return result


def generated_file(path):
    path = Path(path).resolve()
    if GENERATED.resolve() not in path.parents:
        raise ValueError('conversion artifact outside generated directory')
    return path


def build(evidence=EVIDENCE, assets=ASSETS, archive_path=ARCHIVE):
    evidence, assets, archive_path = Path(evidence), Path(assets), Path(archive_path)
    conv = evidence / 'conversion'
    paths = {key: conv / name for key, name in [('source', 'source.json'), ('d3d', 'd3d.json'),
             ('mapped', 'mapped/mapped-translation-report.json'), ('reflection', 'reflection.json')]}
    reports = {key: load(path) for key, path in paths.items()}
    source, d3d, mapped, reflection = (reports[k] for k in ('source', 'd3d', 'mapped', 'reflection'))
    hashes = {key: digest(path.read_bytes()) for key, path in paths.items()}
    archive_hash = shader_archive.sha256(archive_path)
    if (source['sha256'] != archive_hash or d3d['archive_sha256'] != archive_hash or
        mapped['archive_sha256'] != archive_hash or d3d['source_report_sha256'] != hashes['source'] or
        mapped['source_report_sha256'] != hashes['source'] or mapped['d3d_report_sha256'] != hashes['d3d'] or
        reflection['mapped_report_sha256'] != hashes['mapped']):
        raise ValueError('conversion report provenance mismatch')
    if (source['selection_truncated'] or mapped['truncated'] or
        source['selected_shader_count'] != source['decoded_shader_count'] or
        mapped['selected_shaders'] != mapped['validated_shaders']):
        raise ValueError('incomplete conversion')
    tables = [unique(source['shaders'], 'index')] + [unique(report['records'], 'shader_index')
              for report in (d3d, mapped, reflection)]
    if any(set(table) != set(tables[0]) for table in tables[1:]) or any(
        n != len(tables[0]) for n in (source['selected_shader_count'], d3d['inspected_shaders'], mapped['selected_shaders'])):
        raise ValueError('conversion output sets or counts differ')
    owners, used = [], set()
    with shader_archive.ShaderArchive(archive_path, layout='fortnite-12.60') as archive:
        for name, tag, report_name in OWNERS:
            raw = (assets / (name + '.uexp')).read_bytes()
            framing = load(evidence / (tag + '-resource.json'))
            package = '/Engine/Plugins/2D/Paper2D/Content/' + name
            if framing['package'] != package or digest(raw) != framing['uexp_sha256']:
                raise ValueError('asset package or uexp identity changed')
            parsed_framing = framing_parser.parse_resource(raw, framing['wrapper_start'], framing['resource_end'])
            if any(framing[k] != v for k, v in parsed_framing.items()):
                raise ValueError('asset framing or names changed')
            decoded = prefix.parse_prefix(raw[framing['resource_start']:framing['resource_end']], parsed_framing['names'])
            if not decoded['complete_serialized_shader_map'] or decoded['undecoded_bytes']:
                raise ValueError('incomplete serialized shader map')
            prefix.legacy.associate_library(decoded, archive)
            for mesh in decoded['mesh_shader_maps']:
                for pipeline in mesh['pipelines']:
                    prefix.legacy.associate_library({'material_shaders': {'records': pipeline['stages']}}, archive)
            expected = load(evidence / report_name)
            count = len(decoded['material_shaders']['records']) + sum(len(m['records']) + sum(len(p['stages']) for p in m['pipelines']) for m in decoded['mesh_shader_maps'])
            decoded['shader_record_count'] = count
            if {k: v for k, v in expected.items() if k != 'input_sha256'} != decoded:
                raise ValueError('decoded shader map differs from report')
            if decoded['material_path'] != '/Paper2D/DefaultSpriteMaterial.DefaultSpriteMaterial':
                raise ValueError('unexpected shader-map material identity')
            occurrences, pipelines = [], []
            def add(record, location):
                index = record['library_association']['index']
                used.add(index)
                occurrence = {'id': f'{tag}:{record["offset"]}', 'location': location,
                              'archive_index': index, 'archive_output_hash': record['output_hash']['hash'],
                              'frequency': record['frequency'], 'decoded_record': record}
                occurrences.append(occurrence)
                return occurrence['id']
            for record in decoded['material_shaders']['records']:
                add(record, {'kind': 'material'})
            for mi, mesh in enumerate(decoded['mesh_shader_maps']):
                for record in mesh['records']:
                    add(record, {'kind': 'mesh', 'mesh_index': mi})
                for pi, pipeline in enumerate(mesh['pipelines']):
                    stage_ids = [add(stage, {'kind': 'pipeline_stage', 'mesh_index': mi, 'pipeline_index': pi, 'stage_index': si}) for si, stage in enumerate(pipeline['stages'])]
                    if len(stage_ids) != pipeline['stage_count']:
                        raise ValueError('missing pipeline stage')
                    pipelines.append({'mesh_index': mi, 'pipeline_index': pi, 'type': pipeline['type'], 'offset': pipeline['offset'], 'stage_record_ids': stage_ids})
            if len({r['id'] for r in occurrences}) != count or len(occurrences) != count:
                raise ValueError('duplicate or missing shader occurrence')
            owners.append({'owner_path': f'/Paper2D/{name}.{name}', 'package': package,
                           'uexp_sha256': framing['uexp_sha256'], 'framing': parsed_framing,
                           'shader_map_identity': decoded['material_path'], 'decoded_resource': decoded,
                           'shader_records': occurrences, 'pipelines': pipelines})
        if used != set(tables[0]):
            raise ValueError('conversion shaders do not exactly match original occurrences')
        outputs = []
        for index in sorted(used):
            src, drow, mapped_row, reflected = (t[index] for t in tables)
            freq = archive.shaders[index][3]
            code = archive.shader_code(index)
            containers = shader_archive.dxbc_containers(code)
            if (src['library_output_hash'] != archive.shader_hashes[index].hex() or
                src['frequency'] != freq or any(r['frequency'] != freq for r in (drow, mapped_row, reflected)) or
                src['decoded_bytes'] != len(code) or src['decoded_sha256'] != digest(code) or
                generated_file(src['wrapper_path']).read_bytes() != code or len(containers) != 1 or len(src['dxbc']) != 1):
                raise ValueError('original archive wrapper identity mismatch')
            container = src['dxbc'][0]
            if {k: v for k, v in container.items() if k != 'path'} != containers[0]:
                raise ValueError('DXBC framing mismatch')
            dxbc = code[container['offset']:container['offset'] + container['size']]
            if (generated_file(container['path']).read_bytes() != dxbc or
                generated_file(mapped_row['input']).read_bytes() != dxbc or mapped_row['input_sha256'] != digest(dxbc) or
                mapped_row['wrapper_sha256'] != digest(code) or drow['wrapper_sha256'] != digest(code)):
                raise ValueError('conversion input identity mismatch')
            inspected = d3d_parser.inspect_wrapper(code, containers)
            if any(drow[k] != v for k, v in inspected.items()):
                raise ValueError('D3D resource table changed')
            spv_path = generated_file(mapped_row['output'])
            spv = spv_path.read_bytes()
            if (not mapped_row['validated'] or mapped_row['unmatched_resource_table_entries'] or
                mapped_row['output_sha256'] != digest(spv) or mapped_row['output_bytes'] != len(spv) or
                reflected['spirv_sha256'] != digest(spv) or Path(reflected['output']).resolve() != spv_path):
                raise ValueError('mapped shader identity or validation changed')
            bindings, unmatched = translator.correlate(mapped_row['bindings'],
                inventory.spirv_descriptors(spv, include_offsets=True), inspected)
            if bindings != mapped_row['bindings'] or unmatched:
                raise ValueError('mapped bindings differ from original resource tables')
            actual = reflector.Module(spv).reflect()
            assignments = {(b['set'], b['binding']): b for b in mapped_row['bindings']}
            if len(assignments) != len(mapped_row['bindings']) or set(assignments) != {(d['set'], d['binding']) for d in actual['descriptors']}:
                raise ValueError('mapped descriptor assignments differ')
            compatible = {'cbv': {6}, 'sampler': {0}, 'srv': {2, 4}, 'uav': {3, 5, 7}}
            for descriptor in actual['descriptors']:
                binding = assignments[descriptor['set'], descriptor['binding']]
                if descriptor['vulkan_descriptor_type'] not in compatible[binding['kind']]:
                    raise ValueError('descriptor type mismatch')
                descriptor['d3d_register'] = dict(kind=binding['kind'], space=binding['register_space'], index=binding['register_index'])
                descriptor['resource_table_entries'] = binding['resource_table_entries']
            if any(reflected[k] != v for k, v in actual.items()):
                raise ValueError('SPIR-V reflection changed')
            outputs.append({'archive_index': index, 'archive_output_hash': src['library_output_hash'], 'frequency': freq,
                            'source': src, 'd3d': drow, 'mapped': mapped_row, 'reflection': reflected})
    return {'schema': 'paper2d-original-shader-manifest-v1', 'archive_sha256': archive_hash,
            'conversion_report_sha256': hashes, 'owners': owners, 'shaders': outputs,
            'limitations': ['Offline evidence only; runtime shader-map loading and texture binding are not implemented.',
                            'Original shader conversion and reflection do not establish rendered parity.']}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--evidence', type=Path, default=EVIDENCE)
    parser.add_argument('--assets', type=Path, default=ASSETS)
    parser.add_argument('--archive', type=Path, default=ARCHIVE)
    parser.add_argument('--output', type=Path, default=EVIDENCE / 'shader-manifest.json')
    args = parser.parse_args()
    try:
        output = generated_file(args.output)
        result = build(args.evidence, args.assets, args.archive)
        output.parent.mkdir(parents=True, exist_ok=True)
        temporary = output.with_suffix('.tmp')
        temporary.write_text(json.dumps(result, indent=2, allow_nan=False) + '\n')
        temporary.replace(output)
        print(f'{sum(len(o["shader_records"]) for o in result["owners"])} occurrences, {len(result["shaders"])} shaders: {output}')
    except (OSError, ValueError, KeyError, TypeError, IndexError) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
