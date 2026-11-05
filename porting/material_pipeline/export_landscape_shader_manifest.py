#!/usr/bin/env python3
"""Compose the pinned landscape translation evidence; this does not prove runtime bindings."""
import argparse
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
GENERATED = ROOT / 'generated'
RECEIPT = GENERATED / 'material_pipeline/apollo-d4-landscape-spirv/landscape-translation-receipt.json'
RECEIPT_SHA256 = 'fe61856ae9d72f891634e40cb2d20e4e5ba67534b7bdc86a3e71b86ca3c6acea'
OUTPUT = RECEIPT.parent / 'shader-manifest.json'
VF_EVIDENCE = {
    'binding_names': ('apollo-material-evidence/landscape-vf-virtuals/0x143add1e0.c', '90606395f5a44523b4a63cd88357cfc74a9a57a4fe55fadbd393e46c65ce1434'),
    'serialization_order': ('apollo-material-evidence/landscape-vf-virtuals/0x143af1010.c', 'af234cde9ed9982bf80179700b10c1ec5d054c58d3c43666df803806393b69f1'),
    'element_bindings': ('apollo-material-evidence/landscape-vf-element/0x143ae8e90.c', '14d1720664b1c697fe19512d82827654c88362a475615e48081da3716b35e5b6'),
}
VF_FIELDS = (('lod_values', 'LodValues', 0x0e), ('forced_lod', 'ForcedLod', 0x14),
             ('lod_tessellation', 'LodTessellationParams', 0x08),
             ('neighbor_section_lod', 'NeighborSectionLod', 0x1a),
             ('lod_bias', 'LodBias', 0x20), ('section_lods', 'SectionLods', 0x26))


def digest(raw):
    return hashlib.sha256(raw).hexdigest()


def read_pinned(path, expected):
    path = Path(path).resolve()
    if GENERATED.resolve() not in path.parents:
        raise ValueError('evidence path outside generated directory')
    raw = path.read_bytes()
    if digest(raw) != expected:
        raise ValueError(f'evidence digest mismatch: {path}')
    return raw


def indexed(rows, key):
    result = {}
    for row in rows:
        index = row[key]
        if index in result:
            raise ValueError(f'duplicate shader index: {index}')
        result[index] = row
    return result


