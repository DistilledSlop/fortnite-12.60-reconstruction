#!/usr/bin/env python3
"""Strict, partial decoder of the pre-memory-image Fortnite 12.60 shader map.

This decodes a PREFIX, never an executable shader map. Unknown expression types
fail at their tag; the undecoded compilation-output tail is returned explicitly.
The recursive grammar matches Epic's pre-freeze serializers, available at:
https://github.com/folgerwang/UnrealEngine/blob/release/Engine/Source/Runtime/Engine/Private/Materials/MaterialUniformExpressions.h
https://github.com/folgerwang/UnrealEngine/blob/release/Engine/Source/Runtime/Engine/Private/Materials/MaterialUniformExpressions.cpp
The two identity strings and subsequent array boundaries are independently
observed in the four original resource payloads. No LayoutParams are present.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

MAX_BYTES = 64 * 1024 * 1024
ARRAYS = ('vector', 'scalar', 'texture_2d', 'texture_cube', 'texture_volume',
          'texture_virtual', 'texture_external', 'texture_2d_array')
PREFIX = 'FMaterialUniformExpression'


class Reader:
    def __init__(self, data, names):
        self.data, self.names, self.cursor, self.nodes = data, names, 0, 0

    def read(self, fmt):
        size = struct.calcsize('<' + fmt)
        if self.cursor + size > len(self.data):
            raise ValueError(f'truncated legacy shader-map field at {self.cursor}')
        value = struct.unpack_from('<' + fmt, self.data, self.cursor)
        self.cursor += size
        return value[0] if len(value) == 1 else list(value)

    def count(self):
        count = self.read('i')
        if not 0 <= count <= 4096:
            raise ValueError(f'invalid legacy array count at {self.cursor - 4}')
        return count

    def name(self):
        index, number = self.read('ii')
        if not 0 <= index < len(self.names) or number < 0:
            raise ValueError(f'invalid resource FName at {self.cursor - 8}')
        return {'name': self.names[index], 'number': number}

    def string(self):
        count = self.read('i')
        if not 1 <= count <= 4096 or self.cursor + count > len(self.data):
            raise ValueError('invalid identity FString length')
        raw = self.data[self.cursor:self.cursor + count]
        self.cursor += count
        if raw[-1] != 0 or b'\0' in raw[:-1]:
            raise ValueError('invalid identity FString terminator')
        return raw[:-1].decode('utf-8')

    def parameter(self):
        result = {'name': self.name(), 'association': self.read('B'), 'index': self.read('i')}
        if result['association'] > 2 or result['index'] < -1:
            raise ValueError('invalid material parameter info')
        return result

    def expression(self, depth=0):
        self.nodes += 1
        if depth >= 64 or self.nodes > 65536:
            raise ValueError('legacy expression complexity limit exceeded')
        start = self.cursor
        tag = self.name()
        kind = tag['name'].removeprefix(PREFIX)
        if tag['number'] or not tag['name'].startswith(PREFIX):
            raise ValueError(f'invalid expression type at {start}')
        result = {'offset': start, 'type': tag['name']}
        if kind in ('VectorParameter', 'ScalarParameter', 'TextureParameter'):
            result['parameter'] = self.parameter()
            if kind == 'TextureParameter':
                # Original Donut stores [0, -1]. The second word differs from
                # the public old SamplerSource enum; preserve its bits without
                # assigning that unverified meaning. Subsequent array counts
                # validate this record boundary.
                result['serialized_texture_words'] = self.read('ii')
            else:
                result['default'] = self.read('4f' if kind == 'VectorParameter' else 'f')
        elif kind == 'Constant':
            result['value'], result['value_type'] = self.read('4f'), self.read('B')
        elif kind in ('ComponentSwizzle', 'FoldedMath', 'AppendVector', 'TrigMath'):
            result['inputs'] = [self.expression(depth + 1)]
            if kind != 'ComponentSwizzle':
                result['inputs'].append(self.expression(depth + 1))
            if kind == 'ComponentSwizzle':
                channels = self.read('4b')
                elements = self.read('B')
                if not 1 <= elements <= 4 or any(not -1 <= x <= 3 for x in channels):
                    raise ValueError('invalid component swizzle')
                result.update(channels=channels, elements=elements)
            elif kind == 'FoldedMath':
                result['operation'], result['value_type'] = self.read('B'), self.read('I')
                if result['operation'] > 5:
                    raise ValueError('invalid folded math operation')
            elif kind == 'AppendVector':
                result['components_a'] = self.read('I')
                if not 1 <= result['components_a'] <= 4:
                    raise ValueError('invalid append component count')
            else:
                result['operation'] = self.read('B')
                if result['operation'] > 6:
                    raise ValueError('invalid trigonometric operation')
        else:
            raise ValueError(f'unsupported expression {tag["name"]} at {start}')
        result['end'] = self.cursor
        return result


def parse_prefix(data, names):
    """Return verified prefix with relative offsets and an explicit unknown tail.

    The post-array collection slot is intentionally left undecoded: later UE
    branches add VT metadata and change compilation output around this boundary.
    """
    if len(data) > MAX_BYTES:
        raise ValueError('legacy resource exceeds size bound')
    reader = Reader(data, names)
    cooked, valid, quality, feature = reader.read('4I')
    if cooked != 1 or valid != 1 or quality > 5 or feature > 6:
        raise ValueError('unexpected cooked material resource header')
    shader_map_hash = bytes(reader.read('20B')).hex()
    platform = reader.read('I')
    if platform != 0:
        raise ValueError('only original PCD3D_SM5 maps have been verified')
    friendly, path = reader.string(), reader.string()
    if not path.startswith('/Game/') or not path.endswith('.' + friendly):
        raise ValueError('inconsistent legacy shader-map identity')
    arrays = {}
    for name in ARRAYS:
        start = reader.cursor
        count = reader.count()
        expressions = [reader.expression() for _ in range(count)]
        arrays[name] = {'offset': start, 'end': reader.cursor, 'count': count,
                        'expressions': expressions}
    cursor = reader.cursor
    return {'complete_shader_map': False,
            'limitations': ['Prefix grammar only; no shader records or runtime loading verified',
                'TextureParameter trailing words are structurally framed but semantics remain unresolved',
                'Empty texture-array labels follow the public pre-freeze serializer order'],
            'cooked': cooked, 'valid': valid,
            'quality_level': quality, 'feature_level': feature,
            'cooked_shader_map_id_hash': shader_map_hash, 'shader_platform': platform,
            'friendly_name': friendly, 'material_path': path,
            'uniform_expression_arrays': arrays, 'expression_nodes': reader.nodes,
            'decoded_prefix_bytes': cursor,
            'first_unknown': {'offset': cursor,
                'description': 'post-texture-array uniform-expression-set/compilation-output fields',
                'next_32_hex': data[cursor:cursor + 32].hex()},
            'undecoded_bytes': len(data) - cursor,
            'resource_sha256': hashlib.sha256(data).hexdigest()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--uexp', type=Path, required=True)
    parser.add_argument('--resource-report', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    try:
        generated = Path(__file__).resolve().parents[1] / 'generated'
        if generated.resolve() not in args.report.resolve().parents:
            raise ValueError('report must stay in porting/generated')
        framing = json.loads(args.resource_report.read_text())
        raw = args.uexp.read_bytes()
        if len(raw) > MAX_BYTES or hashlib.sha256(raw).hexdigest() != framing['uexp_sha256']:
            raise ValueError('uexp identity does not match resource report')
        start, end = framing['resource_start'], framing['resource_end']
        if not 0 <= start < end <= len(raw):
            raise ValueError('invalid resource bounds')
        result = parse_prefix(raw[start:end], framing['names'])
        if result['resource_sha256'] != framing['resource_sha256']:
            raise ValueError('resource identity does not match resource report')
        result.update(package=framing['package'], resource_start=start, resource_end=end,
                      first_unknown_uexp_offset=start + result['first_unknown']['offset'])
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(result, indent=2, allow_nan=False) + '\n')
        print(f'{result["expression_nodes"]} expression nodes; first unknown .uexp offset '
              f'{result["first_unknown_uexp_offset"]}; {args.report}')
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
