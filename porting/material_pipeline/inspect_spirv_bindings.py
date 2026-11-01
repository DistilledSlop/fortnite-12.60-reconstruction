#!/usr/bin/env python3
"""Record translated SPIR-V descriptors beside exact original D3D bindings."""
import argparse
import hashlib
import json
from pathlib import Path
import struct

GENERATED = Path(__file__).resolve().parents[1] / 'generated'
SPIRV_MAGIC = 0x07230203


def digest(data):
    return hashlib.sha256(data).hexdigest()


def generated_path(value):
    path = Path(value).resolve()
    if GENERATED.resolve() not in path.parents:
        raise ValueError('shader evidence must stay in porting/generated')
    return path


def spirv_descriptors(data):
    if len(data) < 20 or len(data) > 64 * 1024 * 1024 or len(data) % 4:
        raise ValueError('invalid SPIR-V length')
    words = struct.unpack('<' + 'I' * (len(data) // 4), data)
    if words[0] != SPIRV_MAGIC or words[4] != 0 or not 0 < words[3] <= 1000000:
        raise ValueError('invalid SPIR-V header')
    bound = words[3]
    names, decorations, variables = {}, {}, {}
    cursor = 5
    while cursor < len(words):
        header = words[cursor]
        count, opcode = header >> 16, header & 0xffff
        if count < 1 or cursor + count > len(words):
            raise ValueError('invalid SPIR-V instruction length')
        operands = words[cursor + 1:cursor + count]
        if opcode == 5:  # OpName: target ID, null-terminated string.
            if len(operands) < 2 or not 0 < operands[0] < bound:
                raise ValueError('invalid SPIR-V name ID')
            raw = struct.pack('<' + 'I' * (len(operands) - 1), *operands[1:])
            if b'\0' not in raw:
                raise ValueError('unterminated SPIR-V name')
            names[operands[0]] = raw.split(b'\0', 1)[0].decode('utf-8')
        elif opcode == 71:  # OpDecorate
            if len(operands) >= 3 and operands[1] in (33, 34):
                if not 0 < operands[0] < bound:
                    raise ValueError('invalid SPIR-V decoration ID')
                field = 'binding' if operands[1] == 33 else 'set'
                previous = decorations.setdefault(operands[0], {})
                if field in previous and previous[field] != operands[2]:
                    raise ValueError('conflicting SPIR-V descriptor decoration')
                previous[field] = operands[2]
        elif opcode == 59:  # OpVariable: result type, result ID, storage class.
            if len(operands) >= 3:
                if not 0 < operands[1] < bound:
                    raise ValueError('invalid SPIR-V variable ID')
                variables[operands[1]] = operands[2]
        cursor += count
    output = []
    for identifier, decoration in decorations.items():
        if 'set' not in decoration or 'binding' not in decoration or identifier not in variables:
            raise ValueError('incomplete SPIR-V descriptor')
        output.append({'name': names.get(identifier), 'set': decoration['set'],
                       'binding': decoration['binding'], 'storage_class': variables[identifier]})
    output.sort(key=lambda row: (row['set'], row['binding'], row['name'] or ''))
    if len({(row['set'], row['binding']) for row in output}) != len(output):
        raise ValueError('SPIR-V descriptor binding collision')
    return output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--d3d-report', type=Path, required=True)
    parser.add_argument('--translation-report', type=Path, required=True)
    parser.add_argument('--report', type=Path,
                        default=GENERATED / 'material_pipeline/binding-correspondence.json')
    args = parser.parse_args()
    try:
        destination = generated_path(args.report)
        d3d_bytes, translation_bytes = args.d3d_report.read_bytes(), args.translation_report.read_bytes()
        d3d, translated = json.loads(d3d_bytes), json.loads(translation_bytes)
        originals = {row['shader_index']: row for row in d3d['records']}
        if (len(originals) != d3d['inspected_shaders']
                or d3d['archive_sha256'] != translated['archive_sha256']
                or d3d['source_report_sha256'] != translated['source_report_sha256']):
            raise ValueError('source report identities do not align')
        records = []
        for shader in translated['records']:
            if not shader['validated']:
                raise ValueError('translation report includes unvalidated shader')
            original = originals[shader['shader_index']]
            output = generated_path(shader['output'])
            code = output.read_bytes()
            if digest(code) != shader['output_sha256']:
                raise ValueError('translated SPIR-V hash changed')
            records.append({'shader_index': shader['shader_index'], 'frequency': shader['frequency'],
                            'd3d_resource_table_bits': original['resource_table_bits'],
                            'd3d_layout_hashes': original['resource_maps']['layout_hash'],
                            'd3d_bindings': original['resource_bindings'],
                            'd3d_optional_data': original['optional_data'],
                            'spirv_sha256': shader['output_sha256'],
                            'spirv_descriptors': spirv_descriptors(code),
                            'compiler_diagnostics_present': shader['compiler_diagnostics_present']})
        result = {'d3d_report_sha256': digest(d3d_bytes),
                  'translation_report_sha256': digest(translation_bytes),
                  'archive_sha256': d3d['archive_sha256'], 'shader_count': len(records),
                  'records': records,
                  'limitations': ['Descriptor records are observations, not a proven D3D-to-Vulkan binding map',
                                  'Compiler diagnostics and UE shader-map integration remain unresolved']}
        destination.parent.mkdir(parents=True, exist_ok=True)
        temporary = destination.with_suffix('.tmp')
        temporary.write_text(json.dumps(result, indent=2) + '\n')
        temporary.replace(destination)
        print(f'inspected {len(records)} translated shaders: {destination}')
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
