#!/usr/bin/env python3
"""Compile hash-checked original DXBC with explicit register-to-SPIR-V bindings.

This uses vkd3d-shader's public scan/interface API, not SPIR-V debug-name guesses.
No UE shader header, runtime binding, shader-map installation or draw is implied.
"""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess

HERE = Path(__file__).resolve().parent
GENERATED = HERE.parent / 'generated'
spec = importlib.util.spec_from_file_location('spirv_inventory', HERE / 'inspect_spirv_bindings.py')
inventory = importlib.util.module_from_spec(spec)
spec.loader.exec_module(inventory)
KINDS = {0: 'srv', 1: 'uav', 2: 'cbv', 3: 'sampler'}
TABLE_KIND = {'shader_resource_view': 0, 'texture': 0, 'unordered_access_view': 1, 'sampler': 3}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def generated_path(value):
    path = Path(value).resolve()
    if GENERATED.resolve() not in path.parents:
        raise ValueError('shader artifacts must stay in porting/generated')
    return path


def correlate(bindings, descriptors, original):
    """Join explicit API assignments to exact SPIR-V locations and D3D tables."""
    assigned, registers = {}, set()
    for binding in bindings:
        key = binding['set'], binding['binding']
        register = binding['type'], binding['register_space'], binding['register_index']
        if (binding['type'] not in KINDS or binding['count'] != 1
                or binding['register_space'] != 0 or key in assigned or register in registers):
            raise ValueError('unsupported or duplicate explicit descriptor binding')
        assigned[key] = dict(binding, kind=KINDS[binding['type']], resource_table_entries=[])
        registers.add(register)
    observed = {(d['set'], d['binding']): d for d in descriptors}
    if len(observed) != len(descriptors) or assigned.keys() != observed.keys():
        raise ValueError('compiled descriptor locations differ from explicit assignments')
    for key, row in assigned.items():
        descriptor = observed[key]
        expected_storage = 2 if row['type'] == 2 else 0
        if descriptor['storage_class'] != expected_storage:
            raise ValueError('compiled descriptor storage differs from D3D register kind')
        row['spirv_descriptor'] = descriptor
    by_register = {(row['type'], row['register_index']): row for row in assigned.values()}
    unmatched = []
    for table, kind in TABLE_KIND.items():
        for entry in original['resource_bindings'][table]:
            row = by_register.get((kind, entry['bind']))
            item = dict(entry, table=table)
            if row is None:
                unmatched.append(item)
            else:
                row['resource_table_entries'].append(item)
    return list(assigned.values()), unmatched


