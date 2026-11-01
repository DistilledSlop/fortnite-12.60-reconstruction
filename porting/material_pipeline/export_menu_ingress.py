#!/usr/bin/env python3
"""Add the proven original GameTime contract to the numeric MenuButton bundle."""
import argparse
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import struct

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('numeric_exporter', HERE / 'export_numeric_ingress.py')
numeric = importlib.util.module_from_spec(spec)
spec.loader.exec_module(numeric)


def source_parameters(fixture):
    result = {}
    for node in fixture['nodes']:
        if node['type'] in ('ScalarParameter', 'VectorParameter'):
            value = dict(type='scalar' if node['type'] == 'ScalarParameter' else 'vector', parameter=node['parameter'])
            name = node['parameter']['name']['name']
            if name in result and result[name] != value:
                raise ValueError('ambiguous source parameter identity')
            result[name] = value
    return result


def add_view_contract(bundle):
    fixture = bundle['runtime_fixture']
    params = source_parameters(fixture)
    material = bundle['decoded']
    def overrides(values):
        result = []
        for name, value in values:
            if name not in params:
                raise ValueError('missing original MenuButton parameter ' + name)
            entry = copy.deepcopy(params[name])
            entry['value'] = value
            result.append(entry)
        return result
    def case(name, entries, evaluate_entries=None):
        raw = numeric.evaluator.evaluate_report(material,
            numeric.evaluator.overrides_from_json(entries if evaluate_entries is None else evaluate_entries))[0]
        return dict(name=name, overrides=entries, numeric_values=list(struct.unpack('<' + 'f'*(len(raw)//4), raw)),
                    numeric_sha256=hashlib.sha256(raw).hexdigest(), abs_tolerance=0.00001, rel_tolerance=0.00001)
    cases = [case('defaults', [])]
    mutations = []
    for name, value in [('Shine Rotation', 0.3), ('Box Size', 0.5),
                         ('Primary Color', [0.9, 0.1, 0.2, 0.8]), ('Base Color', [0.1, 0.8, 0.2, 1])]:
        entries = overrides([(name, value)])
        mutations += entries
        cases.append(case(name, entries))
    cases.append(case('combined', mutations))
    for field, value in [('association', 1), ('index', 7)]:
        entries = copy.deepcopy(cases[1]['overrides'])
        entries[0]['parameter'][field] = value
        cases.append(case('unmatched_' + field, entries, evaluate_entries=[]))
    fixture['uniform_cases'] = cases
    fixture['owner_uniform_cases'] = copy.deepcopy(cases)
    fixture['draw_parameter'] = 'Primary Color'
    proof = numeric.ROOT / 'view-identity/evidence.json'
    if not proof.is_file():
        raise ValueError('original View metadata evidence is required')
    fixture['view_contract'] = dict(buffer_name='View', struct_name='FViewUniformShaderParameters',
        field_name='GameTime', scalar_type='float32', original_byte_offset=2264, expected_current_byte_offset=2264,
        original_struct_bytes=3952, shader_visible_bytes=2272, proof_path=str(proof.resolve()),
        proof_sha256=hashlib.sha256(proof.read_bytes()).hexdigest(),
        original_member_constructor='0x144502fe0', original_struct_constructor='0x140555220')
    # Controlled scene-view inputs use the normal UE view-uniform construction.
    # Changing RealTime alone must not change this program's output.
    time_parameters = overrides([('s_UseShineSwipe', 1.0)])
    view_cases = []
    # Original cb2[14].zw gates the shine window before Fract(abs(GameTime)).
    # Samples 0/1 (and 0/.25) both miss that window. Derive an active phase
    # from the evaluated original material slots, keeping RealTime isolated.
    window = case('time_window_parameters', time_parameters)['numeric_values']
    threshold, period = window[14*4+2:14*4+4]
    if not 0 <= threshold < period or period - threshold < 0.25:
        raise ValueError('original MenuButton time window differs from inspected shader contract')
    active_time = threshold + 0.25
    for game_time, real_time in ((0.0, 10.0), (active_time, 10.0), (0.0, 20.0), (active_time, 20.0)):
        row = case(f'GameTime_{game_time:g}_RealTime_{real_time:g}', time_parameters)
        row.update(game_time=game_time, real_time=real_time, expected_game_time=game_time,
                   expected_real_time=real_time)
        view_cases.append(row)
    fixture['view_cases'] = view_cases
    fixture['view_draw_expectations'] = dict(equal_image_pairs=[[0, 2], [1, 3]],
        different_image_pairs=[[0, 1], [2, 3]],
        shader_time_window=dict(register='cb2_2[14].zw', threshold=threshold, period=period, active_time=active_time),
        scope='Controlled time sensitivity and field isolation, not an original-client pixel oracle')
    bundle['provenance']['menu_exporter_sha256'] = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    return bundle


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--owner-report', type=Path,
                        default=numeric.GENERATED / 'frontend-probes/material-menu-button.json')
    parser.add_argument('--output', type=Path, default=numeric.ROOT / 'menu-button-ingress-bundle.json')
    args = parser.parse_args()
    try:
        if args.output.exists() or numeric.GENERATED.resolve() not in args.output.resolve().parents:
            raise ValueError('choose a fresh output under generated to preserve pinned bundles')
        bundle = add_view_contract(numeric.make_bundle('menu-button', args.owner_report))
        raw = (json.dumps(bundle, indent=2, allow_nan=False)+'\n').encode()
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_bytes(raw)
        print(json.dumps(dict(path=str(args.output.resolve()), owner_path=bundle['owner_path'],
            bundle_sha1=hashlib.sha1(raw).hexdigest(), resource_sha1=bundle['provenance']['resource_sha1'],
            resource_bytes=len(bytes.fromhex(bundle['resource_raw_hex'])), view_contract=bundle['runtime_fixture']['view_contract'])))
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
