#!/usr/bin/env python3
"""Source-verify a bounded frontend inventory and export fresh numeric v2 bundles.

The manifest is an input to a future generic native provider, not runtime proof.
Every material owner has an accepted, frozen-existing, or rejected report row.
"""
import argparse
import collections
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import struct
import sys

HERE = Path(__file__).resolve().parent
GENERATED = HERE.parent / 'generated'
EXTRACTION = Path('/mnt/vibrato/fortnite-extracted-full')


def sibling(name):
    spec = importlib.util.spec_from_file_location(name, HERE / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


bundle = sibling('export_ingress_bundle')
evaluator = sibling('evaluate_uniforms')
mainl = sibling('export_mainl_ingress')
FROZEN_OWNERS = {
    '/Game/UI/Foundation/Materials/Athena_LockerTabMaster.Athena_LockerTabMaster',
    '/Game/UI/Foundation/Materials/M_UI_GradientPanel.M_UI_GradientPanel',
    '/Game/UI/Foundation/Border/Materials/M-Border-PowerToastGlow.M-Border-PowerToastGlow',
    '/Game/UI/Foundation/Materials/M_MenuButton_Master.M_MenuButton_Master',
    '/Game/Athena/UI/Challenges/Art/Donut/M_UI_DonutIcons.M_UI_DonutIcons',
    '/Game/UI/Foundation/Border/Materials/M-Border-MainL.M-Border-MainL',
}

SLATE_TYPES = {'TSlateMaterialShaderVStrue', 'TSlateMaterialShaderVSfalse',
               *('TSlateMaterialShaderPS' + n for n in ('GrayscaleFont', 'Default', 'Custom', 'ColorFont', 'Border'))}
LIMITATIONS = ['No runtime completeness, attachment, uniform evaluation or draw is proved by export',
               'Only seven original Slate records are adapted; all original records remain in each bundle',
               'Textures, collections, unsupported expressions and unproven View bindings are explicitly rejected',
               'Original floating point and original-client pixel parity remain unverified']


def digest(raw):
    return hashlib.sha256(raw).hexdigest()


def encode(value):
    return (json.dumps(value, indent=2, allow_nan=False) + '\n').encode()


def write(path, value):
    with path.open('xb') as handle:
        handle.write(encode(value))


def package_path(path):
    if path.startswith('FortniteGame/Content/'):
        return '/Game/' + path[len('FortniteGame/Content/'):]
    if path.startswith('Engine/Content/'):
        return '/Engine/' + path[len('Engine/Content/'):]
    if path.startswith(('/Game/', '/Engine/')):
        return path
    raise ValueError('unsupported source mount: ' + path)


def direct_parent(report, owner):
    if owner['type'] == 'UMaterial':
        return ''
    if owner['type'] != 'UMaterialInstanceConstant':
        raise ValueError('unsupported owner type')
    candidates = {package_path(row['root']) + '.' + row['objectName'] for row in report['imports']
                  if row['className'] in ('Material', 'MaterialInstanceConstant')
                  and row['objectName'] == owner['parent']}
    if len(candidates) != 1:
        raise ValueError('ambiguous or missing direct parent import')
    return candidates.pop()


def owner_chain(path, owners):
    chain, seen = [], set()
    while path:
        if path in seen:
            raise ValueError('material inheritance cycle')
        if path not in owners:
            raise ValueError('parent outside bounded inventory: ' + path)
        seen.add(path)
        row = owners[path]
        chain.append(row)
        path = row['parent_path']
    if chain[-1]['owner']['type'] != 'UMaterial':
        raise ValueError('inheritance does not terminate in base material')
    return list(reversed(chain))


def display_name(name):
    return name['name'] + ('_' + str(name['number'] - 1) if name['number'] else '')


def serialized_overrides(row):
    if row['owner']['type'] == 'UMaterial':
        return [], []
    found = [x for x in row['report']['instanceParameters'] if x['name'] == row['owner']['name']]
    if len(found) != 1:
        raise ValueError('missing or ambiguous serialized instance parameters')
    instance, values, seen = found[0], [], set()
    for kind, plural in [('scalar', 'scalars'), ('vector', 'vectors')]:
        for item in instance[plural]:
            parameter = item.get('parameterInfo')
            if parameter is None or item['parameter'] != display_name(parameter['name']):
                raise ValueError('missing or inconsistent full parameter identity')
            key = evaluator.identity(parameter)
            if key in seen:
                raise ValueError('duplicate serialized parameter identity')
            seen.add(key)
            values.append(dict(type=kind, parameter=copy.deepcopy(parameter), value=item['value']))
    return values, instance['textures']


def inherited_values(chain, nodes):
    """Merge full identities base-to-child, retaining source arrays independently."""
    graph = {evaluator.identity(n['parameter']): 'scalar' if n['type'] == 'ScalarParameter' else 'vector'
             for n in nodes if n['type'] in ('ScalarParameter', 'VectorParameter')}
    effective, source = {}, []
    for row in chain:
        values, textures = serialized_overrides(row)
        if textures:
            raise ValueError('texture overrides in owner inheritance chain')
        source.append(dict(owner_path=row['owner_path'], values=values, textures=textures,
            serialized_instance_parameters=copy.deepcopy(next((x for x in row['report'].get('instanceParameters', []) if x['name'] == row['owner']['name']), None))))
        for value in values:
            key = evaluator.identity(value['parameter'])
            if key in effective and effective[key]['type'] != value['type']:
                raise ValueError('inherited scalar/vector identity type conflict')
            if key in graph and graph[key] != value['type']:
                raise ValueError('override conflicts with graph parameter type')
            effective[key] = value
    used = [v for k, v in effective.items() if k in graph]
    unused = [v for k, v in effective.items() if k not in graph]
    return source, list(effective.values()), used, unused


def numeric_contract(parsed):
    if not parsed['complete_serialized_shader_map'] or parsed['undecoded_bytes'] or parsed['shader_record_count'] != 19:
        raise ValueError('unsupported or incomplete shader record shape')
    arrays = parsed['uniform_expression_arrays']
    if any(a['count'] for n, a in arrays.items() if n.startswith('texture_')):
        raise ValueError('texture expressions')
    if parsed['parameter_collections']['count'] or parsed['virtual_texture_stacks']['count']:
        raise ValueError('parameter collections or virtual texture stacks')
    nodes, roots = bundle.fixture_exporter.flatten_expressions(arrays)
    if not nodes or not roots['vector_roots'] and not roots['scalar_roots']:
        raise ValueError('numeric provider requires nonempty numeric graph')
    records = parsed['material_shaders']['records']
    if len(records) != 7 or {r['type']['name'] for r in records} != SLATE_TYPES:
        raise ValueError('unsupported Slate shader record set')
    size = 16 * len(roots['vector_roots']) + 16 * ((len(roots['scalar_roots']) + 3) // 4)
    layout = dict(constant_bytes=size + 16, resource_offsets=[size, size + 8], resource_types=[8, 8])
    if any(r['debug_uniform_buffer_layout'] != layout for r in records):
        raise ValueError('unsupported numeric material layout')
    return nodes, roots, layout


def numeric_cases(parsed, nodes, inherited):
    """Exercise full parameter identities against graph defaults or owner values."""
    cases, mutations, seen = [], [], set()
    baseline = {evaluator.identity(v['parameter']): copy.deepcopy(v) for v in inherited}
    graph_identities = {evaluator.identity(n['parameter']) for n in nodes if n['type'] in ('ScalarParameter', 'VectorParameter')}
    def append(name, overrides):
        effective = dict(baseline)
        effective.update({evaluator.identity(v['parameter']): v for v in overrides
                          if evaluator.identity(v['parameter']) in graph_identities})
        raw = evaluator.evaluate_report(parsed, evaluator.overrides_from_json(list(effective.values())))[0]
        cases.append(dict(name=name, overrides=overrides, numeric_values=list(struct.unpack('<' + 'f' * (len(raw)//4), raw)),
            numeric_sha256=digest(raw), abs_tolerance=0.00001, rel_tolerance=0.00001))
    append('defaults', [])
    for node in nodes:
        if node['type'] not in ('ScalarParameter', 'VectorParameter'):
            continue
        key = evaluator.identity(node['parameter'])
        if key in seen:
            continue
        seen.add(key)
        kind = 'scalar' if node['type'] == 'ScalarParameter' else 'vector'
        if sum(x['type'] == kind for x in mutations) >= 2:
            continue
        value = 0.375 if kind == 'scalar' else [0.15, 0.65, 0.35, 0.85]
        entry = dict(type=kind, parameter=copy.deepcopy(node['parameter']), value=value)
        mutations.append(entry)
        append('override_' + str(len(mutations)), [entry])
    if mutations:
        append('combined', mutations)
        for field in ('association', 'index', 'fname_number'):
            entry = copy.deepcopy(mutations[0])
            if field == 'association':
                options = [v for v in range(3) if v != entry['parameter']['association']]
                for value in options:
                    entry['parameter']['association'] = value
                    if evaluator.identity(entry['parameter']) not in graph_identities:
                        break
                else:
                    continue
            elif field == 'index':
                entry['parameter']['index'] += 17
                while evaluator.identity(entry['parameter']) in graph_identities:
                    entry['parameter']['index'] += 1
            else:
                entry['parameter']['name']['number'] += 17
                while evaluator.identity(entry['parameter']) in graph_identities:
                    entry['parameter']['name']['number'] += 1
            append('unmatched_' + field, [entry])
    return cases


def feature_view_draw_policy(owner_path):
    """Require a visible timer delta only for the observed master diagnostic."""
    require_delta = owner_path == '/Game/UI/Foundation/Buttons/Materials/Feature/M_FeatureButton_Master.M_FeatureButton_Master'
    return dict(require_time_delta=require_delta, equal_image_pairs=[[0,2],[1,3]],
                different_image_pairs=[[0,1],[2,3]] if require_delta else [],
                scope='Controlled Feature GameTime sensitivity and RealTime isolation; no original-client pixel or full View ABI claim',
                time_delta_rationale=('The master diagnostic has demonstrated visible GameTime sensitivity' if require_delta else
                    'Source appearance parameters can mask shine; inherited time deltas are observational and zero changed pixels does not establish original pixel parity'))


def add_feature_view_profile(fixture, parsed, inherited, owner_path):
    """Controlled original Feature PS diagnostic using its observed timer slots."""
    programs = {3558:'66e1f3e0fbc687787eafa1e161c645c53133e7994e268f49066bf3a7ea43dc26',
                3559:'4fc49bec9850cc07fe798e555f9313e2b9d2d6cb9de0c03c56ec818046cfbd80',
                3560:'c9e58e0225667506e992122c8ec7f6f30be1df6d395d43f7e91021a885906ee0',
                3561:'6105271fc367f60a923b584359ca111ebe9a65ccd5e419014a4187ce543e9b56',
                3562:'4cf28e50c64a4fb0454c137d8f4f3c9ba9d8cad91be9234c61cac39800c6db11'}
    observed = {s['record']['library_association']['index']:s['reflection']['spirv_sha256']
                for s in fixture['shaders'] if s['record']['frequency'] == 3}
    if observed != programs:
        fixture['view_draw_profile_status'] = dict(status='unsupported', reason='not the inspected Feature shader programs')
        return
    parameter = dict(name=dict(name='UseShineSwipe', number=0), association=2, index=-1)
    if not any(n['type'] == 'ScalarParameter' and n['parameter'] == parameter for n in fixture['nodes']):
        fixture['view_draw_profile_status'] = dict(status='unsupported', reason='missing exact source UseShineSwipe scalar identity')
        return
    effective = {evaluator.identity(v['parameter']):copy.deepcopy(v) for v in inherited}
    effective[evaluator.identity(parameter)] = dict(type='scalar', parameter=parameter, value=1.0)
    overrides = list(effective.values())
    raw = evaluator.evaluate_report(parsed, evaluator.overrides_from_json(overrides))[0]
    values = list(struct.unpack('<'+'f'*(len(raw)//4),raw))
    if len(values) <= 99:
        fixture['view_draw_profile_status'] = dict(status='unsupported', reason='Feature timer slots absent')
        return
    threshold, period = values[97:99]
    if not 0 <= threshold < period or period - threshold <= 0.25 or values[99] != 1.0:
        fixture['view_draw_profile_status'] = dict(status='unsupported', reason='source effective Feature timer/enable slots differ from inspected contract')
        return
    active = threshold + 0.25
    fixture['view_cases'] = [dict(name='GameTime_%g_RealTime_%g' % (game, real), overrides=copy.deepcopy(overrides),
        numeric_values=values, numeric_sha256=digest(raw), abs_tolerance=0.00001, rel_tolerance=0.00001,
        game_time=game, real_time=real, expected_game_time=game, expected_real_time=real)
        for game, real in ((0.0,10.0),(active,10.0),(0.0,20.0),(active,20.0))]
    fixture['view_draw_expectations'] = dict(**feature_view_draw_policy(owner_path),
        shader_time_window=dict(register='cb2_2[24].yz', threshold=threshold, period=period, active_time=active,
            threshold_numeric_index=97, period_numeric_index=98, enable_register='cb2_2[24].w', enable_numeric_index=99,
            source_default_ps_library_index=3559, source_default_ps_spirv_sha256=programs[3559],
            view_register='cb1_1[141].z', view_byte_offset=2264,
            rationale='Zero precedes the source timer threshold; threshold+0.25 exercises the inspected active window'))
    fixture['view_draw_profile_status'] = dict(status='source_derived', source_programs=programs,
        access_proof_sha256=fixture['view_contract']['access_proof_sha256'])


def load_view_evidence(access_path, metadata_path):
    if access_path is None and metadata_path is None:
        return None
    if access_path is None or metadata_path is None:
        raise ValueError('View access report and original metadata must be supplied together')
    access, metadata = [json.loads(p.read_bytes()) for p in (access_path, metadata_path)]
    expected = dict(struct='FViewUniformShaderParameters', shader_variable='View', member='GameTime',
                    byte_offset=2264, type='float32 scalar', original_metadata_base_type_byte=4,
                    rows=1, columns=1, elements=0, original_struct_size=3952)
    if metadata['conclusion'] != expected:
        raise ValueError('original View metadata does not prove the GameTime field contract')
    original = metadata['original_metadata']
    if original['member_helper'] != '0x144502fe0' or original['constructor'] != '0x140555220':
        raise ValueError('original View metadata constructor identity changed')
    for item in metadata['raw_original_slices']:
        if digest(bytes.fromhex(item['hex'])) != item['sha256']:
            raise ValueError('original metadata evidence slice checksum mismatch')
    if access['schema'] != 'fortnite.shader-view-access.v1':
        raise ValueError('unsupported View access proof schema')
    inspector = sibling('inspect_view_access')
    if access.get('inspector_sha256') != digest((HERE / 'inspect_view_access.py').read_bytes()):
        raise ValueError('View access inspector provenance changed')
    grammar = inspector.Grammar(Path(access['grammar_path']))
    if grammar.sha256 != access['grammar_sha256']:
        raise ValueError('View proof grammar provenance changed')
    records = {r['shader_index']: r for r in access['records']}
    if not records or len(records) != len(access['records']):
        raise ValueError('empty or duplicate View access records')
    contract = dict(buffer_name='View', struct_name='FViewUniformShaderParameters', field_name='GameTime',
        scalar_type='float32', original_byte_offset=2264, expected_current_byte_offset=2264,
        original_struct_bytes=3952, shader_visible_bytes=2272, proof_path=str(metadata_path.resolve()),
        proof_sha256=digest(metadata_path.read_bytes()), original_member_constructor=original['member_helper'],
        original_struct_constructor=original['constructor'], access_proof_path=str(access_path.resolve()),
        access_proof_sha256=digest(access_path.read_bytes()), shader_access_records=access['records'])
    return dict(contract=contract, records=records, grammar=grammar, inspector=inspector,
                sources=[source_receipt(p) for p in (access_path, metadata_path)])


def verify_view_shader(reflected, evidence):
    if evidence is None:
        raise ValueError('View shader lacks explicit per-shader access evidence')
    proof = evidence['records'].get(reflected['shader_index'])
    if proof is None or proof['spirv_sha256'] != reflected['spirv_sha256']:
        raise ValueError('View shader lacks matching module access proof')
    actual = evidence['inspector'].verify_record(reflected, evidence['grammar'])
    if {k: v for k, v in proof.items() if k != 'output'} != {k: v for k, v in actual.items() if k != 'output'}:
        raise ValueError('View shader access proof differs from fresh module inspection')
    return proof


def typed_contract(record, reflected, view_evidence=None):
    """Validate the current numeric provider's exact typed descriptor contract."""
    bindings = []
    uniform_names = {p['binding']['base_index']: p['struct_name']['name']
                     for p in record['uniform_parameters'] if p['binding']['bound']}
    material = record['material_uniform_buffer']
    vertex = record['frequency'] == 0
    semantics = []
    has_view = bool(uniform_names)
    if has_view:
        if uniform_names != {1: 'FViewUniformShaderParameters'} or vertex:
            raise ValueError('unsupported uniform buffer bindings')
        verify_view_shader(reflected, view_evidence)
    for desc in reflected['descriptors']:
        register = desc['d3d_register']
        slot, kind = register['index'], register['kind']
        if register['space'] != 0 or desc['set'] != 0:
            raise ValueError('nonzero register space or descriptor set')
        if desc['resource_table_entries']:
            raise ValueError('unexpected numeric material resource table entries')
        if kind == 'cbv' and desc['vulkan_descriptor_type'] == 6:
            name = uniform_names.get(slot)
            if name is not None:
                if name != 'FViewUniformShaderParameters' or slot != 1 or desc['block_span_bytes'] != 2272:
                    raise ValueError('unsupported View descriptor shape')
                semantic = 'View.GameTime'
            elif slot == 0 and desc['block_span_bytes'] == (80 if vertex else 64):
                semantic = 'SlateLooseConstants'
            elif (not vertex and slot == (2 if has_view else 1) and material == dict(base_index=slot, bound=True) and
                    (0 < desc['block_span_bytes'] <= record['debug_uniform_buffer_layout']['resource_offsets'][0] if has_view else
                     desc['block_span_bytes'] == record['debug_uniform_buffer_layout']['resource_offsets'][0])):
                semantic = 'Material'
            else:
                raise ValueError('unclassified constant buffer register or span ' + str(slot))
        elif not vertex and slot == 0 and kind == 'sampler' and desc['vulkan_descriptor_type'] == 0:
            semantic = 'ElementTextureSampler'
        elif not vertex and slot == 0 and kind == 'srv' and desc['vulkan_descriptor_type'] == 2:
            semantic = 'ElementTexture'
        else:
            raise ValueError('unsupported typed shader descriptor')
        semantics.append(semantic)
        bindings.append(dict(semantic=semantic, d3d_register=register, descriptor=desc))
    expected = {'SlateLooseConstants'} if vertex else {'SlateLooseConstants', 'Material'}
    if has_view:
        expected.add('View.GameTime')
    if 'ElementTexture' in semantics or 'ElementTextureSampler' in semantics:
        expected |= {'ElementTexture', 'ElementTextureSampler'}
    if len(semantics) != len(set(semantics)) or set(semantics) != expected:
        raise ValueError('incomplete or duplicate typed binding contract')
    return bindings


def run_tool(name, arguments, log):
    command = [sys.executable, '-I', str(HERE / (name + '.py')), *map(str, arguments)]
    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    with log.open('xb') as handle:
        handle.write(result.stdout)
    if result.returncode:
        raise ValueError(name + ' failed; see ' + str(log))


def source_receipt(path):
    raw = path.read_bytes()
    return dict(path=str(path.resolve()), bytes=len(raw), sha256=digest(raw))


def load_inventory(path):
    summary = json.loads(path.read_bytes())
    owners = {}
    for entry in summary['entries']:
        if not entry['material_reports']:
            continue
        report_path = Path(entry['report'])
        report, _ = mainl.json_report(report_path)
        if report['materials'] != entry['material_reports'] or package_path(report['path']) != package_path(entry['package']):
            raise ValueError('inventory/report owner mismatch')
        sources = [source_receipt(report_path), source_receipt(Path(entry['export_map']))]
        for suffix in ('uasset', 'uexp'):
            original = EXTRACTION / (entry['package'] + '.' + suffix)
            staged = Path(entry['source_' + suffix])
            if original.read_bytes() != staged.read_bytes():
                raise ValueError('closure bytes differ from immutable source: ' + entry['package'])
            sources.append(source_receipt(original))
        for owner in report['materials']:
            owner_path = package_path(report['path']) + '.' + owner['name']
            if owner_path in owners:
                raise ValueError('duplicate owner')
            owners[owner_path] = dict(owner=owner, owner_path=owner_path, report=report, entry=entry,
                report_path=report_path, parent_path=direct_parent(report, owner), sources=sources)
    return owners


def load_resource(row, archive):
    report_path = row['report_path']
    framing_path = report_path.with_suffix('.inline-resource.json')
    decoded_path = report_path.with_suffix('.decoded.json')
    if not framing_path.exists() or not decoded_path.exists():
        raise ValueError('own resource did not decode in bounded inventory: ' + row['entry']['resource_status'])
    framing, decoded = [json.loads(p.read_bytes()) for p in (framing_path, decoded_path)]
    export_map = Path(row['entry']['export_map'])
    if digest(export_map.read_bytes()) != framing['export_map_sha256']:
        raise ValueError('export map provenance changed')
    if package_path(framing['package']) != row['owner_path'].split('.')[0] or framing['export_name'] != row['owner']['name']:
        raise ValueError('framing owner identity mismatch')
    original = EXTRACTION / (row['entry']['package'] + '.uexp')
    payload = bundle.validate_source(original.read_bytes(), framing)
    parsed = bundle.decoder.parse_prefix(payload, framing['names'])
    bundle.decoder.associate_library(parsed, archive)
    for key, value in parsed.items():
        if key == 'original_serializer_evidence':
            if any(decoded[key].get(k) != v for k, v in value.items()):
                raise ValueError('serializer evidence changed')
        elif decoded.get(key) != value:
            raise ValueError('source complete parse changed: ' + key)
    code = []
    for record in bundle.all_records(parsed):
        index = record['library_association']['index']
        raw = archive.shader_code(index)
        code.append(dict(type=record['type'], output_hash=record['output_hash']['hash'], library_index=index,
                         frequency=record['frequency'], bytes=len(raw), sha256=digest(raw)))
    return dict(parsed=parsed, framing=framing, payload=payload, code=code,
                sources=[source_receipt(p) for p in (framing_path, decoded_path)])


def export_batch(inventory, output, view_access_report=None, view_original_metadata=None):
    output = output.resolve()
    if GENERATED.resolve() not in output.parents or output.exists():
        raise ValueError('choose a fresh batch directory under porting/generated')
    view_evidence = load_view_evidence(view_access_report, view_original_metadata)
    owners = load_inventory(inventory)
    output.mkdir(parents=True)
    archive_path = EXTRACTION / 'FortniteGame/Content/ShaderArchive-FortniteGame-PCD3D_SM5.ushaderbytecode'
    archive_sha = bundle.library_reader.sha256(archive_path)
    candidates, reports, cache = [], [], {}
    with bundle.library_reader.ShaderArchive(archive_path, layout='fortnite-12.60') as archive:
        for path, row in sorted(owners.items()):
            status = dict(owner_path=path, owner_type=row['owner']['type'], parent_path=row['parent_path'], source_receipts=row['sources'])
            reports.append(status)
            try:
                chain = owner_chain(path, owners)
                status['shader_identity'] = chain[0]['owner_path']
                resource_owner = next(x for x in reversed(chain) if x['owner']['type'] == 'UMaterial' or x['owner']['staticPermutation'] is True)
                resource_path = resource_owner['owner_path']
                status['resource_owner_path'] = resource_path
                if resource_path not in cache:
                    cache[resource_path] = load_resource(resource_owner, archive)
                resource = cache[resource_path]
                parsed = resource['parsed']
                if parsed['material_path'] != chain[0]['owner_path']:
                    raise ValueError('shader base identity differs from actual inheritance root')
                nodes, roots, layout = numeric_contract(parsed)
                source, effective, used, unused = inherited_values(chain, nodes)
                candidates.append(dict(row=row, chain=chain, status=status, resource=resource,
                    nodes=nodes, roots=roots, layout=layout, serialized=source, effective=effective, used=used, unused=unused))
                status['status'] = 'pending_shader_contract'
            except (ValueError, KeyError, OSError) as error:
                status.update(status='rejected', reason=str(error))
    indices = sorted({r['library_association']['index'] for c in candidates for r in c['resource']['parsed']['material_shaders']['records']})
    mapped, reflected = {}, {}
    for number, start in enumerate(range(0, len(indices), 256)):
        chunk = output / ('shaders-%03d' % number)
        chunk.mkdir()
        args = ['--archive', archive_path, '--max-shaders', 256, '--extract-dir', chunk / 'original', '--report', chunk / 'archive.json']
        for index in indices[start:start + 256]:
            args += ['--shader-index', index]
        run_tool('shader_archive', args, chunk / 'archive.log')
        run_tool('inspect_d3d_code', ['--source-report', chunk / 'archive.json', '--report', chunk / 'd3d.json'], chunk / 'd3d.log')
        run_tool('translate_mapped_dxbc', ['--source-report', chunk / 'archive.json', '--d3d-report', chunk / 'd3d.json', '--output', chunk / 'mapped', '--limit', 256], chunk / 'mapped.log')
        mapped_path = chunk / 'mapped/mapped-translation-report.json'
        run_tool('reflect_mapped_spirv', ['--mapped-report', mapped_path, '--report', chunk / 'reflection.json'], chunk / 'reflection.log')
        md, rd = [json.loads(p.read_bytes()) for p in (mapped_path, chunk / 'reflection.json')]
        if md['truncated'] or md['archive_sha256'] != archive_sha or rd['mapped_report_sha256'] != digest(mapped_path.read_bytes()):
            raise ValueError('mapped shader chunk provenance mismatch')
        mapped.update({r['shader_index']: r for r in md['records']})
        reflected.update({r['shader_index']: r for r in rd['records']})
    entries = []
    bundle_dir = output / 'bundles'
    bundle_dir.mkdir()
    for c in candidates:
        status, resource, row = c['status'], c['resource'], c['row']
        status['shader_evidence'] = [dict(shader_index=r['library_association']['index'], shader_type=r['type'], mapped_output=mapped[r['library_association']['index']]['output'], reflection=reflected[r['library_association']['index']]) for r in resource['parsed']['material_shaders']['records']]
        parsed = resource['parsed']
        try:
            shaders, contracts = [], []
            for record in parsed['material_shaders']['records']:
                index = record['library_association']['index']
                shader, reflection = mapped[index], reflected[index]
                raw = Path(reflection['output']).read_bytes()
                if not shader['validated'] or shader['unmatched_resource_table_entries'] or shader['compiler_diagnostics'] or digest(raw) != shader['output_sha256'] or digest(raw) != reflection['spirv_sha256']:
                    raise ValueError('unvalidated, diagnostic or unmatched mapped shader')
                if shader['frequency'] != record['frequency'] or reflection['frequency'] != record['frequency']:
                    raise ValueError('mapped shader frequency mismatch')
                bindings = typed_contract(record, reflection, view_evidence)
                contracts.append(dict(shader_type=record['type'], library_index=index, bindings=bindings))
                shaders.append(dict(record=record, mapped=shader, reflection=reflection, spirv_sha1=hashlib.sha1(raw).hexdigest()))
            fixture = dict(schema='fortnite.legacy-slate.v2', material={k: parsed[k] for k in ('friendly_name', 'material_path', 'quality_level', 'feature_level', 'cooked_shader_map_id_hash', 'debug_description')},
                expected_layout=c['layout'], nodes=c['nodes'], **c['roots'], shaders=shaders,
                numeric_comparison_bytes=16*len(c['roots']['vector_roots'])+4*len(c['roots']['scalar_roots']),
                uniform_cases=numeric_cases(parsed, c['nodes'], []), owner_uniform_cases=numeric_cases(parsed, c['nodes'], c['used']),
                owner_parameter_expectations=c['effective'], unused_owner_parameter_expectations=c['unused'],
                serialized_override_chain=c['serialized'], graph_effective_parameter_expectations=c['used'],
                provenance=dict(resource_sha256=parsed['resource_sha256'], shader_library_sha256=archive_sha))
            requires_view = any(x['semantic'] == 'View.GameTime' for contract in contracts for x in contract['bindings'])
            if requires_view:
                fixture['view_contract'] = copy.deepcopy(view_evidence['contract'])
                fixture['view_contract']['shader_access_records'] = [view_evidence['records'][x['record']['library_association']['index']] for x in shaders if x['record']['frequency'] == 3]
                add_feature_view_profile(fixture, parsed, c['used'], row['owner_path'])
            decoded = dict(parsed, package=resource['framing']['package'], resource_start=resource['framing']['resource_start'], resource_end=resource['framing']['resource_end'], shader_library=dict(path=str(archive_path), sha256=archive_sha))
            data = dict(schema='fortnite.legacy-material-ingress.v2', owner_path=row['owner_path'], owner_type=row['owner']['type'],
                parent_path=row['parent_path'], shader_identity=status['shader_identity'], resource_owner_path=status['resource_owner_path'],
                framing=resource['framing'], decoded=decoded, runtime_fixture=fixture,
                resource_raw_hex=resource['payload'].hex(), original_shader_code=resource['code'], shader_binding_contract=contracts,
                provenance=dict(inputs=[s for x in c['chain'] for s in x['sources']] + resource['sources'] + (view_evidence['sources'] if requires_view else []),
                    resource_sha1=hashlib.sha1(resource['payload']).hexdigest(), shader_library=dict(path=str(archive_path), sha256=archive_sha),
                    exporter_sha256=digest(Path(__file__).read_bytes())), limitations=LIMITATIONS)
            target = bundle_dir / (hashlib.sha256(row['owner_path'].encode()).hexdigest()[:20] + '.json')
            write(target, data)
            raw = target.read_bytes()
            item = dict(owner_path=row['owner_path'], owner_type=row['owner']['type'], parent_path=row['parent_path'], shader_identity=status['shader_identity'],
                resource_owner_path=status['resource_owner_path'], resource_sha1=data['provenance']['resource_sha1'], resource_sha256=parsed['resource_sha256'], bundle_path=str(target),
                bundle_sha1=hashlib.sha1(raw).hexdigest(), bundle_sha256=digest(raw), bundle_schema=data['schema'], requires_view=requires_view)
            status.update(item, status='frozen_existing' if row['owner_path'] in FROZEN_OWNERS else 'accepted', serialized_override_count=sum(len(x['values']) for x in c['serialized']), effective_override_count=len(c['effective']), unused_effective_override_count=len(c['unused']))
            if status['status'] == 'accepted':
                entries.append(item)
        except (ValueError, KeyError, OSError) as error:
            status.update(status='rejected', reason=str(error))
    tools = [source_receipt(p) for p in sorted(HERE.glob('*.py')) if not p.name.startswith('test_')]
    manifest = dict(schema='fortnite.legacy-material-manifest.v1', entries=entries, frozen_existing_owners=sorted(FROZEN_OWNERS), limitations=LIMITATIONS,
        provenance=dict(inventory=source_receipt(inventory), shader_library=source_receipt(archive_path), tools=tools))
    write(output / 'manifest.json', manifest)
    manifest_sha = digest((output / 'manifest.json').read_bytes())
    report = dict(schema='fortnite.frontend-numeric-batch-report.v1', inventory=source_receipt(inventory), owners=reports,
        counts=dict(collections.Counter(x['status'] for x in reports)), owner_count=len(reports), unique_selected_slate_shaders=len(indices),
        manifest_path=str(output / 'manifest.json'), manifest_sha256=manifest_sha,
        manifest_own_resource_owners=sum(x['owner_path'] == x['resource_owner_path'] for x in entries),
        manifest_inherited_owners=sum(x['owner_path'] != x['resource_owner_path'] for x in entries),
        runtime_loading_verified=False, limitations=LIMITATIONS)
    write(output / 'report.json', report)
    print(json.dumps({k: report[k] for k in ('counts', 'owner_count', 'unique_selected_slate_shaders', 'manifest_path', 'manifest_sha256')}))
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--inventory', type=Path, default=GENERATED / 'frontend-probes/frontend-material-inventory/summary.json')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--view-access-report', type=Path)
    parser.add_argument('--view-original-metadata', type=Path)
    args = parser.parse_args()
    try:
        export_batch(args.inventory, args.output, args.view_access_report, args.view_original_metadata)
    except (ValueError, KeyError, OSError, TypeError) as error:
        parser.exit(1, 'error: ' + str(error) + '\n')


if __name__ == '__main__':
    main()
