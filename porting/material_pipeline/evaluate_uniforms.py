#!/usr/bin/env python3
"""Evaluate decoded pre-freeze UE uniform expressions, not a material graph.

References: folgerwang/UnrealEngine release, MaterialUniformExpressions.h
(GetNumberValue) and MaterialUniformExpressions.cpp (CreateBufferStruct,
FillUniformBuffer). The report represents serialized defaults, not instance
parameter inheritance. See output limitations before using the bytes.
"""
import argparse
import hashlib
import json
import math
import struct
from pathlib import Path

PREFIX = 'FMaterialUniformExpression'


def f32(value):
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value):
        raise ValueError('expected finite number')
    try:
        return struct.unpack('<f', struct.pack('<f', value))[0]
    except (OverflowError, struct.error) as exc:
        raise ValueError('number outside finite float32 range') from exc


def identity(parameter):
    name = parameter['name']
    return (name['name'], name['number'], parameter['association'], parameter['index'])


def overrides_from_json(data):
    result = {}
    for entry in data:
        kind = entry['type']
        if kind not in ('scalar', 'vector'):
            raise ValueError('override type must be scalar or vector')
        key = (kind, *identity(entry['parameter']))
        if key in result:
            raise ValueError('duplicate parameter override')
        value = entry['value']
        if kind == 'vector':
            if not isinstance(value, list) or len(value) != 4:
                raise ValueError('vector override requires four components')
            value = [f32(v) for v in value]
        else:
            value = f32(value)
        result[key] = value
    return result


def evaluate(node, overrides, used):
    kind = node['type']
    if not kind.startswith(PREFIX):
        raise ValueError('unsupported expression type ' + kind)
    kind = kind[len(PREFIX):]
    if kind == 'Constant':
        if node['value_type'] not in (1, 2, 4, 8, 15):
            raise ValueError('unsupported constant value type')
        value = node['value']
        if len(value) != 4:
            raise ValueError('constant must have four components')
        return [f32(v) for v in value]
    if kind in ('ScalarParameter', 'VectorParameter'):
        typ = 'scalar' if kind == 'ScalarParameter' else 'vector'
        key = (typ, *identity(node['parameter']))
        if key in overrides:
            used.add(key)
        value = overrides.get(key, node['default'])
        if typ == 'scalar':
            return [f32(value)] * 4
        if not isinstance(value, list) or len(value) != 4:
            raise ValueError('vector parameter must have four components')
        return [f32(v) for v in value]
    expected = {'ComponentSwizzle': 1, 'FoldedMath': 2, 'AppendVector': 2, 'TrigMath': 2}
    if kind not in expected:
        raise ValueError('unsupported expression type ' + kind)
    inputs = node['inputs']
    if len(inputs) != expected[kind]:
        raise ValueError('invalid input count for ' + kind)
    values = [evaluate(item, overrides, used) for item in inputs]
    a = values[0]
    if kind == 'ComponentSwizzle':
        channels, n = node['channels'], node['elements']
        if len(channels) != 4 or n not in (1, 2, 3, 4) or any(c not in range(4) for c in channels[:n]) or any(c != -1 for c in channels[n:]):
            raise ValueError('invalid component swizzle')
        return [a[channels[0]]] * 4 if n == 1 else [a[c] for c in channels[:n]] + [0.0] * (4 - n)
    b = values[1]
    if kind == 'AppendVector':
        n = node['components_a']
        if n not in range(5):
            raise ValueError('invalid append component count')
        return a[:n] + b[:4-n]
    op = node['operation']
    if kind == 'TrigMath':
        if op not in (0, 1):
            raise ValueError('unsupported trig operation ' + str(op))
        fn = (math.sin, math.cos)[op]
        return [f32(fn(v)) for v in a]
    if node['value_type'] not in (1, 2, 4, 8, 15):
        raise ValueError('unsupported folded math value type')
    if op not in (0, 1, 2, 3):
        raise ValueError('unsupported folded math operation ' + str(op))
    delta = f32(0.00001)  # DELTA in UE Core math constants
    out = []
    for x, y in zip(a, b):
        if op == 0: value = x + y
        elif op == 1: value = x - y
        elif op == 2: value = x * y
        else:
            divisor = (-delta if y < 0 else delta) if abs(y) < delta else y
            value = x / divisor
        out.append(f32(value))
    return out


