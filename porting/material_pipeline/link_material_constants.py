#!/usr/bin/env python3
"""Link decoded original material bindings to evaluated bytes and mapped SPIR-V.

The output is bridge evidence, not a UE material, shader header or runtime draw.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path

GENERATED = Path(__file__).resolve().parents[1] / 'generated'
_spec = importlib.util.spec_from_file_location('legacy_shader_records',
                                                Path(__file__).with_name('legacy_shader_records.py'))
_records = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_records)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def material_binding(record, reflected, numeric_bytes):
    """Use the original serialized Material UB slot, never a CBV debug-name guess."""
    binding = record['material_uniform_buffer']
    if not binding['bound']:
        return None
    matches = [d for d in reflected['descriptors']
               if d['d3d_register'] == dict(kind='cbv', space=0, index=binding['base_index'])]
    if len(matches) != 1 or matches[0]['vulkan_descriptor_type'] != 6:
        raise ValueError('bound original Material slot lacks one uniform-buffer descriptor')
    descriptor, = matches
    if descriptor['block_span_bytes'] > numeric_bytes:
        raise ValueError('declared Material uniform span exceeds evaluated numeric bytes')
    return dict(d3d_constant_buffer=binding['base_index'], descriptor_set=descriptor['set'],
                descriptor_binding=descriptor['binding'], set_word_offset=descriptor['set_word_offset'],
                binding_word_offset=descriptor['binding_word_offset'],
                declared_block_span_bytes=descriptor['block_span_bytes'], evaluated_numeric_bytes=numeric_bytes)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mapped-report', type=Path, required=True)
    parser.add_argument('--reflection-report', type=Path, required=True)
    parser.add_argument('--material', nargs=3, action='append', required=True,
                        metavar=('DECODED_JSON', 'UNIFORM_JSON', 'UNIFORM_BIN'))
    parser.add_argument('--report', type=Path, default=GENERATED / 'material_pipeline/material-constant-bindings.json')
    args = parser.parse_args()
    try:
        destination = args.report.resolve()
        if GENERATED.resolve() not in destination.parents:
            raise ValueError('output report must stay in porting/generated')
        mapped_bytes, reflected_bytes = args.mapped_report.read_bytes(), args.reflection_report.read_bytes()
        mapped, reflection = json.loads(mapped_bytes), json.loads(reflected_bytes)
        if reflection['mapped_report_sha256'] != digest(mapped_bytes):
            raise ValueError('reflection references a different mapped receipt')
        mapped_shaders = {r['shader_index']: r for r in mapped['records']}
        reflected_shaders = {r['shader_index']: r for r in reflection['records']}
        if len(mapped_shaders) != len(mapped['records']) or len(reflected_shaders) != len(reflection['records']):
            raise ValueError('duplicate shader record')
        materials = []
        for paths in args.material:
            decoded_path, uniforms_path, numeric_path = [Path(x).resolve() for x in paths]
            if any(GENERATED.resolve() not in x.parents for x in (decoded_path, uniforms_path, numeric_path)):
                raise ValueError('material evidence must stay in porting/generated')
            decoded_raw, uniforms_raw, numeric = decoded_path.read_bytes(), uniforms_path.read_bytes(), numeric_path.read_bytes()
            decoded, uniforms = json.loads(decoded_raw), json.loads(uniforms_raw)
            if (uniforms['input_report_sha256'] != digest(decoded_raw)
                    or uniforms['numeric_bytes_sha256'] != digest(numeric)
                    or uniforms['resource_sha256'] != decoded['resource_sha256']
                    or uniforms['cooked_shader_map_id_hash'] != decoded['cooked_shader_map_id_hash']
                    or uniforms['numeric_bytes'] != len(numeric)):
                raise ValueError('evaluated constants do not match the decoded material')
            if decoded['shader_library']['sha256'] != mapped['archive_sha256']:
                raise ValueError('material records reference a different shader library')
            rows = []
            shader_records = _records.all_records(decoded)
            for record in shader_records:
                index = record['library_association']['index']
                mapped_shader, reflected_shader = mapped_shaders[index], reflected_shaders[index]
                code = Path(mapped_shader['output']).resolve()
                if GENERATED.resolve() not in code.parents:
                    raise ValueError('mapped shader path must stay in generated')
                code_hash = digest(code.read_bytes())
                if (code_hash != mapped_shader['output_sha256'] or code_hash != reflected_shader['spirv_sha256']
                        or record['frequency'] != mapped_shader['frequency']
                        or record['frequency'] != reflected_shader['frequency']):
                    raise ValueError('shader bytes/stage disagree across receipts')
                linked = material_binding(record, reflected_shader, len(numeric))
                offsets = record['debug_uniform_buffer_layout']['resource_offsets']
                if offsets and min(offsets) != len(numeric):
                    raise ValueError('serialized original material layout disagrees with numeric span')
                remaining = [d for d in reflected_shader['descriptors'] if linked is None or
                             (d['set'], d['binding']) != (linked['descriptor_set'], linked['descriptor_binding'])]
                row = dict(shader_type=record['type']['name'], shader_index=index,
                    vertex_factory=record['vertex_factory']['name'],
                    output_hash=record['output_hash']['hash'], frequency=record['frequency'],
                    spirv_sha256=code_hash, material_uniform_binding=linked,
                    remaining_descriptors=remaining)
                if record['pipeline']['name'] != 'None':
                    row.update(pipeline=record['pipeline']['name'], record_offset=record['offset'])
                rows.append(row)
            materials.append(dict(material_path=decoded['material_path'], resource_sha256=decoded['resource_sha256'],
                decoded_report_sha256=digest(decoded_raw), uniform_report_sha256=digest(uniforms_raw),
                numeric_path=str(numeric_path), numeric_sha256=digest(numeric), numeric_bytes=len(numeric),
                shader_records=rows))
        result = dict(mapped_report_sha256=digest(mapped_bytes), reflection_report_sha256=digest(reflected_bytes),
                      linker_sha256=digest(Path(__file__).read_bytes()), materials=materials,
                      limitations=['Original Material slot and declared numeric spans are linked; runtime upload is untested',
                                   'Default expression values omit instance/render-proxy lookup and target floating-point parity',
                                   'Other constants, texture/sampler resources, UE headers and actual draws remain unfinished'])
        destination.parent.mkdir(parents=True, exist_ok=True)
        temp = destination.with_suffix('.tmp')
        temp.write_text(json.dumps(result, indent=2, allow_nan=False) + '\n')
        temp.replace(destination)
        records = [r for m in materials for r in m['shader_records']]
        count = sum(r['material_uniform_binding'] is not None for r in records)
        print(f'linked {len(materials)} materials, {len(records)} original records, {count} Material uniform bindings: {destination}')
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