def build(receipt_path=RECEIPT, receipt_sha256=RECEIPT_SHA256):
    receipt = json.loads(read_pinned(receipt_path, receipt_sha256))
    if receipt['schema'] != 'fortnite.landscape-spirv-translation.v1':
        raise ValueError('unexpected receipt schema')
    ingress = json.loads(read_pinned(receipt['ingress_path'], receipt['ingress_sha256']))
    evidence = {}
    for label, (path, sha256) in VF_EVIDENCE.items():
        read_pinned(GENERATED / path, sha256)
        evidence[label] = {'path': str(GENERATED / path), 'sha256': sha256}
    reports = {name: json.loads(read_pinned(receipt[name + '_report'], receipt[name + '_report_sha256']))
               for name in ('source', 'd3d', 'mapped', 'reflection')}
    source, d3d, mapped, reflection = (reports[k] for k in ('source', 'd3d', 'mapped', 'reflection'))
    if (source['sha256'] != receipt['archive_sha256'] or d3d['archive_sha256'] != receipt['archive_sha256']
            or mapped['archive_sha256'] != receipt['archive_sha256']
            or d3d['source_report_sha256'] != receipt['source_report_sha256']
            or mapped['source_report_sha256'] != receipt['source_report_sha256']
            or mapped['d3d_report_sha256'] != receipt['d3d_report_sha256']
            or reflection['mapped_report_sha256'] != receipt['mapped_report_sha256']
            or mapped['truncated'] or mapped['selected_shaders'] != mapped['validated_shaders']):
        raise ValueError('conversion provenance or completeness mismatch')
    tables = [indexed(source['shaders'], 'index')] + [indexed(r['records'], 'shader_index')
              for r in (d3d, mapped, reflection)]
    codes = indexed(ingress['original_shader_code'], 'library_index')
    indices = set(tables[0])
    if any(set(t) != indices for t in tables[1:] + [codes]) or len(indices) != receipt['unique_shaders'] or len(indices) != mapped['selected_shaders']:
        raise ValueError('shader index sets differ')
    original = ingress['record_code_associations']
    occurrences = receipt['occurrences']
    if len(original) != len(occurrences) or len({o['location']['record_offset'] for o in original}) != len(original):
        raise ValueError('missing or duplicate occurrence')
    shaders = []
    for index in sorted(indices):
        src, drow, mapped_row, reflected = (t[index] for t in tables)
        code = codes[index]
        raw = bytes.fromhex(code['code_raw_hex'])
        if (code['bytes'] != len(raw) or digest(raw) != code['sha256'] or code['sha256'] != src['decoded_sha256']
                or code['output_hash'] != src['library_output_hash'] or code['frequency'] != src['frequency']
                or any(r['frequency'] != src['frequency'] for r in (drow, mapped_row, reflected))
                or drow['wrapper_sha256'] != code['sha256'] or mapped_row['wrapper_sha256'] != code['sha256']):
            raise ValueError('shader input identity mismatch')
        spv = read_pinned(reflected['output'], reflected['spirv_sha256'])
        if (not mapped_row['validated'] or mapped_row['unmatched_resource_table_entries']
                or mapped_row['output'] != reflected['output'] or mapped_row['output_sha256'] != digest(spv)
                or mapped_row['output_bytes'] != len(spv)):
            raise ValueError('shader output identity mismatch')
        shaders.append({'archive_index': index, 'archive_output_hash': src['library_output_hash'],
                        'frequency': src['frequency'], 'source': src, 'd3d': drow,
                        'mapped': mapped_row, 'reflection': reflected})
    used = set()
    enriched = []
    for expected, occurrence in zip(original, occurrences):
        if any(occurrence.get(k) != v for k, v in expected.items()):
            raise ValueError('occurrence identity mismatch')
        index = occurrence['library_index']
        if index not in indices:
            raise ValueError('occurrence references missing shader')
        code, ref = codes[index], tables[3][index]
        if (occurrence['output_hash'] != code['output_hash'] or occurrence['frequency'] != code['frequency']
                or occurrence['sha256'] != code['sha256'] or occurrence['bytes'] != code['bytes']
                or occurrence['spirv_path'] != ref['output'] or occurrence['spirv_sha256'] != ref['spirv_sha256']):
            raise ValueError('occurrence shader identity mismatch')
        loc = occurrence['location']
        mesh = ingress['decoded']['maps'][loc['map_index']]['decoded']['mesh_shader_maps'][loc['mesh_index']]
        rows = (mesh['records'] if loc['pipeline_index'] is None
                else mesh['pipelines'][loc['pipeline_index']]['stages'])
        matches = [row for row in rows if row['offset'] == loc['record_offset']]
        if len(matches) != 1:
            raise ValueError('occurrence decoded record mismatch')
        record = matches[0]
        if (record['type'] != occurrence['type'] or record['pipeline'] != occurrence['pipeline']
                or record['vertex_factory'] != occurrence['vertex_factory'] or record['frequency'] != occurrence['frequency']
                or record['output_hash']['hash'] != occurrence['output_hash']):
            raise ValueError('decoded record identity mismatch')
        if loc['pipeline_index'] is not None and rows[loc['stage_index']]['offset'] != loc['record_offset']:
            raise ValueError('pipeline stage order mismatch')
        fields = record.get('vertex_factory_parameters', {}).get('landscape_parameters')
        if (fields is not None) != (occurrence['frequency'] == 0):
            raise ValueError('landscape VF parameter presence mismatch')
        named = []
        if fields is not None:
            if set(fields) != {f[0] for f in VF_FIELDS}:
                raise ValueError('landscape VF field set mismatch')
            buffers = record['resource']['loose_parameter_buffers']
            for field, name, object_offset in VF_FIELDS:
                allocation = fields[field]
                size, offset, slot = (allocation[k] for k in ('byte_count', 'base_index', 'buffer_index'))
                if size:
                    matching = [b for b in buffers if b['buffer_index_and_size'][0] == slot
                                and [offset, size] in b['parameters']]
                    descriptors = [d for d in ref['descriptors'] if d['d3d_register'] ==
                                   {'kind': 'cbv', 'space': 0, 'index': slot}]
                    if len(matching) != 1 or len(descriptors) != 1 or offset + size > descriptors[0]['block_span_bytes']:
                        raise ValueError('landscape VF loose allocation mismatch')
                named.append({'parameter_name': name, 'object_offset': object_offset,
                              'serialized_field': field, 'allocation': allocation})
        enriched.append({**occurrence, 'landscape_vf_loose_contracts': named})
        used.add(index)
    if used != indices:
        raise ValueError('unreferenced shader')
    return {'schema': 'landscape-original-shader-manifest-v1', 'archive_sha256': receipt['archive_sha256'],
            'translation_receipt_sha256': receipt_sha256,
            'conversion_report_sha256': {k: receipt[k + '_report_sha256'] for k in reports},
            'landscape_vf_contract_evidence': evidence, 'occurrences': enriched, 'shaders': shaders,
            'limitations': ['Offline evidence composition only; runtime shader-map loading and bindings are not established.',
                            'Original uniform member layouts, pass loose names and direct-resource bindings are not established by this manifest.',
                            'Landscape VF names and allocations are source evidence, not a current-engine runtime binding contract.',
                            'VT feedback UAV descriptors are preserved as reflected; their runtime binding is unverified.']}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=OUTPUT)
    args = parser.parse_args()
    try:
        output = args.output.resolve()
        if GENERATED.resolve() not in output.parents:
            raise ValueError('output outside generated directory')
        manifest = build()
        output.parent.mkdir(parents=True, exist_ok=True)
        temp = output.with_suffix('.tmp')
        temp.write_text(json.dumps(manifest, indent=2, allow_nan=False) + '\n')
        temp.replace(output)
        print(f'{len(manifest["occurrences"])} occurrences, {len(manifest["shaders"])} shaders: {output}')
    except (OSError, ValueError, KeyError, TypeError, IndexError) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
