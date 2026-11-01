#!/usr/bin/env python3
"""Bounded original M_TempIcon_Login_Google named BC1 texture ingress.

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
    module = importlib.util.module_from_spec(spec); spec.loader.exec_module(module); return module
numeric=sibling('export_numeric_ingress')
bundle,evaluator=numeric.bundle,numeric.evaluator
reflection_tool=sibling('reflect_mapped_spirv');d3d_tool=sibling('inspect_d3d_code');verifier=sibling('verify_runtime_artifacts')
GENERATED=HERE.parent/'generated';ROOT=GENERATED/'material_pipeline';EXTRACTED=Path('/mnt/vibrato/fortnite-extracted-full')
PACKAGE='FortniteGame/Content/UI/Login/Art/M_TempIcon_Login_Google'
OWNER='/Game/UI/Login/Art/M_TempIcon_Login_Google.M_TempIcon_Login_Google'
TEXTURE_PACKAGE='FortniteGame/Content/UI/Login/Art/T_TempIcon_Login_Google'
TEXTURE_OBJECT=TEXTURE_PACKAGE+'.T_TempIcon_Login_Google'
TEXTURE_PATH='/Game/UI/Login/Art/T_TempIcon_Login_Google.T_TempIcon_Login_Google'
LAYOUT=dict(constant_bytes=80,resource_offsets=[48,56,64,72],resource_types=[5,8,8,8])
BASE_TEXTURE=dict(texture_index=0,layer_index=-1,page_table_layer_index=-1,sampler_source=0,virtual_texture=False)
PARAMETER=dict(name=dict(name='Texture',number=0),association=2,index=-1)
SAMPLED_IMAGE=dict(sampled_type=dict(kind='float',bits=32),dimension=1,depth=2,arrayed=0,multisampled=0,sampled=1,format=0)
CONTEXT_RECEIPT=ROOT/'original-texture-context-evidence-v3.json'
CONTEXT_SHA256='5617b511c2abe2553c8186182eec24f548fdff4872fbd366bea70f8c56f8cb08'
DIMENSION_RECEIPT=ROOT/'original-texture-context-evidence-v5.json'
DIMENSION_SHA256='c70a12b30eea35dfbeae40679050a89231fe520c3a367c79269d808f52aede44'
INSPECTION_TOOL=GENERATED/'frontend-probes/cue4inspect-donut-textures/Cue4Inspect.dll'
INSPECTION_SHA256='bd1e145743c2c93832ddf69e3965bace3dfda3b4ac6f5292cb409e1bbdfda676'
RESOURCE_SHA256='6a864eab7a50f8e478f1caf7264816e955f99d23c29c2fd311384c1cec01a925'
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
            elif not vertex and record['material_uniform_buffer'] == dict(base_index=reg['index'], bound=True) and reg['index'] == 1 and desc['block_span_bytes'] == 48:
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


def prove_context(owner,export_map):
    refs=owner['materialProperties'][0]['expressionRefs']; exports=unique_rows(export_map['exports'],'index');expressions=unique_rows(owner['expressionExports'],'name')
    expected=[('MaterialExpressionTextureSampleParameter2D_0','MaterialExpressionTextureSampleParameter2D'),('MaterialExpressionVectorParameter_4','MaterialExpressionVectorParameter'),None,None,('MaterialExpressionScalarParameter_2','MaterialExpressionScalarParameter'),None]
    if len(refs)!=len(expected) or len(expressions)!=3:raise ValueError('unexpected original expression graph')
    trace=[]
    for position,(ref,want) in enumerate(zip(refs,expected)):
        if want is None:
            if ref!=dict(name='None',index=0,isImport=False):raise ValueError('unexpected non-null expression')
            continue
        name,kind=want
        if ref['name']!=name or ref['isImport'] or exports[ref['index']]['name']!=name or expressions[name]['exportType']!=kind:raise ValueError('original expression reference/class mismatch')
        ep=unique_rows(expressions[name]['properties'],'name')
        if position==0:
            if ep['Texture']['value']!="Texture2D'"+TEXTURE_OBJECT+"' (ObjectProperty)" or ep['ParameterName']['value']!='Texture (NameProperty)':raise ValueError('wrong exact named texture source')
            trace.append(dict(expression_position=position,texture_object=TEXTURE_OBJECT,reference_index=0))
    return dict(algorithm='Original UMaterial PostLoad 144314eb0 -> 14430b550, expression order/AddUnique; no function calls',ordered_texture_references=[TEXTURE_PATH],trace=trace)

def validate_arrays(parsed):
    arrays=parsed['uniform_expression_arrays'];tex=arrays['texture_2d']
    if tex['count']!=1 or tex['expressions']!=[dict(offset=338,type='FMaterialUniformExpressionTextureParameter',parameter=PARAMETER,texture=BASE_TEXTURE,end=375)] or any(v['count'] for k,v in arrays.items() if k.startswith('texture_') and k!='texture_2d') or parsed['virtual_texture_stacks']['count'] or parsed['parameter_collections']['count']:raise ValueError('named texture expression differs')
    nodes,roots=bundle.fixture_exporter.flatten_expressions(arrays)
    if len(roots['vector_roots'])!=2 or len(roots['scalar_roots'])!=3 or any(r['debug_uniform_buffer_layout']!=LAYOUT for r in parsed['material_shaders']['records']):raise ValueError('root counts/layout differ')
    return nodes,roots

def create_bundle(inventory,texture_report,shader_directory):
    stem=PACKAGE.replace('/','_')
    paths=[inventory/(stem+suffix) for suffix in ('.json','.export-map.json','.inline-resource.json','.decoded.json')]
    owner,export_map,framing,decoded=[read_json(p)[0] for p in paths]
    for path,expected in [(CONTEXT_RECEIPT,CONTEXT_SHA256),(DIMENSION_RECEIPT,DIMENSION_SHA256),(INSPECTION_TOOL,INSPECTION_SHA256)]:
        if digest(path.read_bytes())!=expected:raise ValueError('original tool/context proof changed')
    source_paths=paths+[CONTEXT_RECEIPT,DIMENSION_RECEIPT,INSPECTION_TOOL,Path(__file__).with_name('google_frame.py')]
    fresh=subprocess.run(['dotnet',str(INSPECTION_TOOL),PACKAGE,str(EXTRACTED),'--loose','--material-report'],capture_output=True,text=True,timeout=180,check=True,cwd=HERE.parent.parent)
    if json.loads(fresh.stdout[fresh.stdout.index('{'):])!=owner:raise ValueError('fresh original material export differs')
    if len(owner['materials'])!=1 or any(owner['materials'][0].get(k)!=v for k,v in dict(name='M_TempIcon_Login_Google',type='UMaterial',blendMode='BLEND_Translucent',shadingModel='MSM_Unlit',twoSided=False,expressionCount=6).items()):raise ValueError('original owner identity changed')
    props={x['name']:x for x in owner['materialProperties'][0]['properties']}
    if props['MaterialDomain']['value']!='MD_UI (EnumProperty)' or owner['instanceParameters']:raise ValueError('wrong domain/instance')
    context=prove_context(owner,export_map)
    source=EXTRACTED/(PACKAGE+'.uexp');raw=source.read_bytes()
    if export_map['headerBytes']!=source.with_suffix('.uasset').stat().st_size or framing['package']!=PACKAGE or framing['export_name']!='M_TempIcon_Login_Google' or framing['export_map_sha256']!=digest(paths[1].read_bytes()) or decoded['package']!=PACKAGE:raise ValueError('source framing changed')
    payload=bundle.validate_source(raw,framing)
    if digest(payload)!=RESOURCE_SHA256 or len(payload)!=8449:raise ValueError('original resource hash/length differs')
    parsed=bundle.decoder.parse_prefix(payload,framing['names'])
    archive_path=EXTRACTED/'FortniteGame/Content/ShaderArchive-FortniteGame-PCD3D_SM5.ushaderbytecode'
    archive_hash=bundle.library_reader.sha256(archive_path)
    if archive_hash!='2e795bd21c5e9bd69a6fdb520ef73f6fd70822c011c4232ce1ba08ccf10aef21':raise ValueError('original archive changed')
    original_code=[]
    with bundle.library_reader.ShaderArchive(archive_path,layout='fortnite-12.60') as archive:
        bundle.decoder.associate_library(parsed,archive)
        for record in bundle.all_records(parsed):
            index=record['library_association']['index'];code=archive.shader_code(index)
            original_code.append(dict(type=record['type'],output_hash=record['output_hash']['hash'],library_index=index,frequency=record['frequency'],bytes=len(code),sha256=digest(code)))
    for key,value in parsed.items():
        if key=='original_serializer_evidence':
            if any(decoded[key].get(k)!=v for k,v in value.items()):raise ValueError('serializer evidence changed')
        elif decoded.get(key)!=value:raise ValueError('complete original parse changed: '+key)
    if not parsed['complete_serialized_shader_map'] or parsed['undecoded_bytes'] or parsed['shader_record_count']!=19 or parsed['material_path']!=OWNER:raise ValueError('incomplete original resource')
    nodes,roots=validate_arrays(parsed)
    meta,_=read_json(texture_report)
    if (meta['width'],meta['height'],meta['pixelFormat'],meta['decodedPixelFormat'],meta['firstMip'],meta['srgb'],meta['filter'],meta['lodGroup'],meta['addressX'],meta['addressY'])!=(512,512,'PF_DXT1','PF_R8G8B8A8',0,True,'TF_Default','TEXTUREGROUP_World','TA_Wrap','TA_Wrap'):raise ValueError('original texture metadata differs')
    with tempfile.TemporaryDirectory(prefix='google-texture-replay-',dir=ROOT) as temporary:
        replay=Path(temporary)/'output'
        subprocess.run(['dotnet',str(INSPECTION_TOOL),TEXTURE_PACKAGE,str(EXTRACTED),'--loose','--texture-report','--texture-output='+str(replay)],capture_output=True,text=True,timeout=180,check=True,cwd=HERE.parent.parent)
        actual,_=read_json(replay/'texture-report.json')
        if {k:v for k,v in actual.items() if k not in ('rgba','png')}!={k:v for k,v in meta.items() if k not in ('rgba','png')} or Path(actual['rgba']).read_bytes()!=Path(meta['rgba']).read_bytes() or Path(actual['png']).read_bytes()!=Path(meta['png']).read_bytes():raise ValueError('fresh texture metadata/decoded pixels differ')
    frame_spec=importlib.util.spec_from_file_location('google_frame',Path(__file__).with_name('google_frame.py'));frame=importlib.util.module_from_spec(frame_spec);frame_spec.loader.exec_module(frame)
    texture_sources=[EXTRACTED/(TEXTURE_PACKAGE+suffix) for suffix in ('.uasset','.uexp','.ubulk')]
    native_texture=frame.frame(*(p.read_bytes() for p in texture_sources),meta)
    if digest(Path(meta['rgba']).read_bytes())!=meta['rgbaSha256'] or digest(Path(meta['png']).read_bytes())!=meta['pngSha256']:raise ValueError('decoded texture hashes differ')
    source_paths += [source,source.with_suffix('.uasset'),texture_report,*texture_sources,Path(meta['rgba']),Path(meta['png'])]
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
    source_paths += shader_paths + [Path(p) for p in mapped['tool_sha256']]
    cases=uniform_cases(parsed,nodes)
    expression=dict(expression_kind='TextureParameter',parameter=PARAMETER,texture_index=0,texture_layer_index=-1,page_table_layer_index=-1,sampler_source=0,virtual_texture=False,default_texture_path=TEXTURE_PATH)
    fixture=dict(schema='fortnite.legacy-slate.v4',material={k:parsed[k] for k in ('friendly_name','material_path','quality_level','feature_level','cooked_shader_map_id_hash','debug_description')},expected_layout=copy.deepcopy(LAYOUT),nodes=nodes,**roots,shaders=shaders,numeric_comparison_bytes=44,uniform_cases=cases,owner_uniform_cases=copy.deepcopy(cases),draw_parameter='SelectionColor',texture_2d_parameters=[expression],ordered_texture_references=[TEXTURE_PATH],texture_context_proof=context,texture_contract=dict(expression_kind='TextureParameter',parameter=PARAMETER,default_texture_path=TEXTURE_PATH,default=meta,source_platform_data=native_texture,named_overrides_allowed=True,scope='Original exact Texture identity and indexed default; all ten original BC1 mips retained. Live named default/fallback and residency remain unverified.'),shader_binding_contracts=contracts,expected_uniform_layout_hash=verifier.uniform_layout_hash(size=80,offsets=LAYOUT['resource_offsets'],types=LAYOUT['resource_types']),provenance=dict(resource_sha256=parsed['resource_sha256'],shader_library_sha256=archive_hash))
    return dict(schema='fortnite.legacy-material-ingress.v4',owner_path=OWNER,owner_type='UMaterial',shader_identity=OWNER,resource_owner_path=OWNER,framing=framing,decoded=decoded,runtime_fixture=fixture,resource_raw_hex=payload.hex(),original_shader_code=original_code,provenance=dict(inputs=[receipt(p) for p in dict.fromkeys(source_paths)],resource_sha1=hashlib.sha1(payload).hexdigest(),shader_library=dict(path=str(archive_path),sha256=archive_hash),exporter_sha256=digest(Path(__file__).read_bytes()),tools=[receipt(HERE/name) for name in ('export_numeric_ingress.py','inspect_material_resource.py','shader_archive.py','export_ingress_bundle.py','legacy_shader_map.py','legacy_shader_records.py','evaluate_uniforms.py','export_runtime_fixture.py','reflect_mapped_spirv.py','inspect_d3d_code.py','verify_runtime_artifacts.py')]),limitations=['Offline source preparation only; no actual owner/resource attachment or draw verified','All nineteen original records and seven Slate programs retained','No raw BC1 GPU readback, residency, generic texture graph or full UI claim'])

def uniform_cases(parsed,nodes):
    params={n['parameter']['name']['name']:dict(type='scalar' if n['type']=='ScalarParameter' else 'vector',parameter=n['parameter']) for n in nodes if n['type'] in ('ScalarParameter','VectorParameter')}
    if set(params)!={'SelectionColor','IconFilter','RefractionDepthBias'}:raise ValueError('numeric parameter identities differ')
    cases=[]
    def append(name,overrides):
        raw=evaluator.evaluate_report(parsed,evaluator.overrides_from_json([] if name.startswith('unmatched_') else overrides))[0]
        if len(raw)!=48:raise ValueError('original numeric byte span differs')
        cases.append(dict(name=name,overrides=overrides,numeric_values=list(struct.unpack('<12f',raw)),numeric_sha256=digest(raw),abs_tolerance=0.00001,rel_tolerance=0.00001))
    append('defaults',[]);combined=[]
    for name,value in [('SelectionColor',[0.2,0.7,0.4,0.25]),('IconFilter',0.1),('RefractionDepthBias',0.3)]:
        entry=dict(copy.deepcopy(params[name]),value=value);combined.append(entry);append(name,[entry])
    append('combined',combined)
    for field,value in [('association',0),('index',7),('number',1)]:
        entry=copy.deepcopy(combined[0]);(entry['parameter']['name'] if field=='number' else entry['parameter'])[field]=value;append('unmatched_'+field,[entry])
    return cases

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--inventory',type=Path,default=GENERATED/'frontend-probes/frontend-material-inventory')
    parser.add_argument('--texture-report',type=Path,default=ROOT/'google-texture-1/texture-report.json')
    parser.add_argument('--shader-directory',type=Path,default=ROOT/'google-shaders-1')
    parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    if GENERATED.resolve() not in args.output.resolve().parents or args.output.exists() or args.output.with_name('runtime-fixture.json').exists():raise ValueError('fresh generated outputs required')
    data=create_bundle(args.inventory,args.texture_report,args.shader_directory)
    args.output.parent.mkdir(parents=True,exist_ok=True)
    raw=(json.dumps(data,indent=2,allow_nan=False)+'\n').encode();fixture=(json.dumps(data['runtime_fixture'],indent=2,allow_nan=False)+'\n').encode()
    args.output.write_bytes(raw);args.output.with_name('runtime-fixture.json').write_bytes(fixture)
    print(json.dumps(dict(bundle_sha1=hashlib.sha1(raw).hexdigest(),bundle_sha256=digest(raw),fixture_sha256=digest(fixture),resource_sha1=data['provenance']['resource_sha1'])))
if __name__=='__main__':main()
