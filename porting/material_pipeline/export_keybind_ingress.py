#!/usr/bin/env python3
"""Bounded original M_Buttonborder_DF ingress; regular Texture has no parameter identity.

All original records remain present. Ordered references follow the recovered
PostLoad algorithm, with complete zero-texture function closures. No runtime
attachment, resource creation, shader-header generation or draw is performed.
"""
import argparse
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import struct
import subprocess

HERE = Path(__file__).resolve().parent


def sibling(name):
    spec = importlib.util.spec_from_file_location(name, HERE / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


numeric = sibling('export_numeric_ingress')
bundle, evaluator = numeric.bundle, numeric.evaluator
reflection_tool = sibling('reflect_mapped_spirv')
d3d_tool = sibling('inspect_d3d_code')
verifier = sibling('verify_runtime_artifacts')
GENERATED = HERE.parent / 'generated'
ROOT = GENERATED / 'material_pipeline'
EXTRACTED = Path('/mnt/vibrato/fortnite-extracted-full')
PACKAGE = 'FortniteGame/Content/UI/InGame/KeybindWidget/Materials/M_Buttonborder_DF'
OWNER = '/Game/UI/InGame/KeybindWidget/Materials/M_Buttonborder_DF.M_Buttonborder_DF'
TEXTURE_PACKAGE = 'FortniteGame/Content/UI/Foundation/Textures/Icons/Input/Misc/T_Buttonborder_DF'
TEXTURE_OBJECT = TEXTURE_PACKAGE + '.T_Buttonborder_DF'
TEXTURE_PATH = numeric.package_path(TEXTURE_PACKAGE) + '.T_Buttonborder_DF'
LAYOUT = dict(constant_bytes=176, resource_offsets=[144, 152, 160, 168], resource_types=[5, 8, 8, 8])
BASE_TEXTURE = dict(texture_index=0, layer_index=-1, page_table_layer_index=-1, sampler_source=0, virtual_texture=False)
SAMPLED_IMAGE = dict(sampled_type=dict(kind='float', bits=32), dimension=1, depth=2,
                     arrayed=0, multisampled=0, sampled=1, format=0)
NO_TEXTURE = {'MaterialExpressionScalarParameter', 'MaterialExpressionVectorParameter'}
CONTEXT_RECEIPT = ROOT / 'original-texture-context-evidence-v3.json'
CONTEXT_SHA256 = '5617b511c2abe2553c8186182eec24f548fdff4872fbd366bea70f8c56f8cb08'
DIMENSION_RECEIPT = ROOT / 'original-texture-context-evidence-v5.json'
DIMENSION_SHA256 = 'c70a12b30eea35dfbeae40679050a89231fe520c3a367c79269d808f52aede44'
INSPECTION_TOOL = GENERATED / 'frontend-probes/cue4inspect-donut-textures/Cue4Inspect.dll'
INSPECTION_SHA256 = 'bd1e145743c2c93832ddf69e3965bace3dfda3b4ac6f5292cb409e1bbdfda676'


def digest(raw):
    return hashlib.sha256(raw).hexdigest()


def read_json(path):
    raw = path.read_bytes()
    return json.loads(raw[raw.index(b'{'):]), raw


def receipt(path):
    raw = path.read_bytes()
    return dict(path=str(path.resolve()), bytes=len(raw), sha256=digest(raw))


def unique_rows(rows, key):
    result = {row[key]: row for row in rows}
    if len(result) != len(rows):
        raise ValueError('duplicate ' + key)
    return result


def index_reference(reference):
    path, separator, index = reference['ObjectPath'].rpartition('.')
    if not separator or not index.isdecimal():
        raise ValueError('expected original export-index reference')
    return path, int(index)


def prove_context(owner, export_map, reports):
    """Only known false-reference classes and call-only function closures are accepted."""
    if owner['path'] != PACKAGE or export_map['package'] != PACKAGE:
        raise ValueError('wrong bounded owner')
    exports = unique_rows(export_map['exports'], 'index')
    expressions = unique_rows(owner['expressionExports'], 'name')
    functions = unique_rows(reports, 'path')
    visited, trace = set(), []

    def root(path):
        if path not in functions:
            raise ValueError('missing original function dependency')
        report = functions[path]
        if report['schema'] != 'fortnite.original-texture-function-package.v1' or report['skippedMetadata']:
            raise ValueError('unsupported function report')
        roots = [(i, x) for i, x in enumerate(report['objects']) if x['Type'] == 'MaterialFunction']
        if len(roots) != 1:
            raise ValueError('ambiguous function root')
        return roots[0]

    def visit(path, ancestors, result):
        if path in ancestors:
            raise ValueError('recursive function closure')
        visited.add(path)
        _, function = root(path)
        for reference in function['Properties']['FunctionExpressions']:
            if reference is None:
                continue
            package, index = index_reference(reference)
            if package != path:
                raise ValueError('external function expression')
            expression = functions[path]['objects'][index]
            if expression['Type'] != 'MaterialExpressionMaterialFunctionCall':
                raise ValueError('unproven texture-bearing function expression')
            expected_name = "MaterialExpressionMaterialFunctionCall'" + function['Name'] + ':' + expression['Name'] + "'"
            if reference['ObjectName'] != expected_name:
                raise ValueError('function expression index/name mismatch')
            target = expression['Properties'].get('MaterialFunction')
            if target is None:
                continue
            child, child_index = index_reference(target)
            root_index, child_root = root(child)
            if child_index != root_index or target['ObjectName'] != "MaterialFunction'" + child_root['Name'] + "'":
                raise ValueError('function root index/name mismatch')
            visit(child, ancestors | {path}, result)
            if child not in result:
                result.append(child)

    ordered = []
    for ordinal, reference in enumerate(owner['materialProperties'][0]['expressionRefs']):
        if reference['index'] == 0:
            continue
        if reference['isImport'] or exports[reference['index']]['name'] != reference['name']:
            raise ValueError('parent expression index/name mismatch')
        expression = expressions[reference['name']]
        kind = expression['exportType']
        if kind in NO_TEXTURE:
            continue
        props = unique_rows(expression['properties'], 'name')
        if kind == 'MaterialExpressionTextureSample':
            match = re.fullmatch(r"Texture2D'([^']+)' \(ObjectProperty\)", props['Texture']['value'])
            if not match or match[1] != TEXTURE_OBJECT:
                raise ValueError('unexpected original texture default')
            if match[1] not in ordered:
                ordered.append(match[1])
            trace.append(dict(expression_position=ordinal, texture_object=match[1], reference_index=ordered.index(match[1])))
        elif kind == 'MaterialExpressionMaterialFunctionCall':
            match = re.fullmatch(r"MaterialFunction'([^']+)\.([^'.]+)' \(ObjectProperty\)", props['MaterialFunction']['value'])
            if not match or root(match[1])[1]['Name'] != match[2]:
                raise ValueError('unexpected parent function target')
            dependency_order = []
            visit(match[1], set(), dependency_order)
            if match[1] not in dependency_order:
                dependency_order.append(match[1])
            trace.append(dict(expression_position=ordinal, function_order=dependency_order, texture_appends=[]))
        else:
            raise ValueError('unproven parent expression class: ' + kind)
    if ordered != [TEXTURE_OBJECT] or visited != set(functions):
        raise ValueError('incomplete or unused bounded function closure')
    return dict(algorithm='Original UMaterial PostLoad 144314eb0 -> 14430b550; function postorder/AddUnique list; function +298 appends',
                ordered_texture_references=[TEXTURE_PATH], trace=trace)


def validate_arrays(parsed):
    arrays = parsed['uniform_expression_arrays']
    if (arrays['texture_2d']['expressions'] != [dict(offset=879, type='FMaterialUniformExpressionTexture', texture=BASE_TEXTURE, end=903)]
            or arrays['texture_2d']['count'] != 1
            or any(v['count'] for k, v in arrays.items() if k.startswith('texture_') and k != 'texture_2d')
            or parsed['virtual_texture_stacks']['count'] or parsed['parameter_collections']['count']):
        raise ValueError('unexpected regular Texture contract')
    nodes, roots = bundle.fixture_exporter.flatten_expressions(arrays)
    if len(roots['vector_roots']) != 6 or len(roots['scalar_roots']) != 10:
        raise ValueError('unexpected original numeric root counts')
    records = parsed['material_shaders']['records']
    if len(records) != 7 or any(r['debug_uniform_buffer_layout'] != LAYOUT for r in records):
        raise ValueError('unexpected original typed uniform layout')
    return nodes, roots


def validate_texture(meta, source, header_bytes):
    """Read the exact bounded one-inline-mip platform frame; never resize pixels."""
    if meta['package'] not in (numeric.package_path(TEXTURE_PACKAGE), TEXTURE_PACKAGE) or meta['name'] != 'T_Buttonborder_DF':
        raise ValueError('wrong texture identity')
    if (meta['width'], meta['height'], meta['pixelFormat'], meta['decodedPixelFormat'], meta['firstMip']) != (64, 64, 'PF_B8G8R8A8', 'PF_B8G8R8A8', 0):
        raise ValueError('unexpected texture dimensions/format/mip')
    if meta['filter'] != 'TF_Default' or meta['lodGroup'] != 'TEXTUREGROUP_UI' or meta['srgb'] is not True or meta['addressX'] != 'TA_Wrap' or meta['addressY'] != 'TA_Wrap':
        raise ValueError('unexpected source texture sampling settings')
    marker = b'PF_B8G8R8A8\0'
    if source.count(marker) != 1:
        raise ValueError('ambiguous native texture platform frame')
    position = source.index(marker)
    if struct.unpack_from('<IIIi', source, position - 16) != (64, 64, 1, len(marker)):
        raise ValueError('native logical platform dimensions differ')
    cursor = position + len(marker)
    first, count, cooked, flags, elements, size, offset = struct.unpack_from('<iiiiiiq', source, cursor)
    cursor += 32
    if (first, count, cooked, flags, elements, size, offset) != (0, 1, 1, 0x48, 16384, 16384, header_bytes + cursor):
        raise ValueError('unsupported native inline mip framing')
    pixels = bytearray(source[cursor:cursor + size])
    cursor += size
    if len(pixels) != size or struct.unpack_from('<iiii', source, cursor) != (64, 64, 1, 0):
        raise ValueError('native mip dimensions or virtual flag differ')
    for index in range(0, len(pixels), 4):
        pixels[index], pixels[index + 2] = pixels[index + 2], pixels[index]
    rgba = Path(meta['rgba']).read_bytes()
    if bytes(pixels) != rgba or digest(rgba) != meta['rgbaSha256'] or digest(Path(meta['png']).read_bytes()) != meta['pngSha256']:
        raise ValueError('decoded pixels differ from exact source inline mip')
    if len(meta['mips']) != 1 or any(meta['mips'][0][k] != v for k, v in dict(width=64, height=64, depth=1, bulkBytes=16384).items()):
        raise ValueError('texture mip report differs from native frame')
    return dict(source_class='Texture2D', virtual_texture=False, logical_width=64, logical_height=64,
                pixel_format='PF_B8G8R8A8', canonical_decoded_format='RGBA8', first_mip=0, mip_count=1,
                native_platform_header_offset=position - 16, inline_pixels_offset=cursor - size,
                stored_bulk_offset=offset, reported_bulk_offset=meta['mips'][0]['bulkOffset'],
                rgba_sha256=digest(rgba), scope='Exact original one-inline-mip source decode; no resource creation or draw proof')


def typed_contract(record, reflected):
    roles, resources = [], []
    vertex = record['frequency'] == 0
    if (record['frequency'] not in (0, 3)
            or record['material_uniform_buffer'] != dict(base_index=0 if vertex else 1, bound=not vertex)
            or any(p['binding']['bound'] for p in record['uniform_parameters'])):
        raise ValueError('unexpected original uniform-buffer roles')
    for desc in reflected['descriptors']:
        reg, entries = desc['d3d_register'], desc['resource_table_entries']
        if reg['space'] or desc['set']:
            raise ValueError('unsupported descriptor space/set')
        if entries and reg['kind'] not in ('srv', 'sampler'):
            raise ValueError('unsupported original resource-table register kind')
        if reg['kind'] == 'srv' and desc.get('image_type') != SAMPLED_IMAGE:
            raise ValueError('unsupported sampled-image shape')
        if entries:
            if len(entries) != 1 or vertex:
                raise ValueError('ambiguous original material resource entry')
            entry = entries[0]
            kind = reg['kind']
            resource, table, dtype = (0, 'texture', 2) if kind == 'srv' else (1, 'sampler', 0)
            if (entry != dict(uniform_buffer=record['material_uniform_buffer']['base_index'], resource=resource, bind=reg['index'], table=table)
                    or desc['vulkan_descriptor_type'] != dtype or LAYOUT['resource_types'][resource] != (5 if kind == 'srv' else 8)):
                raise ValueError('original resource-table entry differs from typed material layout')
            role = 'MaterialTexture' if kind == 'srv' else 'MaterialTextureSampler'
            resources.append(dict(resource_index=resource, byte_offset=LAYOUT['resource_offsets'][resource], d3d_register=reg, descriptor_binding=desc['binding']))
        elif reg['kind'] == 'cbv' and desc['vulkan_descriptor_type'] == 6:
            if reg['index'] == 0 and desc['block_span_bytes'] == (80 if vertex else 64):
                role = 'SlateLooseConstants'
            elif not vertex and record['material_uniform_buffer'] == dict(base_index=reg['index'], bound=True) and reg['index'] == 1 and desc['block_span_bytes'] == 144:
                role = 'Material'
            else:
                raise ValueError('unclassified constant-buffer descriptor')
        elif not vertex and reg['index'] == 1 and reg['kind'] in ('srv', 'sampler') and desc['vulkan_descriptor_type'] == (2 if reg['kind'] == 'srv' else 0):
            role = 'ElementTexture' if reg['kind'] == 'srv' else 'ElementTextureSampler'
        else:
            raise ValueError('unclassified texture/sampler descriptor')
        roles.append(role)
    expected = {'SlateLooseConstants'} if vertex else {'SlateLooseConstants', 'Material', 'MaterialTexture', 'MaterialTextureSampler'}
    if 'ElementTexture' in roles or 'ElementTextureSampler' in roles:
        expected |= {'ElementTexture', 'ElementTextureSampler'}
    if len(roles) != len(set(roles)) or set(roles) != expected:
        raise ValueError('incomplete or duplicate descriptor roles')
    return dict(roles=roles, material_resources=resources)


def uniform_cases(parsed, nodes):
    parameters = {}
    for node in nodes:
        if node['type'] in ('ScalarParameter', 'VectorParameter'):
            key = node['parameter']['name']['name']
            value = dict(type='scalar' if node['type'] == 'ScalarParameter' else 'vector', parameter=node['parameter'])
            if key in parameters and parameters[key] != value:
                raise ValueError('ambiguous numeric parameter identity')
            parameters[key] = value
    values = [('BorderColor', [0.9, 0.1, 0.2, 1.0]), ('InteriorColor', [0.1, 0.8, 0.2, 0.7]),
              ('BorderThreshold', 0.4), ('BorderSize', 0.2), ('Scale', 0.7)]
    cases, combined = [], []
    def append(name, entries, unmatched=False):
        raw = evaluator.evaluate_report(parsed, evaluator.overrides_from_json([] if unmatched else entries))[0]
        cases.append(dict(name=name, overrides=entries, numeric_values=list(struct.unpack('<' + 'f' * (len(raw) // 4), raw)),
                          numeric_sha256=digest(raw), abs_tolerance=0.00001, rel_tolerance=0.00001))
    append('defaults', [])
    for key, value in values:
        entry = dict(copy.deepcopy(parameters[key]), value=value)
        combined.append(entry)
        append(key, [entry])
    append('combined', combined)
    for field, value in [('association', 0), ('index', 7), ('number', 1)]:
        entry = copy.deepcopy(combined[0])
        if field == 'number':
            entry['parameter']['name']['number'] = value
        else:
            entry['parameter'][field] = value
        append('unmatched_' + field, [entry], unmatched=True)
    return cases


def create_bundle(inventory, functions, texture_report, shader_directory):
    stem = PACKAGE.replace('/', '_')
    paths = [inventory / (stem + suffix) for suffix in ('.json', '.export-map.json', '.inline-resource.json', '.decoded.json')]
    owner, export_map, framing, decoded = [read_json(p)[0] for p in paths]
    expected_owner = dict(name='M_Buttonborder_DF', type='UMaterial', blendMode='BLEND_Translucent',
        shadingModel='MSM_Unlit', twoSided=False, expressionCount=16, referencedTextures=[], parent=None, staticPermutation=None)
    if (len(owner['materials']) != 1 or any(owner['materials'][0].get(k) != v for k, v in expected_owner.items())
            or owner['materialProperties'][0]['name'] != 'M_Buttonborder_DF'
            or unique_rows(owner['materialProperties'][0]['properties'], 'name')['MaterialDomain']['value'] != 'MD_UI (EnumProperty)'):
        raise ValueError('bounded original material identity/domain changed')
    if digest(CONTEXT_RECEIPT.read_bytes()) != CONTEXT_SHA256:
        raise ValueError('original reference-context proof changed')
    if digest(DIMENSION_RECEIPT.read_bytes()) != DIMENSION_SHA256:
        raise ValueError('original texture dimension proof changed')
    if digest(INSPECTION_TOOL.read_bytes()) != INSPECTION_SHA256:
        raise ValueError('isolated source inspection tool changed')
    source_paths = paths + [CONTEXT_RECEIPT, DIMENSION_RECEIPT, INSPECTION_TOOL]
    function_reports = []
    for path in sorted(functions.glob('*.json')):
        report, _ = read_json(path)
        if len(report['inputs']) != 2:
            raise ValueError('incomplete function package source receipts')
        for item in report['inputs']:
            source = Path(item['path'])
            if digest(source.read_bytes()) != item['sha256']:
                raise ValueError('function package source changed')
            source_paths.append(source)
        source_paths.append(path)
        function_reports.append(report)
    context = prove_context(owner, export_map, function_reports)
    source = EXTRACTED / (PACKAGE + '.uexp')
    raw = source.read_bytes()
    if export_map['headerBytes'] != source.with_suffix('.uasset').stat().st_size:
        raise ValueError('original package header/export map changed')
    if (framing['package'] != PACKAGE or framing['export_name'] != 'M_Buttonborder_DF'
            or framing['export_map_sha256'] != digest(paths[1].read_bytes())
            or decoded['package'] != PACKAGE):
        raise ValueError('material framing/source identity changed')
    payload = bundle.validate_source(raw, framing)
    parsed = bundle.decoder.parse_prefix(payload, framing['names'])
    archive_path = Path(decoded['shader_library']['path'])
    archive_hash = bundle.library_reader.sha256(archive_path)
    if archive_hash != decoded['shader_library']['sha256']:
        raise ValueError('original shader library changed')
    original_code = []
    with bundle.library_reader.ShaderArchive(archive_path, layout='fortnite-12.60') as archive:
        bundle.decoder.associate_library(parsed, archive)
        for record in bundle.all_records(parsed):
            index = record['library_association']['index']
            code = archive.shader_code(index)
            original_code.append(dict(type=record['type'], output_hash=record['output_hash']['hash'],
                library_index=index, frequency=record['frequency'], bytes=len(code), sha256=digest(code)))
    for key, value in parsed.items():
        if key == 'original_serializer_evidence':
            if any(decoded[key].get(k) != v for k, v in value.items()):
                raise ValueError('original serializer evidence changed')
        elif decoded.get(key) != value:
            raise ValueError('original complete parse changed: ' + key)
    if (not parsed['complete_serialized_shader_map'] or parsed['undecoded_bytes'] or parsed['shader_record_count'] != 19
            or len(original_code) != 19 or parsed['material_path'] != OWNER):
        raise ValueError('incomplete original source resource')
    nodes, roots = validate_arrays(parsed)
    meta, _ = read_json(texture_report)
    texture_source = EXTRACTED / (TEXTURE_PACKAGE + '.uexp')
    texture_asset = texture_source.with_suffix('.uasset')
    native_texture = validate_texture(meta, texture_source.read_bytes(), texture_asset.stat().st_size)
    source_paths += [source, source.with_suffix('.uasset'), texture_report, texture_source, texture_asset, Path(meta['rgba']), Path(meta['png'])]

    shader_paths = [shader_directory / 'archive.json', shader_directory / 'd3d.json',
                    shader_directory / 'mapped/mapped-translation-report.json', shader_directory / 'reflection.json']
    source_report, d3d, mapped, reflected = [read_json(p)[0] for p in shader_paths]
    if (source_report['sha256'] != archive_hash or d3d['archive_sha256'] != archive_hash or mapped['archive_sha256'] != archive_hash
            or source_report['selection_truncated'] or mapped['truncated']
            or d3d['source_report_sha256'] != digest(shader_paths[0].read_bytes())
            or mapped['source_report_sha256'] != digest(shader_paths[0].read_bytes())
            or mapped['d3d_report_sha256'] != digest(shader_paths[1].read_bytes())
            or reflected['mapped_report_sha256'] != digest(shader_paths[2].read_bytes())):
        raise ValueError('mapped shader report provenance changed')
    tables = [unique_rows(doc[key], identity) for doc, key, identity in
              [(source_report, 'shaders', 'index'), (d3d, 'records', 'shader_index'),
               (mapped, 'records', 'shader_index'), (reflected, 'records', 'shader_index')]]
    expected_indices = {r['library_association']['index'] for r in parsed['material_shaders']['records']}
    if len(expected_indices) != 7 or any(set(table) != expected_indices for table in tables):
        raise ValueError('incomplete/extra original Slate shader selection')
    validators = [Path(p) for p in mapped['tool_sha256'] if Path(p).name == 'spirv-val']
    if len(validators) != 1:
        raise ValueError('missing original SPIR-V validator identity')
    validator = validators[0]
    for path, expected in mapped['tool_sha256'].items():
        if digest(Path(path).read_bytes()) != expected:
            raise ValueError('mapped compilation tool changed')
    shaders, contracts = [], []
    for record in parsed['material_shaders']['records']:
        index = record['library_association']['index']
        original, direct, translated, reflection = [table[index] for table in tables]
        wrapper = Path(original['wrapper_path']).read_bytes()
        containers = bundle.library_reader.dxbc_containers(wrapper)
        if (digest(wrapper) != original['decoded_sha256'] or len(containers) != 1
                or {k: v for k, v in direct.items() if k not in ('shader_index', 'frequency', 'wrapper_sha256')} != d3d_tool.inspect_wrapper(wrapper, containers)
                or direct['wrapper_sha256'] != digest(wrapper)):
            raise ValueError('original D3D wrapper/source resource table changed')
        with bundle.library_reader.ShaderArchive(archive_path, layout='fortnite-12.60') as archive:
            if archive.shader_code(index) != wrapper:
                raise ValueError('D3D wrapper differs from actual original archive')
        dxbc_path = Path(translated['input'])
        dxbc = dxbc_path.read_bytes()
        container = containers[0]
        if (dxbc != wrapper[container['offset']:container['offset'] + container['size']]
                or digest(dxbc) != translated['input_sha256'] or translated['wrapper_sha256'] != digest(wrapper)):
            raise ValueError('translated DXBC differs from exact original wrapper slice')
        code_path = Path(reflection['output'])
        code = code_path.read_bytes()
        if (GENERATED.resolve() not in code_path.resolve().parents or not translated['validated']
                or translated['unmatched_resource_table_entries'] or translated['compiler_diagnostics']
                or digest(code) != translated['output_sha256'] or digest(code) != reflection['spirv_sha256']
                or translated['frequency'] != record['frequency'] or reflection['frequency'] != record['frequency']):
            raise ValueError('mapped original shader bytes/stage changed')
        validation = subprocess.run([str(validator), '--target-env', 'vulkan1.0', str(code_path)], capture_output=True, text=True, timeout=30)
        if validation.returncode or validation.stdout or validation.stderr:
            raise ValueError('original mapped SPIR-V validation failed')
        actual = reflection_tool.Module(code).reflect()
        if actual['entry_point'] != reflection['entry_point'] or actual['interfaces'] != reflection['interfaces'] or len(actual['descriptors']) != len(reflection['descriptors']):
            raise ValueError('fresh SPIR-V reflection differs')
        for observed, declared in zip(actual['descriptors'], reflection['descriptors']):
            if any(declared.get(k) != v for k, v in observed.items()):
                raise ValueError('fresh descriptor reflection differs')
        contract = typed_contract(record, reflection)
        contracts.append(dict(shader_type=record['type'], library_index=index, **contract))
        shaders.append(dict(record=record, mapped=translated, reflection=reflection, spirv_sha1=hashlib.sha1(code).hexdigest()))
        source_paths += [Path(original['wrapper_path']), dxbc_path, code_path]
    source_paths += shader_paths
    cases = uniform_cases(parsed, nodes)
    expression = dict(expression_kind='Texture', parameter=None, texture_index=0, texture_layer_index=-1,
                      page_table_layer_index=-1, sampler_source=0, virtual_texture=False, default_texture_path=TEXTURE_PATH)
    fixture = dict(schema='fortnite.legacy-slate.v4', material={k: parsed[k] for k in ('friendly_name', 'material_path',
        'quality_level', 'feature_level', 'cooked_shader_map_id_hash', 'debug_description')}, expected_layout=copy.deepcopy(LAYOUT),
        nodes=nodes, **roots, shaders=shaders, numeric_comparison_bytes=136, uniform_cases=cases, owner_uniform_cases=copy.deepcopy(cases),
        owner_parameter_expectations=[], draw_parameter='BorderColor', texture_2d_parameters=[expression],
        ordered_texture_references=context['ordered_texture_references'], texture_context_proof=context,
        texture_contract=dict(expression_kind='Texture', parameter=None, default_texture_path=TEXTURE_PATH,
            default=meta, source_platform_data=native_texture, named_overrides_allowed=False,
            scope='Original regular Texture uses indexed default only; parameter identity is genuinely absent'),
        diagnostic_sampling_control=dict(resource_index=0, resource_byte_offset=144, source_rgba=meta['rgba'],
            source_rgba_sha256=meta['rgbaSha256'], width=64, height=64, upload_format='PF_B8G8R8A8', srgb=True,
            upload_conversion='RGBA8 source bytes to BGRA8; no resize', mip_count=1,
            primary_numeric_profile='defaults', secondary_numeric_profile='combined',
            owner_cache_mutation_allowed=False, named_texture_overrides_allowed=False,
            requirements=['actual owner source vs independent uploaded source copy: identical readbacks',
                          'actual owner source vs asymmetric checker scratch resource: different readbacks',
                          'same source shaders, layout, numeric profile and sampler resources for every control'],
            scope='Future diagnostic scratch-buffer draws only; no runtime outcome is asserted'),
        shader_binding_contracts=contracts, expected_uniform_layout_hash=verifier.uniform_layout_hash(**dict(size=176, offsets=LAYOUT['resource_offsets'], types=LAYOUT['resource_types'])),
        provenance=dict(resource_sha256=parsed['resource_sha256'], shader_library_sha256=archive_hash))
    return dict(schema='fortnite.legacy-material-ingress.v4', owner_path=OWNER, owner_type='UMaterial', parent_path='',
        shader_identity=OWNER, resource_owner_path=OWNER,
        framing=framing, decoded=decoded, runtime_fixture=fixture, resource_raw_hex=payload.hex(), original_shader_code=original_code,
        provenance=dict(inputs=[receipt(p) for p in dict.fromkeys(source_paths)], resource_sha1=hashlib.sha1(payload).hexdigest(),
            shader_library=dict(path=str(archive_path), sha256=archive_hash), exporter_sha256=digest(Path(__file__).read_bytes()),
            tools=[receipt(HERE / name) for name in ('export_ingress_bundle.py', 'legacy_shader_map.py', 'legacy_shader_records.py',
                'evaluate_uniforms.py', 'export_runtime_fixture.py', 'reflect_mapped_spirv.py', 'inspect_d3d_code.py', 'verify_runtime_artifacts.py')]),
        limitations=['Bounded actual original parent only; all 19 records retained and seven Slate programs translated',
                    'Regular Texture has no ParameterInfo; named overrides are forbidden',
                    'Source pixels/descriptor contracts are offline evidence; actual owner GT/RT attachment, RHI draw and readbacks remain unverified',
                    'No texture resizing, substitution, TextureProperty, virtual texture or generic texture graph support'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--inventory', type=Path, default=ROOT / 'keybind-inventory-2')
    parser.add_argument('--function-reports', type=Path, default=ROOT / 'keybind-function-properties-1')
    parser.add_argument('--texture-report', type=Path, default=ROOT / 'keybind-texture-1/texture-report.json')
    parser.add_argument('--shader-directory', type=Path, default=ROOT / 'keybind-shaders-1')
    parser.add_argument('--output', type=Path, default=ROOT / 'keybind-ingress-3/bundle.json')
    args = parser.parse_args()
    try:
        fixture_path = args.output.with_name('runtime-fixture.json')
        if GENERATED.resolve() not in args.output.resolve().parents or args.output.exists() or fixture_path.exists() or fixture_path == args.output:
            raise ValueError('choose fresh generated bundle and fixture outputs')
        data = create_bundle(args.inventory, args.function_reports, args.texture_report, args.shader_directory)
        raw = (json.dumps(data, indent=2, allow_nan=False) + '\n').encode()
        fixture = (json.dumps(data['runtime_fixture'], indent=2, allow_nan=False) + '\n').encode()
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_bytes(raw); fixture_path.write_bytes(fixture)
        print(json.dumps(dict(path=str(args.output.resolve()), bundle_sha1=hashlib.sha1(raw).hexdigest(), bundle_sha256=digest(raw),
            fixture=str(fixture_path.resolve()), fixture_sha256=digest(fixture), resource_sha1=data['provenance']['resource_sha1'],
            original_records=len(data['original_shader_code']), translated_slate_records=len(data['runtime_fixture']['shaders']))))
    except (OSError, ValueError, KeyError, TypeError, IndexError, StopIteration, subprocess.TimeoutExpired) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
