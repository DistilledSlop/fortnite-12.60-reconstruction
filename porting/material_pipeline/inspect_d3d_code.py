#!/usr/bin/env python3
"""Inspect original D3D shader wrappers using UE's serialized resource table.

This is an evidence reader. It does not translate bindings to Vulkan or install
shader code in an engine shader map.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

GENERATED = Path(__file__).resolve().parents[1] / 'generated'
MAP_NAMES = ('shader_resource_view', 'sampler', 'unordered_access_view',
             'layout_hash', 'texture')


def u32(data, offset, end):
    if offset + 4 > end:
        raise ValueError('truncated shader wrapper integer')
    return struct.unpack_from('<I', data, offset)[0]


def path_in_generated(value):
    path = Path(value).resolve()
    if GENERATED.resolve() not in path.parents:
        raise ValueError('shader evidence path must be inside porting/generated')
    return path


def optional_names(data):
    """Decode UE's optional 'u' TArray<FString> only when its framing is exact."""
    end = len(data)
    count = u32(data, 0, end)
    if count > 256:
        raise ValueError('too many uniform-buffer names')
    cursor, names = 4, []
    for _ in range(count):
        length = struct.unpack_from('<i', data, cursor)[0] if cursor + 4 <= end else None
        if length is None or abs(length) > 4096:
            raise ValueError('invalid uniform-buffer name length')
        cursor += 4
        if length == 0:
            names.append('')
            continue
        needed = length if length > 0 else -length * 2
        if cursor + needed > end:
            raise ValueError('uniform-buffer name exceeds optional data')
        raw = data[cursor:cursor + needed]
        cursor += needed
        if length > 0 and raw.endswith(b'\0'):
            names.append(raw[:-1].decode('utf-8'))
        elif length < 0 and raw.endswith(b'\0\0'):
            names.append(raw[:-2].decode('utf-16-le'))
        else:
            raise ValueError('unterminated uniform-buffer name')
    if cursor != end:
        raise ValueError('trailing uniform-buffer name bytes')
    return names


def inspect_wrapper(data, containers):
    """Require FBaseShaderResourceTable + texture map + DXBC + UE optional data."""
    end = len(data)
    optional_size = u32(data, end - 4, end)
    if optional_size < 4 or optional_size > end:
        raise ValueError('invalid optional data size')
    code_end = end - optional_size
    bits = u32(data, 0, code_end)
    cursor, maps = 4, {}
    for name in MAP_NAMES:
        count = u32(data, cursor, code_end)
        cursor += 4
        if count > 65536 or count > (code_end - cursor) // 4:
            raise ValueError('resource table array exceeds code')
        maps[name] = list(struct.unpack_from('<' + 'I' * count, data, cursor)) if count else []
        cursor += count * 4
    hashes = maps['layout_hash']
    if bits >> len(hashes):
        raise ValueError('resource table bit exceeds layout-hash table')
    bindings = {}
    for name, values in maps.items():
        if name == 'layout_hash':
            continue
        if not values:
            bindings[name] = []
            continue
        if len(values) < len(hashes) + 1 or values[-1] != 0xffffffff:
            raise ValueError('resource map lacks header or end token')
        entries = []
        for packed in values[len(hashes):-1]:
            uniform_buffer, resource, bind = packed >> 24, (packed >> 8) & 0xffff, packed & 0xff
            if uniform_buffer >= len(hashes) or not bits & (1 << uniform_buffer):
                raise ValueError('resource map refers to inactive uniform buffer')
            entries.append({'uniform_buffer': uniform_buffer, 'resource': resource, 'bind': bind})
        bindings[name] = entries
    if len(containers) != 1:
        raise ValueError('expected exactly one reported DXBC container')
    container = containers[0]
    if cursor != container['offset'] or cursor + container['size'] != code_end:
        raise ValueError('resource table/DXBC/optional boundaries do not align')
    if data[cursor:cursor + 4] != b'DXBC':
        raise ValueError('missing DXBC after resource table')
    options, option_cursor = [], code_end
    while option_cursor < end - 4:
        if option_cursor + 5 > end - 4:
            raise ValueError('truncated optional data header')
        key = data[option_cursor]
        length = u32(data, option_cursor + 1, end - 4)
        option_cursor += 5
        if length > end - 4 - option_cursor:
            raise ValueError('optional data value exceeds wrapper')
        value = data[option_cursor:option_cursor + length]
        option_cursor += length
        row = {'key': chr(key) if 32 <= key <= 126 else f'0x{key:02x}',
               'size': length, 'sha256': hashlib.sha256(value).hexdigest()}
        if key == ord('p'):
            if length != 5:
                raise ValueError('invalid packed resource count size')
            row['packed_counts'] = dict(zip(('global_uniform_buffer_used', 'samplers', 'srvs',
                                             'constant_buffers', 'uavs'), value))
        elif key == ord('u'):
            row['uniform_buffer_names'] = optional_names(value)
        options.append(row)
    if option_cursor != end - 4:
        raise ValueError('optional data boundary mismatch')
    return {'resource_table_bits': bits, 'resource_maps': maps, 'resource_bindings': bindings,
            'resource_table_bytes': cursor, 'dxbc_bytes': container['size'],
            'optional_data_bytes': optional_size, 'optional_data': options}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-report', type=Path, required=True)
    parser.add_argument('--report', type=Path, default=GENERATED / 'material_pipeline/d3d-wrapper-inspection.json')
    args = parser.parse_args()
    try:
        destination = path_in_generated(args.report)
        source_bytes = args.source_report.read_bytes()
        source = json.loads(source_bytes)
        records = []
        for shader in source['shaders']:
            wrapper = path_in_generated(shader['wrapper_path'])
            data = wrapper.read_bytes()
            if hashlib.sha256(data).hexdigest() != shader['decoded_sha256']:
                raise ValueError(f'wrapper hash changed for shader {shader["index"]}')
            record = inspect_wrapper(data, shader['dxbc'])
            record.update(shader_index=shader['index'], frequency=shader['frequency'],
                          wrapper_sha256=shader['decoded_sha256'])
            records.append(record)
        result = {'source_report_sha256': hashlib.sha256(source_bytes).hexdigest(),
                  'archive_sha256': source['sha256'], 'inspected_shaders': len(records),
                  'records': records,
                  'limitations': ['D3D resource table entries are preserved, not rebound to Vulkan',
                                  'No shader has been run in a UE draw']}
        destination.parent.mkdir(parents=True, exist_ok=True)
        temporary = destination.with_suffix('.tmp')
        temporary.write_text(json.dumps(result, indent=2) + '\n')
        temporary.replace(destination)
        print(f'inspected {len(records)} original D3D shader wrappers: {destination}')
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
