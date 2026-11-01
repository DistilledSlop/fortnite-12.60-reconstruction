#!/usr/bin/env python3
"""Bounded original static-permutation M_UI_AthenaSeason_FadedFlogo named-texture ingress.

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
import tempfile

HERE = Path(__file__).resolve().parent


def sibling(name):
    spec = importlib.util.spec_from_file_location(name, HERE / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


numeric = sibling('export_numeric_ingress')
mainl = sibling('export_mainl_ingress')
bundle, evaluator = numeric.bundle, numeric.evaluator
reflection_tool = sibling('reflect_mapped_spirv')
d3d_tool = sibling('inspect_d3d_code')
verifier = sibling('verify_runtime_artifacts')
GENERATED = HERE.parent / 'generated'
ROOT = GENERATED / 'material_pipeline'
EXTRACTED = Path('/mnt/vibrato/fortnite-extracted-full')
PACKAGE = 'FortniteGame/Content/Athena/UI/Season/Art/M_UI_AthenaSeason_FadedFlogo'
PARENT_PACKAGE = 'FortniteGame/Content/Athena/UI/Season/Art/M_UI_AthenaSeason_Stylized_Font'
PARENT = '/Game/Athena/UI/Season/Art/M_UI_AthenaSeason_Stylized_Font.M_UI_AthenaSeason_Stylized_Font'
OWNER = '/Game/Athena/UI/Season/Art/M_UI_AthenaSeason_FadedFlogo.M_UI_AthenaSeason_FadedFlogo'
TEXTURE_PACKAGE = 'FortniteGame/Content/UI/Foundation/Textures/Logo/T-Logo-Wordmark-1024x256'
TEXTURE_OBJECT = TEXTURE_PACKAGE + '.T-Logo-Wordmark-1024x256'
TEXTURE_PATH = numeric.package_path(TEXTURE_PACKAGE) + '.T-Logo-Wordmark-1024x256'
LAYOUT = dict(constant_bytes=208, resource_offsets=[176, 184, 192, 200], resource_types=[5, 8, 8, 8])
BASE_TEXTURE = dict(texture_index=0, layer_index=-1, page_table_layer_index=-1, sampler_source=0, virtual_texture=False)
SAMPLED_IMAGE = dict(sampled_type=dict(kind='float', bits=32), dimension=1, depth=2,
                     arrayed=0, multisampled=0, sampled=1, format=0)
NO_TEXTURE = {'MaterialExpressionScalarParameter', 'MaterialExpressionVectorParameter', 'MaterialExpressionStaticSwitchParameter'}
CONTEXT_RECEIPT = ROOT / 'original-texture-context-evidence-v3.json'
CONTEXT_SHA256 = '5617b511c2abe2553c8186182eec24f548fdff4872fbd366bea70f8c56f8cb08'
DIMENSION_RECEIPT = ROOT / 'original-texture-context-evidence-v5.json'
DIMENSION_SHA256 = 'c70a12b30eea35dfbeae40679050a89231fe520c3a367c79269d808f52aede44'
INSPECTION_TOOL = GENERATED / 'frontend-probes/cue4inspect-donut-textures/Cue4Inspect.dll'
INSPECTION_SHA256 = 'bd1e145743c2c93832ddf69e3965bace3dfda3b4ac6f5292cb409e1bbdfda676'
FUNCTION_TOOL = HERE / 'original_texture_function_probe/bin/Release/net10.0/OriginalTextureFunctionProbe.dll'
FUNCTION_PATH = 'Engine/Content/Functions/Engine_MaterialFunctions01/Gradient/RadialGradientExponential'
DENSITY_PATH = 'Engine/Content/Functions/Engine_MaterialFunctions01/Density/ExponentialDensity'
RESOURCE_SHA256 = '2386c2a88b189a01950322933e0cfc8666201bc953a77159a66127273d962a63'
SOURCE_HASHES = {
    PACKAGE + '.uasset': '6bed29337d67ba42d35058e7967e455b72a03faa5eb6de0788430faffa91e9dd',
    PACKAGE + '.uexp': 'efa6ff991e40ab7dbe03cceeb3049a47d86ea165ead42bf5b148d0e69eef92fb',
    PARENT_PACKAGE + '.uasset': '0ced3756ad089110a5c6c54599d952e9fdad7fbd57da56076dc8aa4d88d1c6da',
    PARENT_PACKAGE + '.uexp': '66d060895abc54ed2611a79cd419e05f40b7e2bd61c1c4060bfe2acef2b92e2c',
    TEXTURE_PACKAGE + '.uasset': '407da51def5b04af628df90f7e301476976344644bf6edba2cf2e37edc5ac293',
    TEXTURE_PACKAGE + '.uexp': '93d7fb119a7a9f153c3410c23f761ab51335c8385bdf716a5e6fcec43a8b8a17',
    FUNCTION_PATH + '.uasset': '08920f5adb3af5c824b50288686abfe8e14e3f371f6b1736a6ece9d5184cbfbe',
    FUNCTION_PATH + '.uexp': 'c82aa89b95c20c13e1e64fd78e7ce7c08dfda40018a1aacec294767c84f29d55',
    DENSITY_PATH + '.uasset': '0094087e9eb03483fd29d0e3c93df6e79d0ea83f921f1969e418e82778589a56',
    DENSITY_PATH + '.uexp': '4d0266b5d4a9e9f3b6e4deb5d286d42aaf1baf265b6b77d7bf6d3a91ed005fb7',
}
FUNCTION_TOOL_SHA256 = '91759f4058c48d5c84d5c9fc25655c55a66807ab57ac3d763e79d9684d14de62'


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
    if owner['path'] != PARENT_PACKAGE or export_map['package'] != PARENT_PACKAGE:
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
        if kind == 'MaterialExpressionTextureSampleParameter2D':
            match = re.fullmatch(r"Texture2D'([^']+)' \(ObjectProperty\)", props['Texture']['value'])
            if props.get('ParameterName', {}).get('value') != 'TextureMask (NameProperty)' or not match or match[1] != TEXTURE_OBJECT:
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
    if (ordered != [TEXTURE_OBJECT] or visited != {FUNCTION_PATH, DENSITY_PATH} or set(functions) != {FUNCTION_PATH, DENSITY_PATH}
            or sum('texture_object' in x for x in trace) != 1
            or sum('function_order' in x for x in trace) != 1):
        raise ValueError('incomplete or unused bounded function closure')
    return dict(algorithm='Original UMaterial PostLoad 144314eb0 -> 14430b550; function postorder/AddUnique list; function +298 appends',
                ordered_texture_references=[TEXTURE_PATH], trace=trace)


def validate_arrays(parsed):
    arrays = parsed['uniform_expression_arrays']
    if (arrays['texture_2d']['expressions'] != [dict(offset=881, type='FMaterialUniformExpressionTextureParameter', parameter=dict(name=dict(name='TextureMask',number=0),association=2,index=-1), texture=BASE_TEXTURE, end=918)]
            or arrays['texture_2d']['count'] != 1
            or any(v['count'] for k, v in arrays.items() if k.startswith('texture_') and k != 'texture_2d')
            or parsed['virtual_texture_stacks']['count'] or parsed['parameter_collections']['count']):
        raise ValueError('unexpected named TextureParameter contract')
    nodes, roots = bundle.fixture_exporter.flatten_expressions(arrays)
    if len(roots['vector_roots']) != 7 or len(roots['scalar_roots']) != 14:
        raise ValueError('unexpected original numeric root counts')
    records = parsed['material_shaders']['records']
    if len(records) != 7 or any(r['debug_uniform_buffer_layout'] != LAYOUT for r in records):
        raise ValueError('unexpected original typed uniform layout')
    return nodes, roots


def validate_texture(meta, source, header_bytes):
    """Validate original one-mip DXT5 framing; compare replayed decoded pixels separately."""
    if meta['package'] not in (numeric.package_path(TEXTURE_PACKAGE), TEXTURE_PACKAGE) or meta['name'] != 'T-Logo-Wordmark-1024x256':
        raise ValueError('wrong texture identity')
    if (meta['width'], meta['height'], meta['pixelFormat'], meta['decodedPixelFormat'], meta['firstMip']) != (1024, 256, 'PF_DXT5', 'PF_R8G8B8A8', 0):
        raise ValueError('unexpected texture dimensions/format/mip')
    if meta['filter'] != 'TF_Default' or meta['lodGroup'] != 'TEXTUREGROUP_UI' or meta['srgb'] is not True or meta['addressX'] != 'TA_Clamp' or meta['addressY'] != 'TA_Clamp':
        raise ValueError('unexpected source texture sampling settings')
    marker=b'PF_DXT5\0'
    if source.count(marker)!=1: raise ValueError('ambiguous native texture platform frame')
    position=source.index(marker)
    if struct.unpack_from('<IIIi',source,position-16)!=(1024,256,1,len(marker)): raise ValueError('native platform dimensions differ')
    cursor=position+len(marker)
    first,count,cooked,flags,elements,size,offset=struct.unpack_from('<iiiiiiq',source,cursor);cursor+=32
    if (first,count,cooked,flags,elements,size,offset)!=(0,1,1,0x48,262144,262144,header_bytes+cursor): raise ValueError('unsupported DXT5 inline mip frame')
    blocks=source[cursor:cursor+size];cursor+=size
    if len(blocks)!=size or struct.unpack_from('<iiii',source,cursor)!=(1024,256,1,0): raise ValueError('DXT5 mip dimensions or virtual flag differ')
    if len(meta['mips'])!=1 or any(meta['mips'][0][k]!=v for k,v in dict(width=1024,height=256,depth=1,bulkBytes=262144).items()): raise ValueError('DXT5 mip report differs')
    rgba=Path(meta['rgba']).read_bytes()
    if len(rgba)!=1024*256*4 or digest(rgba)!=meta['rgbaSha256'] or digest(Path(meta['png']).read_bytes())!=meta['pngSha256']: raise ValueError('decoded DXT5 pixels differ')
    return dict(source_class='Texture2D',virtual_texture=False,logical_width=1024,logical_height=256,pixel_format='PF_DXT5',canonical_decoded_format='RGBA8',first_mip=0,mip_count=1,stored_bulk_offset=offset,compressed_bytes=size,compressed_sha256=digest(blocks),rgba_sha256=digest(rgba),scope='Exact original one-inline-mip DXT5 frame; decoder replay separately verified')


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
            elif not vertex and record['material_uniform_buffer'] == dict(base_index=reg['index'], bound=True) and reg['index'] == 1 and desc['block_span_bytes'] == 176:
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


def uniform_cases(parsed, nodes, inherited):
    parameters={}
    for node in nodes:
        if node['type'] in ('ScalarParameter','VectorParameter'):
            name=node['parameter']['name']['name']; kind='scalar' if node['type']=='ScalarParameter' else 'vector'
            entry=dict(type=kind,parameter=node['parameter'])
            if name in parameters and parameters[name]!=entry: raise ValueError('ambiguous numeric parameter')
            parameters[name]=entry
    changes=[('Color1',[0.9,0.1,0.2,1]),('Color2',[0.1,0.8,0.2,0.7]),('Density',2.0),('OverallAlpha',0.7)]
    cases=[]
    def append(name, values, unmatched=False):
        effective={evaluator.identity(x['parameter']):copy.deepcopy(x) for x in inherited}
        if not unmatched:effective.update({evaluator.identity(x['parameter']):copy.deepcopy(x) for x in values})
        raw=evaluator.evaluate_report(parsed,evaluator.overrides_from_json(list(effective.values())))[0]
        cases.append(dict(name=name,overrides=values,numeric_values=list(struct.unpack('<'+'f'*(len(raw)//4),raw)),numeric_sha256=digest(raw),abs_tolerance=0.00001,rel_tolerance=0.00001))
    append('defaults',[]); combined=[]
    for name,value in changes:
        if name not in parameters: raise ValueError('missing source numeric graph parameter')
        item=dict(copy.deepcopy(parameters[name]),value=value);combined.append(item);append(name,[item])
    append('combined',combined)
    for field,value in [('association',0),('index',7),('number',1)]:
        item=copy.deepcopy(combined[0]);(item['parameter']['name'] if field=='number' else item['parameter'])[field]=value
        append('unmatched_'+field,[item],True)
    return cases,parameters


def create_bundle(inventory, functions, texture_report, shader_directory):
    stem = PACKAGE.replace('/', '_')
    paths = [inventory / (stem + suffix) for suffix in ('.json', '.export-map.json', '.inline-resource.json', '.decoded.json')]
    owner, export_map, framing, decoded = [read_json(p)[0] for p in paths]
    expected_owner = dict(name='M_UI_AthenaSeason_FadedFlogo', type='UMaterialInstanceConstant', parent='M_UI_AthenaSeason_Stylized_Font', staticPermutation=True)
    if (len(owner['materials']) != 1 or any(owner['materials'][0].get(k) != v for k, v in expected_owner.items())
            or owner['instanceParameters'][0]['name'] != 'M_UI_AthenaSeason_FadedFlogo'):
        raise ValueError('bounded original material identity/domain changed')
    if digest(CONTEXT_RECEIPT.read_bytes()) != CONTEXT_SHA256:
        raise ValueError('original reference-context proof changed')
    if digest(DIMENSION_RECEIPT.read_bytes()) != DIMENSION_SHA256:
        raise ValueError('original texture dimension proof changed')
    if digest(INSPECTION_TOOL.read_bytes()) != INSPECTION_SHA256:
        raise ValueError('isolated source inspection tool changed')
    # Re-observe source properties from the exact original package. This avoids
    # treating a checked-in or previously generated JSON report as source.
    for relative, expected in SOURCE_HASHES.items():
        if digest((EXTRACTED / relative).read_bytes()) != expected:
            raise ValueError('original source package hash differs: ' + relative)
    if digest(FUNCTION_TOOL.read_bytes()) != FUNCTION_TOOL_SHA256:
        raise ValueError('original function inspection tool changed')
    fresh = subprocess.run(['dotnet', str(INSPECTION_TOOL), PACKAGE, str(EXTRACTED), '--loose', '--material-report'],
                           capture_output=True, text=True, timeout=180, check=True)
    if json.loads(fresh.stdout[fresh.stdout.index('{'):]) != owner:
        raise ValueError('fresh original material export properties differ')
    source_paths = paths + [CONTEXT_RECEIPT, DIMENSION_RECEIPT, INSPECTION_TOOL, FUNCTION_TOOL]
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
    with tempfile.TemporaryDirectory(prefix='flogo-function-replay-') as temporary:
        replay = Path(temporary) / 'output'
        subprocess.run(['dotnet', str(FUNCTION_TOOL), str(EXTRACTED), str(replay), FUNCTION_PATH, DENSITY_PATH],
                       capture_output=True, text=True, timeout=180, check=True)
        files = list(replay.glob('*.json'))
        if len(files) != 2 or {read_json(p)[0]['path']: read_json(p)[0] for p in files} != {r['path']: r for r in function_reports}:
            raise ValueError('fresh original function closure differs')
    parent_stem = PARENT_PACKAGE.replace('/', '_')
    parent_paths = [inventory / (parent_stem + suffix) for suffix in ('.json', '.export-map.json')]
    parent_owner, parent_exports = [read_json(p)[0] for p in parent_paths]
    fresh_parent = subprocess.run(['dotnet', str(INSPECTION_TOOL), PARENT_PACKAGE, str(EXTRACTED), '--loose', '--material-report'],
                                  capture_output=True, text=True, timeout=180, check=True)
    if json.loads(fresh_parent.stdout[fresh_parent.stdout.index('{'):]) != parent_owner:
        raise ValueError('fresh original parent properties differ')
    context = prove_context(parent_owner, parent_exports, function_reports)
    source_paths += parent_paths + [EXTRACTED / (PARENT_PACKAGE + '.uasset'), EXTRACTED / (PARENT_PACKAGE + '.uexp')]
    instance_path = ROOT / 'flogo-instance-full-1' / (PACKAGE.replace('/', '_') + '.json')
    instance_full, _ = read_json(instance_path)
    with tempfile.TemporaryDirectory(prefix='flogo-instance-replay-') as temporary:
        replay = Path(temporary) / 'output'
        subprocess.run(['dotnet', str(FUNCTION_TOOL), str(EXTRACTED), str(replay), '--instance', PACKAGE], capture_output=True, text=True, timeout=180, check=True)
        files = list(replay.glob('*.json'))
        if len(files)!=1 or read_json(files[0])[0]!=instance_full: raise ValueError('fresh original instance properties differ')
    props=instance_full['objects'][0]['Properties']
    expected_ref=dict(Name='TextureMask',Association='GlobalParameter',Index=-1)
    textures=props['TextureParameterValues']
    if (props['bHasStaticPermutationResource'] is not True or len(textures)!=1 or textures[0]['ParameterInfo']!=expected_ref
            or textures[0]['ParameterValue']['ObjectPath']!=TEXTURE_PACKAGE+'.0'
            or props['Parent']['ObjectPath']!=PARENT_PACKAGE+'.0'):
        raise ValueError('original instance parent/texture identity differs')
    source_paths.append(instance_path)
    # Cross-check every simplified numeric expectation with the independently
    # replayed complete original instance export and its full identity.
    simple = owner['instanceParameters'][0]
    for source_key, report_key, vector in [('ScalarParameterValues','scalars',False),('VectorParameterValues','vectors',True)]:
        values=props[source_key]; rows=simple[report_key]
        if len(values)!=len(rows): raise ValueError('original instance numeric parameter count differs')
        for value,row in zip(values,rows):
            source_info=value['ParameterInfo']; report_info=row['parameterInfo']
            if (source_info['Name']!=row['parameter'] or source_info['Name']!=report_info['name']['name'] or
                report_info['name']['number']!=0 or source_info['Association']!='GlobalParameter' or
                report_info['association']!=2 or source_info['Index']!=report_info['index']):
                raise ValueError('original instance numeric parameter identity differs')
            source_value=value['ParameterValue']; source_value=[source_value[k] for k in ('R','G','B','A')] if vector else source_value
            if source_value!=row['value']: raise ValueError('original instance numeric parameter value differs')

    source = EXTRACTED / (PACKAGE + '.uexp')
    raw = source.read_bytes()
    if export_map['headerBytes'] != source.with_suffix('.uasset').stat().st_size:
        raise ValueError('original package header/export map changed')
    if (framing['package'] != PACKAGE or framing['export_name'] != 'M_UI_AthenaSeason_FadedFlogo'
            or framing['export_map_sha256'] != digest(paths[1].read_bytes())
            or decoded['package'] != PACKAGE):
        raise ValueError('material framing/source identity changed')
    payload = bundle.validate_source(raw, framing)
    if digest(payload) != RESOURCE_SHA256 or len(payload) != 9187:
        raise ValueError('original material resource hash/length differs')
    parsed = bundle.decoder.parse_prefix(payload, framing['names'])
    archive_path = EXTRACTED / 'FortniteGame/Content/ShaderArchive-FortniteGame-PCD3D_SM5.ushaderbytecode'
    archive_hash = bundle.library_reader.sha256(archive_path)
    if archive_hash != '2e795bd21c5e9bd69a6fdb520ef73f6fd70822c011c4232ce1ba08ccf10aef21':
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
            or len(original_code) != 19 or parsed['material_path'] != PARENT):
        raise ValueError('incomplete original source resource')
    nodes, roots = validate_arrays(parsed)
    meta, _ = read_json(texture_report)
    with tempfile.TemporaryDirectory(prefix='flogo-texture-replay-', dir=ROOT) as temporary:
        replay = Path(temporary) / 'output'
        subprocess.run(['dotnet', str(INSPECTION_TOOL), TEXTURE_PACKAGE, str(EXTRACTED), '--loose',
                        '--texture-report', '--texture-output=' + str(replay)],
                       capture_output=True, text=True, timeout=180, check=True)
        actual, _ = read_json(replay / 'texture-report.json')
        if {k: v for k, v in actual.items() if k not in ('rgba', 'png')} != {k: v for k, v in meta.items() if k not in ('rgba', 'png')}:
            raise ValueError('fresh original texture metadata differs')
        if Path(actual['rgba']).read_bytes() != Path(meta['rgba']).read_bytes() or Path(actual['png']).read_bytes() != Path(meta['png']).read_bytes():
            raise ValueError('fresh original texture pixel decode differs')
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
    _, parameters = uniform_cases(parsed, nodes, [])
    instance = owner['instanceParameters'][0]
    original_textures = instance['textures']
    if len(original_textures)!=1 or original_textures[0].get('parameter')!='TextureMask' or original_textures[0].get('package')!='/Game/UI/Foundation/Textures/Logo/T-Logo-Wordmark-1024x256':
        raise ValueError('unexpected actual instance texture override')
    numeric_instance = dict(instance, textures=[])
    graph = {name: (item['type'], item['parameter']) for name,item in parameters.items()}
    expectations, graph_expectations, unused_expectations = mainl.partition_owner_expectations(numeric_instance, graph)
    if len(expectations)!=14: raise ValueError('unexpected original numeric override count')
    cases,_ = uniform_cases(parsed, nodes, [])
    owner_cases,_ = uniform_cases(parsed, nodes, graph_expectations)
    expression = dict(expression_kind='TextureParameter', parameter=dict(name=dict(name='TextureMask',number=0),association=2,index=-1), texture_index=0, texture_layer_index=-1,
                      page_table_layer_index=-1, sampler_source=0, virtual_texture=False, default_texture_path=TEXTURE_PATH)
    fixture = dict(schema='fortnite.legacy-slate.v4', material={k: parsed[k] for k in ('friendly_name', 'material_path',
        'quality_level', 'feature_level', 'cooked_shader_map_id_hash', 'debug_description')}, expected_layout=copy.deepcopy(LAYOUT),
        nodes=nodes, **roots, shaders=shaders, numeric_comparison_bytes=168, uniform_cases=cases, owner_uniform_cases=owner_cases,
        owner_parameter_expectations=expectations, unused_owner_parameter_expectations=unused_expectations, graph_effective_parameter_expectations=graph_expectations, draw_parameter='Color1', texture_2d_parameters=[expression],
        ordered_texture_references=context['ordered_texture_references'], texture_context_proof=context,
        texture_contract=dict(expression_kind='TextureParameter', parameter=dict(name=dict(name='TextureMask',number=0),association=2,index=-1), default_texture_path=TEXTURE_PATH, override_texture_path=TEXTURE_PATH,
            default=meta, source_platform_data=native_texture, named_overrides_allowed=True,
            scope='Original named TextureMask parameter with source-verified parent default and instance override'),
        shader_binding_contracts=contracts, expected_uniform_layout_hash=verifier.uniform_layout_hash(**dict(size=208, offsets=LAYOUT['resource_offsets'], types=LAYOUT['resource_types'])),
        provenance=dict(resource_sha256=parsed['resource_sha256'], shader_library_sha256=archive_hash))
    return dict(schema='fortnite.legacy-material-ingress.v4', owner_path=OWNER, owner_type='UMaterialInstanceConstant', parent_path=PARENT,
        shader_identity=PARENT, resource_owner_path=OWNER,
        framing=framing, decoded=decoded, original_instance_properties=props, runtime_fixture=fixture, resource_raw_hex=payload.hex(), original_shader_code=original_code,
        provenance=dict(inputs=[receipt(p) for p in dict.fromkeys(source_paths)], resource_sha1=hashlib.sha1(payload).hexdigest(),
            shader_library=dict(path=str(archive_path), sha256=archive_hash), exporter_sha256=digest(Path(__file__).read_bytes()),
            tools=[receipt(HERE / name) for name in ('export_ingress_bundle.py', 'legacy_shader_map.py', 'legacy_shader_records.py',
                'evaluate_uniforms.py', 'export_runtime_fixture.py', 'reflect_mapped_spirv.py', 'inspect_d3d_code.py', 'verify_runtime_artifacts.py')]),
        limitations=['Bounded actual original parent only; all 19 records retained and seven Slate programs translated',
                    'Original named TextureMask identity and actual instance override are retained',
                    'Source pixels/descriptor contracts are offline evidence; actual owner GT/RT attachment, RHI draw and readbacks remain unverified',
                    'No texture resizing, substitution, TextureProperty, virtual texture or generic texture graph support'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--inventory', type=Path, default=GENERATED / 'frontend-probes/frontend-material-inventory')
    parser.add_argument('--function-reports', type=Path, default=ROOT / 'flogo-functions-1')
    parser.add_argument('--texture-report', type=Path, default=ROOT / 'flogo-texture-1/texture-report.json')
    parser.add_argument('--shader-directory', type=Path, default=ROOT / 'flogo-shaders-1')
    parser.add_argument('--output', type=Path, default=ROOT / 'flogo-ingress-3/bundle.json')
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
    except (OSError, ValueError, KeyError, TypeError, IndexError, StopIteration, struct.error, subprocess.SubprocessError) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
