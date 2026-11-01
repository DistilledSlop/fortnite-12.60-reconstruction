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
import os
import tempfile
import struct
from pathlib import Path

PREFIX = 'FMaterialUniformExpression'
GENERATED_ROOT = Path(__file__).resolve().parents[1] / 'generated'
MAX_DEPTH = 128
MAX_NODES = 100000


def f32(value):
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value):
        raise ValueError('expected finite number')
    try:
        result = struct.unpack('<f', struct.pack('<f', value))[0]
        if not math.isfinite(result):
            raise ValueError('number outside finite float32 range')
        return result
    except (OverflowError, struct.error) as exc:
        raise ValueError('number outside finite float32 range') from exc


def identity(parameter):
    name = parameter['name']
    result = (name['name'], name['number'], parameter['association'], parameter['index'])
    if not isinstance(result[0], str) or not all(type(x) is int for x in result[1:]):
        raise ValueError('invalid parameter identity')
    return result


def overrides_from_json(data):
    if not isinstance(data, list):
        raise ValueError('overrides must be a list')
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


def evaluate(node, overrides, used, budget=None, depth=0):
    if budget is None:
        budget = [MAX_NODES]
    if depth > MAX_DEPTH:
        raise ValueError('expression depth limit exceeded')
    budget[0] -= 1
    if budget[0] < 0:
        raise ValueError('expression node limit exceeded')
    kind = node['type']
    if not isinstance(kind, str) or not kind.startswith(PREFIX):
        raise ValueError('unsupported expression type ' + str(kind))
    kind = kind[len(PREFIX):]
    if kind == 'Constant':
        if type(node['value_type']) is not int or node['value_type'] not in (1, 2, 4, 8, 15):
            raise ValueError('unsupported constant value type')
        value = node['value']
        if not isinstance(value, list) or len(value) != 4:
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
        raise ValueError('unsupported expression type ' + str(kind))
    inputs = node['inputs']
    if len(inputs) != expected[kind]:
        raise ValueError('invalid input count for ' + kind)
    values = [evaluate(item, overrides, used, budget, depth + 1) for item in inputs]
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
    if type(op) is not int:
        raise ValueError('invalid operation')
    if kind == 'TrigMath':
        if op not in (0, 1):
            raise ValueError('unsupported trig operation ' + str(op))
        fn = (math.sin, math.cos)[op]
        return [f32(fn(v)) for v in a]
    if type(node['value_type']) is not int or node['value_type'] not in (1, 2, 4, 8, 15):
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
    budget = [MAX_NODES]
    for kind in ('vector', 'scalar'):
        array = arrays[kind]
        expressions = array['expressions']
        if len(expressions) != array['count']:
            raise ValueError('expression count mismatch for ' + kind)
        results[kind] = [evaluate(e, overrides, used, budget) for e in expressions]
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
    try:
        run(args)
    except (ValueError, TypeError, KeyError, IndexError, AttributeError, OverflowError, RecursionError, OSError) as exc:
        parser.exit(2, f'{parser.prog}: error: {exc}\n')


def output_path(path):
    resolved = path.resolve()
    if not resolved.is_relative_to(GENERATED_ROOT.resolve()) or resolved == GENERATED_ROOT.resolve():
        raise ValueError('outputs must resolve under porting/generated')
    return resolved


def publish(outputs):
    # Validate and stage every output before replacing either destination.
    staged = []
    try:
        for path, content in outputs:
            with tempfile.NamedTemporaryFile(dir=path.parent, prefix='.' + path.name + '.', delete=False) as stream:
                staged.append(Path(stream.name))
                stream.write(content)
        for temp, (path, _) in zip(staged, outputs):
            os.replace(temp, path)
    finally:
        for temp in staged:
            temp.unlink(missing_ok=True)


def run(args):
    output, metadata_path = output_path(args.output), output_path(args.metadata)
    inputs = {args.report.resolve()}
    if args.overrides:
        inputs.add(args.overrides.resolve())
    if output == metadata_path or output in inputs or metadata_path in inputs:
        raise ValueError('outputs must be distinct from each other and inputs')
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
        'evaluator_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
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
    metadata_bytes = (json.dumps(metadata, indent=2, allow_nan=False) + '\n').encode('utf-8')
    publish([(output, data), (metadata_path, metadata_bytes)])
    print(json.dumps({k: metadata[k] for k in ('material_path', 'numeric_bytes', 'vector_count', 'scalar_count', 'padding_bytes', 'numeric_bytes_sha256')}))


if __name__ == '__main__':
    main()
