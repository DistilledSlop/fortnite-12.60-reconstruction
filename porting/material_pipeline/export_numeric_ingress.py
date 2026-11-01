#!/usr/bin/env python3
"""Export independently verified numeric UI assets, including real MI identity.

Keeps the earlier pinned Locker bundle and exporter unchanged. Original package,
shader identity, parameter overrides and complete resource records remain distinct.
"""
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
EXTRACTION = Path('/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content')


def sibling(name):
    spec = importlib.util.spec_from_file_location(name, HERE / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


bundle = sibling('export_ingress_bundle')
evaluator = sibling('evaluate_uniforms')


def digest(raw):
    return hashlib.sha256(raw).hexdigest()


def package_path(path):
    if path.startswith('FortniteGame/Content/'):
        return '/Game/' + path.removeprefix('FortniteGame/Content/')
    if path.startswith('/Game/'):
        return path
    raise ValueError('unsupported original package mount')


def json_report(path):
    raw = path.read_bytes()
    return json.loads(raw[raw.index(b'{'):]), raw


def numeric_cases(material, parameters, inherited):
    """Proxy perturbations are separate from the real owner's inherited values."""
    result = []
    def append(name, overrides, unmatched=False):
        effective = {evaluator.identity(x['parameter']): copy.deepcopy(x) for x in inherited}
        if not unmatched:
            effective.update({evaluator.identity(x['parameter']): copy.deepcopy(x) for x in overrides})
        raw = evaluator.evaluate_report(material, evaluator.overrides_from_json(list(effective.values())))[0]
        result.append(dict(name=name, overrides=overrides,
            numeric_values=list(struct.unpack('<' + 'f' * (len(raw) // 4), raw)),
            numeric_sha256=digest(raw), abs_tolerance=0.00001, rel_tolerance=0.00001))
    append('defaults', [])
    mutations = []
    for name, value in [('Rotation', 0.25), ('Scale', 0.75), ('RGBA0', [0.8, 0.1, 0.3, 0.25]),
                        ('RGBA1', [0.1, 0.9, 0.2, 0.8])]:
        if name not in parameters:
            continue
        kind, parameter = parameters[name]
        entry = dict(type=kind, parameter=copy.deepcopy(parameter), value=value)
        mutations.append(entry)
        append(name, [entry])
    append('combined', mutations)
    if mutations:
        for field, value in [('association', 1), ('index', 7)]:
            entry = copy.deepcopy(mutations[0])
            entry['parameter'][field] = value
            append('unmatched_' + field, [entry], unmatched=True)
    return result


def make_bundle(key, owner_report):
    framing_path, decoded_path = [ROOT / (key + suffix) for suffix in ('-inline-resource.json', '-legacy-prefix.json')]
    framing_raw, decoded_raw = framing_path.read_bytes(), decoded_path.read_bytes()
    framing, decoded = json.loads(framing_raw), json.loads(decoded_raw)
    owner, owner_raw = json_report(owner_report)
    package = package_path(framing['package'])
    material_owner = next(row for row in owner['materials'] if row['name'] == framing['export_name'])
    owner_path = package + '.' + material_owner['name']
    if package_path(owner['path']) != package:
        raise ValueError('owner report belongs to another package')
    source = EXTRACTION / (package.removeprefix('/Game/') + '.uexp')
    source_raw = source.read_bytes()
    payload = bundle.validate_source(source_raw, framing)
    parsed = bundle.decoder.parse_prefix(payload, framing['names'])
    archive_path = Path(decoded['shader_library']['path'])
    archive_hash = bundle.library_reader.sha256(archive_path)
    if archive_hash != decoded['shader_library']['sha256']:
        raise ValueError('original shader library changed')
    code_receipts = []
    with bundle.library_reader.ShaderArchive(archive_path, layout='fortnite-12.60') as archive:
        bundle.decoder.associate_library(parsed, archive)
        for record in bundle.all_records(parsed):
            index = record['library_association']['index']
            code = archive.shader_code(index)
            code_receipts.append(dict(type=record['type'], output_hash=record['output_hash']['hash'],
                library_index=index, frequency=record['frequency'], bytes=len(code), sha256=digest(code)))
    for name, value in parsed.items():
        if name == 'original_serializer_evidence':
            if any(decoded[name].get(k) != v for k, v in value.items()):
                raise ValueError('original serializer evidence changed')
        elif decoded.get(name) != value:
            raise ValueError('original complete parse changed: ' + name)
    if parsed['undecoded_bytes'] or parsed['shader_record_count'] != 19:
        raise ValueError('original resource is incomplete')
    arrays = parsed['uniform_expression_arrays']
    if any(value['count'] for name, value in arrays.items() if name.startswith('texture_')) or parsed['parameter_collections']['count']:
        raise ValueError('numeric adapter cannot convert texture or collection expressions')
    nodes, roots = bundle.fixture_exporter.flatten_expressions(arrays)
    parameters = {}
    for node in nodes:
        if node['type'] in ('ScalarParameter', 'VectorParameter'):
            name = node['parameter']['name']['name']
            value = ('scalar' if node['type'] == 'ScalarParameter' else 'vector', node['parameter'])
            if name in parameters and parameters[name] != value:
                raise ValueError('parameter name aliases multiple identities')
            parameters[name] = value
    parent_path, expectations = '', []
    if material_owner['type'] == 'UMaterialInstanceConstant':
        imports = [row for row in owner['imports'] if row['className'] == 'Material' and row['objectName'] == material_owner['parent']]
        if len(imports) != 1:
            raise ValueError('actual material-instance parent import is ambiguous')
        parent_path = package_path(imports[0]['root']) + '.' + imports[0]['objectName']
        if parent_path != parsed['material_path']:
            raise ValueError('instance parent does not match this verified shader identity')
        instance = next(row for row in owner['instanceParameters'] if row['name'] == material_owner['name'])
        if instance['textures']:
            raise ValueError('numeric owner unexpectedly overrides textures')
        for kind, plural in [('scalar', 'scalars'), ('vector', 'vectors')]:
            for row in instance[plural]:
                parameter = row['parameterInfo']
                if parameter is None or parameters.get(row['parameter']) != (kind, parameter):
                    raise ValueError('serialized instance parameter identity does not match shader graph')
                expectations.append(dict(type=kind, parameter=parameter, value=row['value']))
    elif material_owner['type'] != 'UMaterial' or owner_path != parsed['material_path']:
        raise ValueError('unsupported actual material owner or shader identity')
    mapped_path, reflection_path = ROOT / 'frontend-mapped/mapped-translation-report.json', ROOT / 'mapped-spirv-reflection.json'
    mapped_raw, reflection_raw = mapped_path.read_bytes(), reflection_path.read_bytes()
    mapped, reflection = json.loads(mapped_raw), json.loads(reflection_raw)
    if mapped['archive_sha256'] != archive_hash or reflection['mapped_report_sha256'] != digest(mapped_raw):
        raise ValueError('mapped shader report provenance changed')
    mapped_rows, reflected_rows = [{row['shader_index']: row for row in doc['records']} for doc in (mapped, reflection)]
    records, shaders = parsed['material_shaders']['records'], []
    layout = records[0]['debug_uniform_buffer_layout']
    numeric_bytes = 16 * len(roots['vector_roots']) + 16 * ((len(roots['scalar_roots']) + 3) // 4)
    if (len(records) != 7 or any(row['debug_uniform_buffer_layout'] != layout for row in records)
            or layout != dict(constant_bytes=numeric_bytes + 16, resource_offsets=[numeric_bytes, numeric_bytes + 8], resource_types=[8, 8])):
        raise ValueError('serialized numeric material layout differs from checked contract')
    for record in records:
        index = record['library_association']['index']
        shader, reflected = mapped_rows[index], reflected_rows[index]
        code_path = Path(reflected['output']).resolve()
        if GENERATED.resolve() not in code_path.parents:
            raise ValueError('mapped shader outside generated')
        code = code_path.read_bytes()
        if (not shader['validated'] or digest(code) != shader['output_sha256'] or digest(code) != reflected['spirv_sha256']
                or shader['frequency'] != record['frequency'] or reflected['frequency'] != record['frequency']):
            raise ValueError('mapped code does not match original library association')
        shaders.append(dict(record=record, mapped=shader, reflection=reflected, spirv_sha1=hashlib.sha1(code).hexdigest()))
    fixture = dict(schema='fortnite.legacy-slate.v2', material={k: parsed[k] for k in ('friendly_name', 'material_path',
        'quality_level', 'feature_level', 'cooked_shader_map_id_hash', 'debug_description')}, expected_layout=layout,
        nodes=nodes, **roots, shaders=shaders, numeric_comparison_bytes=16*len(roots['vector_roots'])+4*len(roots['scalar_roots']),
        uniform_cases=numeric_cases(parsed, parameters, []), owner_uniform_cases=numeric_cases(parsed, parameters, expectations),
        owner_parameter_expectations=expectations, draw_parameter='RGBA1',
        provenance=dict(resource_sha256=parsed['resource_sha256'], shader_library_sha256=archive_hash))
    sources = [(framing_path, framing_raw), (decoded_path, decoded_raw), (owner_report, owner_raw),
               (mapped_path, mapped_raw), (reflection_path, reflection_raw), (source, source_raw),
               (source.with_suffix('.uasset'), source.with_suffix('.uasset').read_bytes())]
    return dict(schema='fortnite.legacy-material-ingress.v2', owner_path=owner_path, owner_type=material_owner['type'],
        parent_path=parent_path, framing=framing, decoded=decoded, runtime_fixture=fixture,
        resource_raw_hex=payload.hex(), original_shader_code=code_receipts,
        provenance=dict(inputs=[dict(path=str(p.resolve()), bytes=len(raw), sha256=digest(raw)) for p, raw in sources],
            resource_sha1=hashlib.sha1(payload).hexdigest(), shader_library=dict(path=str(archive_path), sha256=archive_hash),
            exporter_sha256=digest(Path(__file__).read_bytes())),
        limitations=['Only the named numeric original asset; all 19 records retained, seven Slate records adapted',
                    'Actual owner, inheritance, uniform cases and draw require runtime checks',
                    'Original-client floating-point and pixel parity remain unverified'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--key', choices=('gradient-panel', 'power-toast'), required=True)
    parser.add_argument('--owner-report', type=Path, required=True)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    try:
        output = args.output or ROOT / (args.key + '-ingress-bundle.json')
        if GENERATED.resolve() not in output.resolve().parents or output.exists():
            raise ValueError('choose a fresh output under generated to preserve pinned bundles')
        data = make_bundle(args.key, args.owner_report)
        raw = (json.dumps(data, indent=2, allow_nan=False)+'\n').encode()
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_bytes(raw)
        print(json.dumps(dict(path=str(output.resolve()), owner_path=data['owner_path'], parent_path=data['parent_path'],
            shader_identity=data['decoded']['material_path'], bundle_sha1=hashlib.sha1(raw).hexdigest(),
            resource_sha1=data['provenance']['resource_sha1'], resource_bytes=len(bytes.fromhex(data['resource_raw_hex'])))))
    except (OSError, ValueError, KeyError, TypeError, StopIteration) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
