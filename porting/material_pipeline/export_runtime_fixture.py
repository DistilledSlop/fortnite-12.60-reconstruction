#!/usr/bin/env python3
"""Export verified original Locker data for the live UE material bridge probe."""
import argparse
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import struct

HERE = Path(__file__).resolve().parent
GENERATED = HERE.parent / 'generated'
ROOT = GENERATED / 'material_pipeline'
spec = importlib.util.spec_from_file_location('uniform_evaluator', HERE / 'evaluate_uniforms.py')
evaluator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(evaluator)
PREFIX = 'FMaterialUniformExpression'
KINDS = {'Constant', 'ScalarParameter', 'VectorParameter', 'FoldedMath', 'AppendVector',
         'ComponentSwizzle', 'TrigMath', 'Max', 'Saturate'}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def flatten_expressions(arrays):
    nodes, roots = [], {}
    def visit(expression, depth=0):
        if depth >= 64 or len(nodes) >= 4096:
            raise ValueError('runtime fixture expression bound exceeded')
        kind = expression['type'].removeprefix(PREFIX)
        if not expression['type'].startswith(PREFIX) or kind not in KINDS:
            raise ValueError('unsupported runtime expression ' + kind)
        item = {k: copy.deepcopy(v) for k, v in expression.items() if k not in ('offset', 'end', 'inputs', 'type')}
        item['type'] = kind
        if 'inputs' in expression:
            item['inputs'] = [visit(child, depth + 1) for child in expression['inputs']]
        if len(nodes) >= 4096:
            raise ValueError('runtime fixture expression bound exceeded')
        index = len(nodes)
        nodes.append(item)
        return index
    for name in ('vector', 'scalar'):
        array = arrays[name]
        if len(array['expressions']) != array['count']:
            raise ValueError('runtime expression count mismatch')
        roots[name + '_roots'] = [visit(expression) for expression in array['expressions']]
    return nodes, roots


