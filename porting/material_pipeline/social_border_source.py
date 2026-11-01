#!/usr/bin/env python3
"""Source-only evidence for Social border; never evaluate TextureProperty lanes."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import tempfile

HERE=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('hollow',HERE/'export_hollow_progress_ingress.py');h=importlib.util.module_from_spec(spec);spec.loader.exec_module(h)
bundle=h.bundle; digest=h.digest; read_json=h.read_json; receipt=h.receipt; unique_rows=h.unique_rows
ROOT=h.ROOT; GENERATED=h.GENERATED; EXTRACTED=h.EXTRACTED
PACKAGE='FortniteGame/Content/UI/SocialPanel/Art/Materials/M_UI_SocialUserItemBorder_Master'
OWNER='/Game/UI/SocialPanel/Art/Materials/M_UI_SocialUserItemBorder_Master.M_UI_SocialUserItemBorder_Master'
ATLAS='FortniteGame/Content/UI/SocialPanel/Art/CA_SocialPanel'
ATLAS_OBJECT=ATLAS+'.CA_SocialPanel'
FUNCTION='Engine/Content/Functions/UserInterface/GetUserInterfaceUV'
LAYOUT=dict(constant_bytes=160,resource_offsets=[128,136,144,152],resource_types=[5,8,8,8])
RESOURCE_SHA256='d9107ef73a3a33c5b1c1075806184342b2dca49cc2cec50b37e187b1b76c3962'
SOURCE_HASHES={
 PACKAGE+'.uasset':'70be8fb9ccc2ac5a8feb70be1a06f51daa3cec63714683470f9980014c0848ed',
 PACKAGE+'.uexp':'6895905dafbb0cb5fd23d0dd3fe9ba9d11ff7d7aaad1452d9304693a6ce92c2b',
 ATLAS+'.uasset':'39d404e1c6b2af2cd07a4786890f47b7c7fa72757ffbad3ef65cb7e517972e23',
 ATLAS+'.uexp':'45a48f299e3cf56282695ae000376f3105dbaaf8ab9cf83d829a97a73bc21037',
 FUNCTION+'.uasset':'a714d34c0d2e05f88b65f6754a2ee9e7a7ff7e8af7f30a60638fd23622e3cd2e',
 FUNCTION+'.uexp':'9a2d2b522143b51e7a17d4c79dbcf475427661e97064813c9e9aa8542850603f',
}
ATLAS_TOOL=HERE/'social_border_texture_probe/bin/Release/net10.0/SocialBorderTextureProbe.dll'
ATLAS_TOOL_SHA='6c5ef81dc99799e37d85ac2644db9801d51545103761ec05d75eed2b1e0db9e5'


def atlas_frame(meta, raw, header_bytes):
    if (meta['schema']!='fortnite.social-border-atlas-metadata.v1' or meta['package']!=ATLAS or meta['name']!='CA_SocialPanel' or
        meta['export_type']!='CurveLinearColorAtlas' or meta['pixelFormat']!='PF_FloatRGBA' or meta['srgb'] is not True or
        meta['filter']!='TF_Default' or meta['lodGroup']!='TEXTUREGROUP_World' or meta['addressX']!='TA_Wrap' or meta['addressY']!='TA_Wrap' or
        meta['firstMip']!=0 or len(meta['mips'])!=1): raise ValueError('unexpected atlas source metadata')
    mip=meta['mips'][0]
    if any(mip.get(k)!=v for k,v in dict(width=32,height=32,depth=1,bulkBytes=8192).items()): raise ValueError('unexpected atlas mip metadata')
    marker=b'PF_FloatRGBA\0'
    if raw.count(marker)!=1: raise ValueError('ambiguous atlas platform format')
    pos=raw.index(marker)
    if struct.unpack_from('<IIIi',raw,pos-16)!=(32,32,1,len(marker)): raise ValueError('wrong logical atlas dimensions')
    cursor=pos+len(marker); first,count,cooked,flags,elements,size,offset=struct.unpack_from('<iiiiiiq',raw,cursor);cursor+=32
    if (first,count,cooked,flags,elements,size,offset)!=(0,1,1,0x48,8192,8192,header_bytes+cursor): raise ValueError('wrong atlas inline mip framing')
    pixels=raw[cursor:cursor+size];cursor+=size
    if len(pixels)!=8192 or struct.unpack_from('<iiii',raw,cursor)!=(32,32,1,0): raise ValueError('wrong atlas mip dimensions or VT flag')
    return dict(logical_width=32,logical_height=32,format='PF_FloatRGBA',first_mip=0,mip_count=1,stored_bulk_offset=offset,
                raw_bytes=len(pixels),raw_sha256=digest(pixels),raw_encoding='original little-endian half-float pixel bytes, unconverted')


def source_context(owner, exports, function):
    if owner['path']!=PACKAGE or exports['package']!=PACKAGE or function['path']!=FUNCTION or function['skippedMetadata']:
        raise ValueError('source owner/function identity differs')
    rows=unique_rows(exports['exports'],'index'); expressions=unique_rows(owner['expressionExports'],'name')
    if len(function['objects'])!=1 or function['objects'][0]['Type']!='MaterialFunction' or function['objects'][0]['Name']!='GetUserInterfaceUV':
        raise ValueError('unexpected function closure')
    if any(x is not None for x in function['objects'][0]['Properties']['FunctionExpressions']):
        raise ValueError('function closure contains unproven expression')
    trace=[]
    for index,ref in enumerate(owner['materialProperties'][0]['expressionRefs']):
        if ref['index']==0:continue
        if ref['isImport'] or rows[ref['index']]['name']!=ref['name']:raise ValueError('expression source index/name mismatch')
        e=expressions[ref['name']];kind=e['exportType'];props=unique_rows(e['properties'],'name')
        if kind in ('MaterialExpressionScalarParameter','MaterialExpressionVectorParameter'):continue
        if kind=='MaterialExpressionCurveAtlasRowParameter':
            if props['Atlas']['value']!="CurveLinearColorAtlas'"+ATLAS_OBJECT+"' (ObjectProperty)":raise ValueError('curve atlas object differs')
            trace.append(dict(position=index,expression=ref['name'],kind=kind,atlas_object=ATLAS_OBJECT))
        elif kind=='MaterialExpressionMaterialFunctionCall':
            if props['MaterialFunction']['value']!="MaterialFunction'"+FUNCTION+".GetUserInterfaceUV' (ObjectProperty)":raise ValueError('function target differs')
            trace.append(dict(position=index,expression=ref['name'],kind=kind,function=FUNCTION,texture_appends=[]))
        else:raise ValueError('unproven source expression class: '+kind)
    if [x['position'] for x in trace]!=[3,5,9]:raise ValueError('unexpected source texture/function order')
    return dict(trace=trace,candidate_atlas=ATLAS_OBJECT,ordered_texture_references=None,
                status='unproven until original CurveAtlasRowParameter GetReferencedTexture behavior is established')


def check_arrays(parsed):
    a=parsed['uniform_expression_arrays']; vectors=a['vector']['expressions']
    if a['vector']['count']!=5 or a['scalar']['count']!=9 or len(vectors)!=5:raise ValueError('unexpected numeric root counts')
    expr=vectors[2]
    expected=dict(texture_index=0,layer_index=-1,page_table_layer_index=-1,sampler_source=2,virtual_texture=False)
    if (expr['type']!='FMaterialUniformExpressionTextureProperty' or expr['property']!=0 or len(expr['inputs'])!=1 or
        expr['inputs'][0]['type']!='FMaterialUniformExpressionTexture' or expr['inputs'][0]['texture']!=expected):
        raise ValueError('unexpected texture property source or child identity')
    if a['texture_2d']['count']!=1 or a['texture_2d']['expressions'][0]['type']!='FMaterialUniformExpressionTexture' or a['texture_2d']['expressions'][0]['texture']!=expected:
        raise ValueError('unexpected regular texture root')
    if any(v['count'] for k,v in a.items() if k.startswith('texture_') and k!='texture_2d') or parsed['virtual_texture_stacks']['count'] or parsed['parameter_collections']['count']:
        raise ValueError('unexpected texture arrays or collections')
    if parsed['shader_record_count']!=19 or any(r['debug_uniform_buffer_layout']!=LAYOUT for r in parsed['material_shaders']['records']):
        raise ValueError('unexpected original shader records or layout')
    return dict(vector_root_index=2,property=0,child_texture=expected,lane_semantics='unresolved: original writes R/G only; B/A caller initialization is not established')


def descriptor_contract(record, reflected):
    vertex=record['frequency']==0
    if record['frequency'] not in (0,3) or record['material_uniform_buffer']!=dict(base_index=0 if vertex else 1,bound=not vertex):raise ValueError('unexpected original material binding')
    roles=[]
    for d in reflected['descriptors']:
        reg=d['d3d_register']; entries=d['resource_table_entries']
        if reg['space'] or d['set']:raise ValueError('unsupported descriptor space')
        if entries:
            expected_resource=0 if reg['kind']=='srv' else 3 if reg['kind']=='sampler' else -1
            expected_table='texture' if reg['kind']=='srv' else 'sampler'
            if vertex or expected_resource<0 or len(entries)!=1 or entries[0]!=dict(uniform_buffer=1,resource=expected_resource,bind=reg['index'],table=expected_table):raise ValueError('unexpected material resource table')
            if d['vulkan_descriptor_type']!=(2 if reg['kind']=='srv' else 0):raise ValueError('unexpected descriptor type')
            roles.append('MaterialTexture' if expected_resource==0 else 'WorldClampSampler')
        elif reg['kind']=='cbv' and d['vulkan_descriptor_type']==6:
            if reg['index']==0 and d['block_span_bytes']==(80 if vertex else 64):roles.append('SlateLooseConstants')
            elif not vertex and reg['index']==1 and d['block_span_bytes']==112:roles.append('Material')
            else:raise ValueError('unexpected constant buffer role/span')
        elif not vertex and reg['index']==1 and reg['kind'] in ('srv','sampler') and d['vulkan_descriptor_type']==(2 if reg['kind']=='srv' else 0):
            roles.append('ElementTexture' if reg['kind']=='srv' else 'ElementTextureSampler')
        else:raise ValueError('unclassified descriptor')
        if reg['kind']=='srv' and d.get('image_type')!=h.SAMPLED_IMAGE:raise ValueError('unexpected sampled image shape')
    expected={'SlateLooseConstants'} if vertex else {'SlateLooseConstants','Material','MaterialTexture','WorldClampSampler'}
    if 'ElementTexture' in roles or 'ElementTextureSampler' in roles:expected|={'ElementTexture','ElementTextureSampler'}
    if len(roles)!=len(set(roles)) or set(roles)!=expected:raise ValueError('incomplete original resource roles')
    return dict(roles=roles,material_texture_resource_index=0 if not vertex else None,world_clamp_sampler_resource_index=3 if not vertex else None)


def make_source(inventory, function_report, atlas_report, shaders):
    for rel,expected in SOURCE_HASHES.items():
        if digest((EXTRACTED/rel).read_bytes())!=expected:raise ValueError('original source hash differs: '+rel)
    if digest(h.INSPECTION_TOOL.read_bytes())!=h.INSPECTION_SHA256 or digest(h.FUNCTION_TOOL.read_bytes())!=h.FUNCTION_TOOL_SHA256 or digest(ATLAS_TOOL.read_bytes())!=ATLAS_TOOL_SHA:
        raise ValueError('source inspection tool changed')
    if digest(h.CONTEXT_RECEIPT.read_bytes())!=h.CONTEXT_SHA256 or digest(h.DIMENSION_RECEIPT.read_bytes())!=h.DIMENSION_SHA256:
        raise ValueError('original source context proof changed')
    stem=PACKAGE.replace('/','_');paths=[inventory/(stem+x) for x in ('.json','.export-map.json','.inline-resource.json','.decoded.json')]
    owner,export_map,framing,decoded=[read_json(p)[0] for p in paths]
    actual=subprocess.run(['dotnet',str(h.INSPECTION_TOOL),PACKAGE,str(EXTRACTED),'--loose','--material-report'],capture_output=True,text=True,timeout=180,check=True)
    if json.loads(actual.stdout[actual.stdout.index('{'):])!=owner:raise ValueError('fresh material source report differs')
    if (len(owner['materials'])!=1 or owner['materials'][0].get('name')!='M_UI_SocialUserItemBorder_Master' or
        owner['materials'][0].get('type')!='UMaterial' or owner['materials'][0].get('expressionCount')!=10 or
        owner['materials'][0].get('blendMode')!='BLEND_Translucent' or owner['materials'][0].get('shadingModel')!='MSM_Unlit' or
        unique_rows(owner['materialProperties'][0]['properties'],'name')['MaterialDomain']['value']!='MD_UI (EnumProperty)'):
        raise ValueError('original owner identity/domain differs')
    function,_=read_json(function_report)
    with tempfile.TemporaryDirectory(prefix='social-border-function-') as temp:
        output=Path(temp)/'out';subprocess.run(['dotnet',str(h.FUNCTION_TOOL),str(EXTRACTED),str(output),FUNCTION],capture_output=True,text=True,timeout=180,check=True)
        files=list(output.glob('*.json'))
        if len(files)!=1 or read_json(files[0])[0]!=function:raise ValueError('fresh function closure differs')
    context=source_context(owner,export_map,function)
    meta,_=read_json(atlas_report)
    with tempfile.TemporaryDirectory(prefix='social-border-atlas-') as temp:
        output=Path(temp)/'atlas.json';subprocess.run(['dotnet',str(ATLAS_TOOL),str(EXTRACTED),str(output)],capture_output=True,text=True,timeout=180,check=True)
        if read_json(output)[0]!=meta:raise ValueError('fresh atlas metadata differs')
    atlas=atlas_frame(meta,(EXTRACTED/(ATLAS+'.uexp')).read_bytes(),(EXTRACTED/(ATLAS+'.uasset')).stat().st_size)
    source=(EXTRACTED/(PACKAGE+'.uexp')).read_bytes()
    if framing['package']!=PACKAGE or framing['export_name']!='M_UI_SocialUserItemBorder_Master' or framing['export_map_sha256']!=digest(paths[1].read_bytes()) or decoded['package']!=PACKAGE:
        raise ValueError('resource framing/source identity differs')
    raw=bundle.validate_source(source,framing)
    if len(raw)!=9001 or digest(raw)!=RESOURCE_SHA256:raise ValueError('source resource hash/length differs')
    parsed=bundle.decoder.parse_prefix(raw,framing['names'])
    archive_path=EXTRACTED/'FortniteGame/Content/ShaderArchive-FortniteGame-PCD3D_SM5.ushaderbytecode';archive_hash=bundle.library_reader.sha256(archive_path)
    if archive_hash!='2e795bd21c5e9bd69a6fdb520ef73f6fd70822c011c4232ce1ba08ccf10aef21':raise ValueError('original shader archive differs')
    originals=[]
    with bundle.library_reader.ShaderArchive(archive_path,layout='fortnite-12.60') as archive:
        bundle.decoder.associate_library(parsed,archive)
        for record in bundle.all_records(parsed):
            index=record['library_association']['index'];code=archive.shader_code(index)
            originals.append(dict(type=record['type'],output_hash=record['output_hash']['hash'],library_index=index,frequency=record['frequency'],bytes=len(code),sha256=digest(code)))
    for key,value in parsed.items():
        if key=='original_serializer_evidence':
            if any(decoded[key].get(k)!=v for k,v in value.items()):raise ValueError('original serializer evidence differs')
        elif decoded.get(key)!=value:raise ValueError('original complete parse differs: '+key)
    if not parsed['complete_serialized_shader_map'] or parsed['undecoded_bytes'] or parsed['shader_record_count']!=19 or len(originals)!=19 or parsed['material_path']!=OWNER:
        raise ValueError('incomplete original source resource')
    property_contract=check_arrays(parsed)
    shader_paths=[shaders/'archive.json',shaders/'d3d.json',shaders/'mapped/mapped-translation-report.json',shaders/'reflection.json']
    src,d3d,mapped,reflected=[read_json(p)[0] for p in shader_paths]
    if (src['sha256']!=archive_hash or d3d['archive_sha256']!=archive_hash or mapped['archive_sha256']!=archive_hash or src['selection_truncated'] or mapped['truncated'] or
        d3d['source_report_sha256']!=digest(shader_paths[0].read_bytes()) or mapped['source_report_sha256']!=digest(shader_paths[0].read_bytes()) or
        mapped['d3d_report_sha256']!=digest(shader_paths[1].read_bytes()) or reflected['mapped_report_sha256']!=digest(shader_paths[2].read_bytes())):
        raise ValueError('mapped shader source provenance differs')
    tables=[unique_rows(doc[key],identity) for doc,key,identity in [(src,'shaders','index'),(d3d,'records','shader_index'),(mapped,'records','shader_index'),(reflected,'records','shader_index')]]
    expected_indices={2446,2447,3361,3362,3363,3364}
    if any(set(t)!=expected_indices for t in tables) or {r['library_association']['index'] for r in parsed['material_shaders']['records']}!=expected_indices:
        raise ValueError('original shader selection differs')
    validators=[Path(p) for p in mapped['tool_sha256'] if Path(p).name=='spirv-val']
    if len(validators)!=1:raise ValueError('missing SPIR-V validator')
    for p,expected in mapped['tool_sha256'].items():
        if digest(Path(p).read_bytes())!=expected:raise ValueError('mapped shader tool changed')
    shader_records=[];shader_inputs=[]
    for record in parsed['material_shaders']['records']:
        index=record['library_association']['index']; original,direct,translated,reflection=[t[index] for t in tables]
        wrapper=Path(original['wrapper_path']).read_bytes();containers=bundle.library_reader.dxbc_containers(wrapper)
        if (digest(wrapper)!=original['decoded_sha256'] or len(containers)!=1 or direct['wrapper_sha256']!=digest(wrapper) or
            {k:v for k,v in direct.items() if k not in ('shader_index','frequency','wrapper_sha256')}!=h.d3d_tool.inspect_wrapper(wrapper,containers)):
            raise ValueError('original wrapper/resource table differs')
        with bundle.library_reader.ShaderArchive(archive_path,layout='fortnite-12.60') as archive:
            if archive.shader_code(index)!=wrapper:raise ValueError('wrapper differs from original archive')
        dxbc=Path(translated['input']).read_bytes();container=containers[0]
        if dxbc!=wrapper[container['offset']:container['offset']+container['size']] or digest(dxbc)!=translated['input_sha256'] or translated['wrapper_sha256']!=digest(wrapper):
            raise ValueError('mapped DXBC source differs')
        code=Path(reflection['output']).read_bytes()
        if (GENERATED.resolve() not in Path(reflection['output']).resolve().parents or not translated['validated'] or translated['unmatched_resource_table_entries'] or translated['compiler_diagnostics'] or
            digest(code)!=translated['output_sha256'] or digest(code)!=reflection['spirv_sha256'] or translated['frequency']!=record['frequency'] or reflection['frequency']!=record['frequency']):
            raise ValueError('mapped shader code differs')
        validation=subprocess.run([str(validators[0]),'--target-env','vulkan1.0',reflection['output']],capture_output=True,text=True,timeout=30)
        if validation.returncode or validation.stdout or validation.stderr:raise ValueError('SPIR-V validator rejected shader')
        fresh=h.reflection_tool.Module(code).reflect()
        if fresh['entry_point']!=reflection['entry_point'] or fresh['interfaces']!=reflection['interfaces'] or len(fresh['descriptors'])!=len(reflection['descriptors']):raise ValueError('fresh SPIR-V reflection differs')
        for observed,declared in zip(fresh['descriptors'],reflection['descriptors']):
            if any(declared.get(k)!=v for k,v in observed.items()):raise ValueError('fresh descriptor reflection differs')
        shader_records.append(dict(record=record,mapped=translated,reflection=reflection,contract=descriptor_contract(record,reflection)))
        shader_inputs.extend([Path(original['wrapper_path']),Path(translated['input']),Path(reflection['output'])])
    paths += shader_paths+shader_inputs+[function_report,atlas_report,h.INSPECTION_TOOL,h.FUNCTION_TOOL,ATLAS_TOOL,h.CONTEXT_RECEIPT,h.DIMENSION_RECEIPT]
    paths += [EXTRACTED/p for p in SOURCE_HASHES]
    return dict(schema='fortnite.social-border-source-evidence.v1',owner_path=OWNER,framing=framing,decoded=decoded,
        resource_raw_hex=raw.hex(),resource_sha1=hashlib.sha1(raw).hexdigest(),texture_property=property_contract,source_context=context,
        atlas_metadata=meta,atlas_frame=atlas,shader_records=shader_records,original_shader_code=originals,
        runtime_fixture=None,numeric_values=None,provenance=dict(inputs=[receipt(p) for p in dict.fromkeys(paths)],
            shader_library=dict(path=str(archive_path),sha256=archive_hash),exporter_sha256=digest(Path(__file__).read_bytes())),
        limitations=['Source-only; no TextureProperty numeric flattening or runtime conversion',
          'Original texture-size getter writes R/G only; caller B/A initialization and live lanes are unresolved',
          'Ordered runtime texture-reference array remains unproven until original CurveAtlasRowParameter getter is established',
          'Actual atlas loading, uniform evaluation, GPU attachment and rendering remain unverified'])


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--inventory',type=Path,default=GENERATED/'frontend-probes/frontend-material-inventory-textures')
    ap.add_argument('--function-report',type=Path,default=ROOT/'social_border-function-1/Engine_Content_Functions_UserInterface_GetUserInterfaceUV.json')
    ap.add_argument('--atlas-report',type=Path,default=ROOT/'social_border-atlas-metadata-1.json')
    ap.add_argument('--shaders',type=Path,default=ROOT/'social_border-shaders-1')
    ap.add_argument('--output',type=Path,default=ROOT/'social_border-source-1/source.json');args=ap.parse_args()
    try:
        if GENERATED.resolve() not in args.output.resolve().parents or args.output.exists():raise ValueError('choose fresh generated output')
        data=make_source(args.inventory,args.function_report,args.atlas_report,args.shaders)
        raw=(json.dumps(data,indent=2,allow_nan=False)+'\n').encode();args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_bytes(raw)
        print(json.dumps(dict(path=str(args.output.resolve()),sha256=digest(raw),resource_sha1=data['resource_sha1'],original_records=len(data['original_shader_code']),slate_associations=len(data['shader_records']),ordered_references_proven=False,numeric_lanes_proven=False)))
    except (OSError,ValueError,KeyError,TypeError,IndexError,StopIteration,struct.error,subprocess.SubprocessError) as error:ap.exit(1,'error: '+str(error)+'\n')
if __name__=='__main__':main()