def evaluate_report(report, overrides=None):
    overrides = overrides or {}
    arrays = report['uniform_expression_arrays']
    if report['virtual_texture_stacks']['count'] or arrays['texture_virtual']['count']:
        raise ValueError('virtual texture uniform data is unsupported')
    used = set()
    results = {}
    for kind in ('vector', 'scalar'):
        array = arrays[kind]
        expressions = array['expressions']
        if len(expressions) != array['count']:
            raise ValueError('expression count mismatch for ' + kind)
        results[kind] = [evaluate(e, overrides, used) for e in expressions]
    if set(overrides) != used:
        raise ValueError('unused parameter overrides: ' + repr(sorted(set(overrides) - used)))
    values = [component for vector in results['vector'] for component in vector]
    values += [vector[0] for vector in results['scalar']]
    padding = (-len(results['scalar'])) % 4
    values += [0.0] * padding
    data = struct.pack('<' + 'f' * len(values), *values)
    return data, results, padding


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('report', type=Path, help='decoded legacy-prefix JSON')
    parser.add_argument('--overrides', type=Path, help='JSON list of {type, parameter, value}; parameter is the full decoded identity')
    parser.add_argument('--output', type=Path, required=True, help='output numeric constant bytes')
    parser.add_argument('--metadata', type=Path, required=True, help='output provenance and values JSON')
    args = parser.parse_args()
    raw = args.report.read_bytes()
    report = json.loads(raw)
    override_raw = args.overrides.read_bytes() if args.overrides else None
    overrides = overrides_from_json(json.loads(override_raw)) if override_raw is not None else {}
    data, values, padding = evaluate_report(report, overrides)
    debug_layouts = [r['debug_uniform_buffer_layout'] for r in report.get('material_shaders', {}).get('records', []) if 'debug_uniform_buffer_layout' in r]
    for layout in debug_layouts:
        offsets = layout['resource_offsets']
        if offsets and min(offsets) != len(data):
            raise ValueError('numeric span does not match first serialized resource offset')
    metadata = {
        'input_report_sha256': hashlib.sha256(raw).hexdigest(),
        'override_sha256': hashlib.sha256(override_raw).hexdigest() if override_raw is not None else None,
        'resource_sha256': report.get('resource_sha256'),
        'cooked_shader_map_id_hash': report.get('cooked_shader_map_id_hash'),
        'material_path': report.get('material_path'),
        'numeric_bytes_sha256': hashlib.sha256(data).hexdigest(),
        'numeric_bytes': len(data), 'serialized_debug_layouts': debug_layouts,
        'vector_count': len(values['vector']),
        'scalar_count': len(values['scalar']), 'padding_bytes': padding * 4,
        'vector_values': values['vector'], 'scalar_values': [v[0] for v in values['scalar']],
        'texture_counts': {k: v['count'] for k, v in report['uniform_expression_arrays'].items() if k.startswith('texture_')},
        'limitations': [
            'Serialized expression defaults plus explicit overrides only; material instance inheritance and render-proxy lookup are not recovered.',
            'Float32 rounding is modeled per operation; host libm, compiler and target floating-point behavior may differ from the original runtime.',
            'Scalar padding is zeroed here; FillUniformBuffer skips these bytes without initializing them.',
            'Numeric bytes exclude separate texture and sampler resources; no runtime binding or shader semantic equivalence is established.',
        ],
    }
    args.output.write_bytes(data)
    args.metadata.write_text(json.dumps(metadata, indent=2, allow_nan=False) + '\n')
    print(json.dumps({k: metadata[k] for k in ('material_path', 'numeric_bytes', 'vector_count', 'scalar_count', 'padding_bytes', 'numeric_bytes_sha256')}))


if __name__ == '__main__':
    main()