def checked_run(command, environment, timeout=30):
    result = subprocess.run(command, env=environment, capture_output=True, text=True, timeout=timeout)
    if result.returncode:
        raise ValueError(f'command exited {result.returncode}: {command[0]}: {result.stderr[-4000:]}')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-report', type=Path, required=True)
    parser.add_argument('--d3d-report', type=Path, required=True)
    parser.add_argument('--tool-prefix', type=Path,
                        default=GENERATED / 'material_pipeline/tools/vkd3d2-compiled')
    parser.add_argument('--include', type=Path,
                        default=GENERATED / 'material_pipeline/tools/vkd3d-2.0/include')
    parser.add_argument('--output', type=Path, default=GENERATED / 'material_pipeline/frontend-mapped')
    parser.add_argument('--binding-base', type=int, default=0)
    parser.add_argument('--limit', type=int, default=64)
    args = parser.parse_args()
    if not 0 <= args.binding_base <= 4096 or not 1 <= args.limit <= 256:
        parser.error('binding base must be 0..4096 and limit 1..256')
    try:
        output = generated_path(args.output)
        library = args.tool_prefix.resolve() / 'lib/x86_64-linux-gnu/libvkd3d-shader.so.1'
        validator = args.tool_prefix.resolve() / 'bin/spirv-val'
        cc = Path(shutil.which('cc') or '/missing-cc').resolve()
        sources = [HERE / 'map_dxbc_bindings.c', args.include / 'vkd3d_shader.h',
                   args.include / 'vkd3d_types.h', library, validator, cc]
        identities = {str(p.resolve()): digest(p.read_bytes()) for p in sources}
        environment = dict(os.environ, LD_LIBRARY_PATH=str(library.parent))
        source_bytes, d3d_bytes = args.source_report.read_bytes(), args.d3d_report.read_bytes()
        source, d3d = json.loads(source_bytes), json.loads(d3d_bytes)
        if (d3d['source_report_sha256'] != digest(source_bytes)
                or d3d['archive_sha256'] != source['sha256']):
            raise ValueError('original report identities differ')
        originals = {r['shader_index']: r for r in d3d['records']}
        shaders = source['shaders']
        if len(originals) != len(d3d['records']) or len({s['index'] for s in shaders}) != len(shaders):
            raise ValueError('duplicate original shader index')
        output.mkdir(parents=True, exist_ok=True)
        helper = output / 'map-dxbc-bindings'
        build_command = [str(cc), '-std=c11', '-Wall', '-Wextra', '-Werror', '-O2',
                         '-I', str(args.include.resolve()), str(sources[0]), str(library),
                         '-o', str(helper.with_suffix('.partial'))]
        checked_run(build_command, environment)
        helper.with_suffix('.partial').replace(helper)
        identities[str(helper)] = digest(helper.read_bytes())
        records = []
        for shader in shaders[:args.limit]:
            wrapper = generated_path(shader['wrapper_path']).read_bytes()
            if digest(wrapper) != shader['decoded_sha256'] or len(shader['dxbc']) != 1:
                raise ValueError('original wrapper identity/container count differs')
            container = shader['dxbc'][0]
            path = generated_path(container['path'])
            data = path.read_bytes()
            offset, size = container['offset'], container['size']
            if offset < 0 or size < 32 or offset + size > len(wrapper) or data != wrapper[offset:offset + size]:
                raise ValueError('DXBC differs from exact original wrapper slice')
            original = originals[shader['index']]
            if original['wrapper_sha256'] != shader['decoded_sha256']:
                raise ValueError('D3D report wrapper identity differs')
            dest = output / f"{shader['index']}-{digest(data)[:20]}.spv"
            temporary = dest.with_suffix('.partial.spv')
            command = [str(helper), str(path), str(temporary), str(args.binding_base)]
            compiled = checked_run(command, environment)
            mapping = json.loads(compiled.stdout)
            code = temporary.read_bytes()
            bindings, unmatched = correlate(mapping['bindings'], inventory.spirv_descriptors(code, include_offsets=True), original)
            validated = checked_run([str(validator), '--target-env', 'vulkan1.0', str(temporary)], environment)
            if path.read_bytes() != data:
                raise ValueError('DXBC changed during mapped compilation')
            temporary.replace(dest)
            records.append({'shader_index': shader['index'], 'frequency': shader['frequency'],
                            'input': str(path), 'input_sha256': digest(data),
                            'wrapper_sha256': shader['decoded_sha256'], 'library_version': mapping['library_version'],
                            'command': command, 'compiler_diagnostics': compiled.stderr,
                            'validator_diagnostics': validated.stdout + validated.stderr,
                            'validated': True, 'bindings': bindings,
                            'unmatched_resource_table_entries': unmatched,
                            'output': str(dest), 'output_sha256': digest(code), 'output_bytes': len(code)})
        report = {'source_report_sha256': digest(source_bytes), 'd3d_report_sha256': digest(d3d_bytes),
                  'archive_sha256': source['sha256'], 'build_command': build_command,
                  'tool_sha256': identities, 'target_env': 'vulkan1.0', 'binding_base': args.binding_base,
                  'selected_shaders': len(shaders), 'validated_shaders': len(records),
                  'truncated': len(shaders) > args.limit, 'records': records,
                  'limitations': ['API assignments establish D3D register-to-SPIR-V descriptor locations only',
                                  'UE shader headers, material values, runtime resource binding and draws remain unverified']}
        receipt = output / 'mapped-translation-report.json'
        receipt.with_suffix('.tmp').write_text(json.dumps(report, indent=2, allow_nan=False) + '\n')
        receipt.with_suffix('.tmp').replace(receipt)
        count = sum(len(r['bindings']) for r in records)
        missing = sum(len(r['unmatched_resource_table_entries']) for r in records)
        print(f'validated {len(records)} shaders; {count} explicit descriptors; {missing} unmatched table entries; {receipt}')
    except (OSError, ValueError, KeyError, TypeError, subprocess.TimeoutExpired) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
