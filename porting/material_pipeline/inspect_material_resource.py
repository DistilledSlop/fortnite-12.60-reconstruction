#!/usr/bin/env python3
"""Locate and frame an original material's cooked inline resource in .uexp.

Requires the legacy package export map emitted by Cue4Inspect --export-map-json.
The local UE writer's name/loc/bytes schema is checked against exact export
boundaries. This does not deserialize a shader map or change any package.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

GENERATED = Path(__file__).resolve().parents[1] / 'generated'
MAX_FILE = 64 * 1024 * 1024


def i32(data, offset, end):
    if offset < 0 or offset + 4 > end:
        raise ValueError('truncated material field')
    return struct.unpack_from('<i', data, offset)[0]


def name_entry(data, cursor, end):
    length = i32(data, cursor, end)
    cursor += 4
    if not length or abs(length) > 4096:
        raise ValueError('invalid material name length')
    count = length if length > 0 else -length * 2
    if cursor + count + 4 > end:
        raise ValueError('material name exceeds export')
    raw = data[cursor:cursor + count]
    cursor += count
    if length > 0:
        if raw[-1] != 0:
            raise ValueError('unterminated material name')
        name = raw[:-1].decode('utf-8')
    else:
        if raw[-2:] != b'\0\0':
            raise ValueError('unterminated wide material name')
        name = raw[:-2].decode('utf-16-le')
    if not name or any(ord(char) < 32 for char in name):
        raise ValueError('invalid material name character')
    # UE's FNameEntrySerialized writes two 16-bit case hashes here.
    cursor += 4
    return name, cursor


def parse_resource(data, start, export_end):
    names_count = i32(data, start, export_end)
    if not 1 <= names_count <= 4096:
        raise ValueError('unreasonable material name count')
    names, cursor = [], start + 4
    for _ in range(names_count):
        name, cursor = name_entry(data, cursor, export_end)
        names.append(name)
    locations_count = i32(data, cursor, export_end)
    cursor += 4
    if not 1 <= locations_count <= 32 or cursor + 6 * locations_count + 4 > export_end:
        raise ValueError('invalid material location count')
    locations = []
    for _ in range(locations_count):
        offset, feature, quality = struct.unpack_from('<IBB', data, cursor)
        cursor += 6
        locations.append({'offset': offset, 'feature_level': feature, 'quality_level': quality})
    resource_size = i32(data, cursor, export_end)
    cursor += 4
    if resource_size < 1 or cursor + resource_size != export_end:
        raise ValueError('resource does not end at export boundary')
    if locations[0]['offset'] != 0 or any(location['offset'] >= resource_size
                                          or location['feature_level'] > 6
                                          or location['quality_level'] > 5 for location in locations):
        raise ValueError('invalid material resource location')
    offsets = [location['offset'] for location in locations]
    if offsets != sorted(offsets) or len(set(offsets)) != len(offsets):
        raise ValueError('material resource locations are not ordered')
    payload = data[cursor:export_end]
    return {'wrapper_start': start, 'resource_start': cursor, 'resource_end': export_end,
            'resource_bytes': resource_size, 'resource_sha256': hashlib.sha256(payload).hexdigest(),
            'name_count': names_count, 'names': names, 'locations': locations,
            'resource_first_64_hex': payload[:64].hex()}


def scan_resource(data, export_start, export_end):
    if not 0 <= export_start < export_end <= len(data):
        raise ValueError('export range exceeds .uexp')
    matches = []
    for cursor in range(export_start, export_end - 12):
        if data[cursor + 3] != 0:
            continue
        try:
            matches.append(parse_resource(data, cursor, export_end))
        except (ValueError, UnicodeError, struct.error):
            continue
    if len(matches) != 1:
        raise ValueError(f'expected one exact material resource wrapper, found {len(matches)}')
    return matches[0]


def read_fstring(data, cursor, end):
    length = i32(data, cursor, end)
    cursor += 4
    if not 1 <= length <= 4096 or cursor + length > end:
        raise ValueError('invalid inline material string length')
    raw = data[cursor:cursor + length]
    if raw[-1] != 0:
        raise ValueError('unterminated inline material string')
    return raw[:-1].decode('utf-8'), cursor + length


def inspect_legacy_header(data, resource_start, resource_end, export_name):
    """Inspect observed 12.60 header words without naming unverified fields."""
    if resource_start + 48 > resource_end:
        raise ValueError('inline material header too short')
    cooked, valid, quality, feature = struct.unpack_from('<IIII', data, resource_start)
    if cooked != 1 or valid != 1 or quality > 5 or feature > 6:
        raise ValueError('unexpected inline material leading fields')
    shader_map_id_hash = data[resource_start + 16:resource_start + 36].hex()
    after_hash = i32(data, resource_start + 36, resource_end)
    first, cursor = read_fstring(data, resource_start + 40, resource_end)
    second, cursor = read_fstring(data, cursor, resource_end)
    if not second.startswith('/Game/') or not second.endswith('.' + first):
        raise ValueError('inline material identity strings are inconsistent')
    return {'cooked': cooked, 'valid': valid, 'quality_level': quality,
            'feature_level': feature, 'shader_map_id_hash': shader_map_id_hash,
            'word_after_hash': after_hash, 'identity_name': first, 'identity_path': second,
            'identity_matches_export': first == export_name,
            'next_field_offset': cursor, 'next_u32': i32(data, cursor, resource_end),
            'local_layout_interpretation': {
                'max_field_alignment': after_hash,
                'flags': i32(data, resource_start + 40, resource_end),
                'initialized_flag_set': bool(i32(data, resource_start + 40, resource_end) & 1)}}


def generated_path(value):
    path = Path(value).resolve()
    if GENERATED.resolve() not in path.parents:
        raise ValueError('report must stay in porting/generated')
    return path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--uexp', type=Path, required=True)
    parser.add_argument('--export-map', type=Path, required=True)
    parser.add_argument('--export-index', type=int, default=1)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    try:
        report_path = generated_path(args.report)
        raw = args.uexp.read_bytes()
        if len(raw) > MAX_FILE:
            raise ValueError('uexp exceeds bounded material probe size')
        map_bytes = args.export_map.read_bytes()
        map_json = next(line for line in map_bytes.splitlines() if line.startswith(b'{'))
        export_map = json.loads(map_json)
        export = next(row for row in export_map['exports'] if row['index'] == args.export_index)
        first = export['serialOffset'] - export_map['headerBytes']
        end = first + export['serialSize']
        result = scan_resource(raw, first, end)
        result['observed_inline_header'] = inspect_legacy_header(
            raw, result['resource_start'], result['resource_end'], export['name'])
        result.update(package=export_map['package'], export_index=args.export_index,
                      export_name=export['name'], export_start=first, export_end=end,
                      uexp_sha256=hashlib.sha256(raw).hexdigest(),
                      export_map_sha256=hashlib.sha256(map_bytes).hexdigest(),
                      limitations=['Wrapper location and framing verified; inner shader-map schema not decoded',
                                   'No engine asset loading or draw verified'])
        report_path.parent.mkdir(parents=True, exist_ok=True)
        temporary = report_path.with_suffix('.tmp')
        temporary.write_text(json.dumps(result, indent=2) + '\n')
        temporary.replace(report_path)
        print(f'original material resource {result["resource_start"]}..{end} '
              f'in export {first}..{end}; {result["name_count"]} names; {report_path}')
    except (OSError, ValueError, KeyError, TypeError, StopIteration) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