def make_uniform_cases(material, nodes):
    defaults, _, _ = evaluator.evaluate_report(material)
    parameters = {}
    for node in nodes:
        if node['type'] in ('ScalarParameter', 'VectorParameter'):
            name = node['parameter']['name']['name']
            value = ('scalar' if node['type'] == 'ScalarParameter' else 'vector', node['parameter'])
            if name in parameters and parameters[name] != value:
                raise ValueError('test parameter name has multiple identities')
            parameters[name] = value
    cases, overrides = [], []
    def append(name, entries, expected=None):
        data = expected if expected is not None else evaluator.evaluate_report(
            material, evaluator.overrides_from_json(entries))[0]
        cases.append(dict(name=name, overrides=entries,
                          numeric_values=list(struct.unpack('<' + 'f' * (len(data) // 4), data)),
                          numeric_sha256=digest(data), abs_tolerance=0.00001, rel_tolerance=0.00001))
    append('defaults', [], defaults)
    for name, value in [('Rotation_U', 0.25), ('Offset_U', 0.125),
                        ('RBGA1', [0.1, 0.3, 0.7, 0.9]), ('SelectionColor', [0.8, 0.2, 0.4, 1.0])]:
        kind, parameter = parameters[name]
        entry = dict(type=kind, parameter=copy.deepcopy(parameter), value=value)
        overrides.append(entry)
        append(name, [entry])
    append('combined', overrides)
    for field, value in [('association', 0), ('index', 3)]:
        entry = copy.deepcopy(overrides[0])
        if entry['parameter'][field] == value:
            raise ValueError('identity-isolation fixture accidentally matches')
        entry['parameter'][field] = value
        append('unmatched_' + field, [entry], defaults)
    return cases


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--material', type=Path, default=ROOT / 'locker-legacy-prefix.json')
    parser.add_argument('--mapped-report', type=Path, default=ROOT / 'frontend-mapped/mapped-translation-report.json')
    parser.add_argument('--reflection-report', type=Path, default=ROOT / 'mapped-spirv-reflection.json')
    parser.add_argument('--output', type=Path, default=ROOT / 'locker-runtime-fixture.json')
    args = parser.parse_args()
    try:
        if GENERATED.resolve() not in args.output.resolve().parents:
            raise ValueError('runtime fixture output must stay in porting/generated')
        paths = [args.material, args.mapped_report, args.reflection_report]
        if args.output.resolve() in {p.resolve() for p in paths}:
            raise ValueError('runtime fixture must not overwrite an input')
        data = [p.read_bytes() for p in paths]
        material, mapped, reflection = [json.loads(d) for d in data]
        if (not material['complete_serialized_shader_map'] or material['undecoded_bytes']
                or material['shader_library']['sha256'] != mapped['archive_sha256']
                or reflection['mapped_report_sha256'] != digest(data[1])):
            raise ValueError('material and mapped shader identities do not align')
        if any(array['count'] for name, array in material['uniform_expression_arrays'].items()
               if name.startswith('texture_')) or material['virtual_texture_stacks']['count']:
            raise ValueError('first runtime fixture supports numeric Locker material only')
        nodes, roots = flatten_expressions(material['uniform_expression_arrays'])
        if len(roots['vector_roots']) != 12 or len(roots['scalar_roots']) != 30:
            raise ValueError('first runtime fixture requires original Locker slot counts')
        mapped_rows = {r['shader_index']: r for r in mapped['records']}
        reflected_rows = {r['shader_index']: r for r in reflection['records']}
        shaders, layouts = [], []
        for record in material['material_shaders']['records']:
            index = record['library_association']['index']
            shader, reflected = mapped_rows[index], reflected_rows[index]
            code_path = Path(shader['output']).resolve()
            if GENERATED.resolve() not in code_path.parents or not shader['validated']:
                raise ValueError('unvalidated/outside-generated SPIR-V')
            code = code_path.read_bytes()
            if (digest(code) != shader['output_sha256'] or digest(code) != reflected['spirv_sha256']
                    or record['frequency'] != shader['frequency']
                    or record['frequency'] != reflected['frequency']):
                raise ValueError('runtime shader bytes/stages differ from receipts')
            layouts.append(record['debug_uniform_buffer_layout'])
            shaders.append(dict(record=record, mapped=shader, reflection=reflected,
                                spirv_sha1=hashlib.sha1(code).hexdigest()))
        if len(shaders) != 7 or any(layout != layouts[0] for layout in layouts):
            raise ValueError('expected seven consistent original Slate records')
        if layouts[0] != dict(constant_bytes=336, resource_offsets=[320, 328], resource_types=[8, 8]):
            raise ValueError('Locker serialized layout differs from checked bridge contract')
        output = dict(schema='fortnite.legacy-slate.v1',
            material={k: material[k] for k in ('friendly_name', 'material_path', 'quality_level', 'feature_level',
                      'cooked_shader_map_id_hash', 'debug_description')},
            expected_layout=layouts[0], nodes=nodes, **roots, shaders=shaders,
            uniform_cases=make_uniform_cases(material, nodes), numeric_comparison_bytes=312,
            provenance=dict(inputs=[dict(path=str(p.resolve()), sha256=digest(d)) for p, d in zip(paths, data)],
                resource_sha256=material['resource_sha256'], shader_library_sha256=mapped['archive_sha256'],
                exporter_sha256=digest(Path(__file__).read_bytes()),
                evaluator_sha256=digest((HERE / 'evaluate_uniforms.py').read_bytes())),
            limitations=['Detached seven-shader input; twelve original mesh records still required for full map coverage',
                         'Expected numeric values use the reference evaluator tolerance; original target FP parity unverified',
                         'Padding and sampler pointer bytes are excluded from numeric comparison',
                         'Runtime construction, resource binding and draw parity require executing the native probe'])
        args.output.parent.mkdir(parents=True, exist_ok=True)
        temporary = args.output.with_suffix('.tmp')
        temporary.write_text(json.dumps(output, indent=2, allow_nan=False) + '\n')
        temporary.replace(args.output)
        print(f'{len(nodes)} nodes; 7 original shaders; {len(output["uniform_cases"])} uniform cases; {args.output}')
    except (OSError, ValueError, KeyError, TypeError, OverflowError, RecursionError) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
