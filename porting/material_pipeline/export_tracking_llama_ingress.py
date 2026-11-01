#!/usr/bin/env python3
"""Export the source-authenticated TrackingLlama material as an offline v2 bundle."""
import argparse, copy, hashlib, importlib.util, json, struct, subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent
REPO=HERE.parents[1]
GENERATED=HERE.parent/'generated'
ROOT=GENERATED/'material_pipeline'
INVENTORY=GENERATED/'frontend-probes/frontend-material-inventory-textures'
EXTRACTED=Path('/mnt/vibrato/fortnite-extracted-full')
PACKAGE='FortniteGame/Content/UI/Foundation/Modals/ErrorWindow/Materials/M_UI_TrackingLlama'
TEXTURE=PACKAGE.rsplit('/',1)[0]+'/Icon-Llama-NoEyes.Icon-Llama-NoEyes'
RESOURCE_HASH='f8e8037af481b446578a91a752726d56c0155b13e95e26e43193ee35bcf03ead'
ARCHIVE_SHA256='2e795bd21c5e9bd69a6fdb520ef73f6fd70822c011c4232ce1ba08ccf10aef21'
LAYOUT=dict(constant_bytes=144,resource_offsets=[112,120,128,136],resource_types=[5,8,8,8])
def sibling(name):
    spec=importlib.util.spec_from_file_location(name,HERE/(name+'.py')); module=importlib.util.module_from_spec(spec); spec.loader.exec_module(module); return module
comic=sibling('export_comicbubble_ingress'); numeric=comic.numeric; bundle=numeric.bundle
digest=comic.digest; require=comic.require; read_json=comic.read_json; resolve=comic.resolve; source_receipt=comic.source_receipt; write_bundles=comic.write_bundles

def validate_compiled(parsed):
    arrays=parsed['uniform_expression_arrays']
    require(parsed['complete_serialized_shader_map'] and parsed['undecoded_bytes']==0 and parsed['shader_record_count']==19 and len(bundle.all_records(parsed))==19 and parsed['resource_sha256']==RESOURCE_HASH,'original resource changed')
    require(arrays['texture_2d']['count']==1 and arrays['texture_2d']['expressions']==[dict(offset=815,type='FMaterialUniformExpressionTexture',end=839,texture=dict(texture_index=0,layer_index=-1,page_table_layer_index=-1,sampler_source=0,virtual_texture=False))], 'texture expression changed')
    require(all(v['count']==0 and v['expressions']==[] for k,v in arrays.items() if k.startswith('texture_') and k!='texture_2d') and parsed['virtual_texture_stacks']['count']==0 and parsed['parameter_collections']['count']==0,'unsupported textures or collections')
    require([arrays[k]['count'] for k in ('vector','scalar')]==[5,6], 'numeric counts changed')
    records=parsed['material_shaders']['records']
    require([r['type'] for r in records]==[dict(name=n,number=0) for n in comic.SLATE_NAMES] and [r['library_association']['index'] for r in records]==[2480,2481,15501,15502,15503,15504,15505], 'Slate programs changed')
    for r in records:
        vertex=r['type']['name'].startswith('TSlateMaterialShaderVS'); font=r['type']['name'] in ('TSlateMaterialShaderPSGrayscaleFont','TSlateMaterialShaderPSColorFont')
        require(r['frequency']==(0 if vertex else 3) and r['debug_uniform_buffer_layout']==LAYOUT and r['layout_name']==dict(name='Material',number=0) and r['material_uniform_buffer']==dict(base_index=0 if vertex else 1,bound=not vertex),'original layout changed')
        require(r['resource']['parameter_map']==dict(uniform_buffers=[] if vertex else [[1,0]],samplers=[[1,1]] if font else [],srvs=[[1,1]] if font else []) and r['resource_parameters']==([] if vertex else [dict(base_index=int(font),resource_count=int(font))]*2) and r['serialized_expression_counts']==[5,6,1,0,0,0,0], 'original resource bindings changed')
    return bundle.fixture_exporter.flatten_expressions(arrays)

def numeric_contract(parsed):
    nodes, roots = validate_compiled(parsed)
    inherited = []
    # Regular Texture roots are independent of the numeric graph; evaluate only numeric arrays.
    numeric_only=copy.deepcopy(parsed)
    for name,array in numeric_only['uniform_expression_arrays'].items():
        if name.startswith('texture_'):
            array.update(count=0,expressions=[])
    parameters={}
    for node in nodes:
        if node['type'] in ('ScalarParameter','VectorParameter'):
            identity=numeric.evaluator.identity(node['parameter'])
            parameters.setdefault(identity,dict(type='scalar' if node['type']=='ScalarParameter' else 'vector',parameter=node['parameter'],value=node['default']))
    cases=[]
    effective={numeric.evaluator.identity(x['parameter']):copy.deepcopy(x) for x in inherited}
    def append(name,changes):
        values=copy.deepcopy(effective)
        values.update({numeric.evaluator.identity(x['parameter']):x for x in changes})
        raw=numeric.evaluator.evaluate_report(numeric_only,numeric.evaluator.overrides_from_json([v for k,v in values.items() if k in parameters]))[0]
        cases.append(dict(name=name,overrides=changes,numeric_values=list(struct.unpack('<'+'f'*(len(raw)//4),raw)),numeric_sha256=digest(raw),abs_tolerance=0.00001,rel_tolerance=0.00001))
    append('defaults',[])
    for item in parameters.values():
        change=copy.deepcopy(item)
        value=change['value']
        change['value']=value+0.125 if change['type']=='scalar' else [v+0.125 for v in value]
        append(change['parameter']['name']['name'],[change])
    if parameters:
        change=copy.deepcopy(next(iter(parameters.values())))
        for field,value in [('association',0),('index',7),('number',1)]:
            altered=copy.deepcopy(change)
            (altered['parameter']['name'] if field=='number' else altered['parameter'])[field]=value
            append('unmatched_'+field,[altered])
    return dict(nodes=nodes,**roots,owner_parameter_expectations=inherited,owner_uniform_cases=cases,
                numeric_comparison_bytes=16*len(roots['vector_roots'])+4*len(roots['scalar_roots']))


def shader_evidence(checkpoint, directory):
    """Revalidate translation provenance, exact archive wrappers and fresh SPIR-V reflection."""
    reflection_tool=sibling('reflect_mapped_spirv');d3d_tool=sibling('inspect_d3d_code')
    paths=[directory/'archive.json',directory/'d3d.json',directory/'mapped/mapped-translation-report.json',directory/'reflection.json']
    source,d3d,mapped,reflected=[read_json(p) for p in paths]
    require(source['sha256']==d3d['archive_sha256']==mapped['archive_sha256']==ARCHIVE_SHA256
        and not source['selection_truncated'] and not mapped['truncated']
        and d3d['source_report_sha256']==mapped['source_report_sha256']==digest(paths[0].read_bytes())
        and mapped['d3d_report_sha256']==digest(paths[1].read_bytes())
        and reflected['mapped_report_sha256']==digest(paths[2].read_bytes()),'mapped shader report provenance changed')
    tables=[]
    for doc,key in [(source,'shaders'),(d3d,'records'),(mapped,'records'),(reflected,'records')]:
        rows=doc[key];identity='index' if key=='shaders' else 'shader_index'
        table={r[identity]:r for r in rows};require(len(table)==len(rows),'duplicate shader index');tables.append(table)
    expected={r['library_association']['index'] for o in checkpoint['owners'] for r in o['slate_programs']}
    require(len(expected)==7 and all(set(t)==expected for t in tables),'incomplete/extra original seven Slate selection')
    for path,expected_sha in mapped['tool_sha256'].items():
        require(digest(Path(path).read_bytes())==expected_sha,'mapped shader tool changed')
    validators=[Path(p) for p in mapped['tool_sha256'] if Path(p).name=='spirv-val']
    require(len(validators)==1,'missing exact SPIR-V validator')
    result=[]
    for instance,owner in enumerate(checkpoint['owners']):
        shaders=[];contracts=[]
        original_codes={r['library_index']:bytes.fromhex(r['wrapper_hex']) for r in owner['original_shader_code']}
        for record in owner['slate_programs']:
            index=record['library_association']['index'];original,direct,translated,reflection=[t[index] for t in tables]
            wrapper=Path(original['wrapper_path']).read_bytes();containers=bundle.library_reader.dxbc_containers(wrapper)
            require(wrapper==original_codes[index] and digest(wrapper)==original['decoded_sha256'] and len(containers)==1,'original shader wrapper changed')
            require({k:v for k,v in direct.items() if k not in ('shader_index','frequency','wrapper_sha256')}==d3d_tool.inspect_wrapper(wrapper,containers)
                and direct['wrapper_sha256']==digest(wrapper),'original D3D resource-table inspection changed')
            container=containers[0];dxbc=Path(translated['input']).read_bytes()
            require(dxbc==wrapper[container['offset']:container['offset']+container['size']] and digest(dxbc)==translated['input_sha256']
                and translated['wrapper_sha256']==digest(wrapper),'translated DXBC differs from original slice')
            path=Path(reflection['output']);code=path.read_bytes()
            require(GENERATED.resolve() in path.resolve().parents and translated['validated']
                and not translated['unmatched_resource_table_entries'] and not translated['compiler_diagnostics']
                and digest(code)==translated['output_sha256']==reflection['spirv_sha256']
                and translated['frequency']==reflection['frequency']==record['frequency'],'mapped shader code/stage changed')
            validation=subprocess.run([str(validators[0]),'--target-env','vulkan1.0',str(path)],capture_output=True,text=True,timeout=30)
            require(validation.returncode==0 and not validation.stdout and not validation.stderr,'mapped SPIR-V validation failed')
            actual=reflection_tool.Module(code).reflect()
            require(actual['entry_point']==reflection['entry_point'] and actual['interfaces']==reflection['interfaces']
                and len(actual['descriptors'])==len(reflection['descriptors']),'fresh SPIR-V reflection changed')
            for observed,declared in zip(actual['descriptors'],reflection['descriptors']):
                require(all(declared.get(k)==v for k,v in observed.items()),'fresh descriptor reflection changed')
            # Reconstruct annotations from the exact translated binding map, not the reflection report alone.
            mapping={r['binding']:r for r in translated['bindings']}
            for desc in reflection['descriptors']:
                require(desc['binding'] in mapping,'unmapped descriptor binding')
                annotation=mapping[desc['binding']]
                require(desc['d3d_register']==dict(kind=annotation['kind'],space=annotation['register_space'],index=annotation['register_index']) and desc['resource_table_entries']==annotation['resource_table_entries'], 'descriptor annotations differ from D3D mapping')
            contract=typed_contract(record,reflection);contracts.append(dict(shader_type=record['type'],library_index=index,**contract))
            shaders.append(dict(record=record,mapped=translated,reflection=reflection,spirv_sha1=hashlib.sha1(code).hexdigest()))
        result.append((shaders,contracts))
    return result



def validate_texture_frame(row, blobs):
    """Re-observe logical/stored dimensions, signed bulk offsets and every original BC3 mip."""
    header, export, bulk = blobs
    require([digest(x) for x in blobs] == [r['sha256'] for r in row['source_files']], 'texture source SHA mismatch')
    require([len(x) for x in blobs] == [r['bytes'] for r in row['source_files']], 'texture source length mismatch')
    require(row['texture_object'] == TEXTURE, 'unrecognized original texture')
    dim, count, external = 256,9,2
    require((row['logical_width'],row['logical_height'],row['mip_count'],row['pixel_format'],row['first_mip']) == (dim,dim,count,'PF_DXT5',0), 'logical texture contract changed')
    require(struct.unpack_from('<6i',header) == (-1641380927,-7,0,0,0,0), 'package summary changed')
    start = struct.unpack_from('<q',header,169)[0]
    require(start == len(header)+len(export)-4 == row['bulk_data_start_offset'], 'bulk start changed')
    marker = b'PF_DXT5\0'
    require(export.count(marker) == 1, 'ambiguous DXT5 frame')
    position = export.index(marker)
    require(position-16 == row['platform_header_offset'] and struct.unpack_from('<IIIi',export,position-16) == (dim,dim,1,len(marker)), 'logical platform dimensions changed')
    cursor = position+len(marker)
    require(struct.unpack_from('<ii',export,cursor) == (0,count) and len(row['mips']) == count, 'mip count changed')
    cursor += 8
    result, end = [], 0
    for level, mip in enumerate(row['mips']):
        logical, physical = max(dim>>level,1), max(dim>>level,4)
        size = (physical//4)**2*16
        cooked,flags,elements,length,stored = struct.unpack_from('<iiiiq',export,cursor)
        cursor += 24
        require((cooked,flags,elements,length) == (1,0x501 if level<external else 0x48,size,size), 'mip frame changed')
        if level < external:
            offset = stored+start
            require(offset == end and offset+size <= len(bulk), 'external bulk extent changed')
            data = bulk[offset:offset+size]
            end += size
            storage = 'ubulk'
        else:
            offset = cursor
            require(stored == len(header)+cursor and cursor+size <= len(export), 'inline mip offset changed')
            data = export[cursor:cursor+size]
            cursor += size
            storage = 'uexp'
        require(struct.unpack_from('<iii',export,cursor) == (physical,physical,1), 'stored mip dimensions changed')
        cursor += 12
        actual = dict(level=level,logical_width=logical,logical_height=logical,stored_width=physical,stored_height=physical,
            depth=1,cooked=cooked,bulk_flags=flags,element_count=elements,compressed_bytes=size,stored_bulk_offset=stored,
            cue_reported_bulk_offset=stored+start,storage=storage,resolved_file_offset=offset,sha256=digest(data))
        require(actual == mip, 'mip receipt disagrees with original bytes')
        result.append(dict(actual,compressed_hex=data.hex()))
    require(end == len(bulk) and row['external_bulk_extent'] == [0,end], 'bulk coverage changed')
    require(row['terminal_offset'] == row['inline_frame_end'] == cursor and row['platform_virtual_flag'] is False,
            'platform terminal receipt changed')
    # The package is hash pinned. Validate terminal name index against its actual name map.
    name_length = struct.unpack_from('<i',header,28)[0]
    _, name_count, name_position = struct.unpack_from('<Iii',header,32+name_length)
    names=[]
    for _ in range(name_count):
        length=struct.unpack_from('<i',header,name_position)[0];name_position+=4
        require(0<length<=256 and header[name_position+length-1]==0,'invalid package name')
        names.append(header[name_position:name_position+length-1].decode());name_position+=length+4
    virtual,ni,number,tag=struct.unpack_from('<iiiI',export,cursor)
    require(virtual==0 and 0<=ni<len(names) and names[ni]=='None' and number==0 and tag==0x9e2a83c1 and cursor+16==len(export), 'platform terminal changed')
    return dict(texture_object=row['texture_object'], source_platform_data=copy.deepcopy(row), mips=result,
                effective_sampler=None, effective_srgb=None)



def create_checkpoint(inventory=INVENTORY, frame_receipt=Path('/tmp/tracking-llama-texture-frame-1/receipt.json'), proof_path=Path('/tmp/tracking-llama-texture-context-1/proof-verified.json')):
    require(digest(proof_path.read_bytes())=='9749b60eaf0490fc009260b7f4ca47eee5a5967b50264281273cd4b352b6cc4b','context proof changed')
    proof=read_json(proof_path); comic.verify_inputs(proof)
    expected_binding=dict(uniform_position=0,texture_index=0,layer_index=-1,page_table_layer_index=-1,sampler_source=0,virtual_texture=False,texture_object=TEXTURE)
    require(proof['owner']==PACKAGE and proof['ordered_texture_objects']==[TEXTURE] and proof['texture_2d_bindings']==[expected_binding] and proof['original_resource_sha256']==RESOURCE_HASH,'original texture context changed')
    require(digest(frame_receipt.read_bytes())=='0a04773f887a655bff04e6ebf78a6ed1f13e21da309848af20400c37f313d25f','texture frame changed')
    frame=read_json(frame_receipt); require(frame['schema']=='scratch.tracking-llama.original-texture-frame.v1' and len(frame['textures'])==1,'unexpected texture frame')
    row=frame['textures'][0]; require(row['compiled_uniforms']==[expected_binding], 'frame context mismatch')
    texture=validate_texture_frame(row,[Path(r['path']).read_bytes() for r in row['source_files']])
    stem=PACKAGE.replace('/','_'); framing=read_json(inventory/(stem+'.inline-resource.json')); decoded=read_json(inventory/(stem+'.decoded.json')); owner=read_json(inventory/(stem+'.json'))
    require(framing['package']==decoded['package']==owner['path']==PACKAGE,'owner identity changed')
    payload=bundle.validate_source((EXTRACTED/(PACKAGE+'.uexp')).read_bytes(),framing);require(digest(payload)==RESOURCE_HASH,'resource changed')
    parsed=bundle.decoder.parse_prefix(payload,framing['names']); archive_path=Path(decoded['shader_library']['path'])
    require(bundle.library_reader.sha256(archive_path)==ARCHIVE_SHA256==decoded['shader_library']['sha256'],'archive changed')
    codes=[]
    with bundle.library_reader.ShaderArchive(archive_path,layout='fortnite-12.60') as archive:
        bundle.decoder.associate_library(parsed,archive)
        for record in bundle.all_records(parsed):
            index=record['library_association']['index']; code=archive.shader_code(index)
            codes.append(dict(type=record['type'],library_index=index,frequency=record['frequency'],sha256=digest(code),bytes=len(code),wrapper_hex=code.hex()))
    for key,value in parsed.items():
        if key=='original_serializer_evidence':require(all(decoded[key].get(k)==v for k,v in value.items()),'serializer evidence changed')
        else:require(decoded.get(key)==value,'fresh original parse changed: '+key)
    contract=numeric_contract(parsed)
    item=dict(owner_path=numeric.package_path(PACKAGE)+'.M_UI_TrackingLlama',parent_path='',resource_raw_hex=payload.hex(),decoded=decoded,owner_report=owner,framing=framing,expected_layout=LAYOUT,ordered_texture_references=[numeric.package_path(TEXTURE)],texture_context_proof=proof,numeric_contract=contract,original_shader_code=codes,slate_programs=copy.deepcopy(parsed['material_shaders']['records']))
    return dict(owners=[item],textures=[texture])

def typed_contract(record, reflected):
    """Distinguish compiled Material resource-table entries from loose ElementTexture."""
    vertex = record['frequency']==0
    layout=LAYOUT
    expected_resources={} if vertex else {0:('srv',0,'texture',2),1:('sampler',0,'sampler',0)}
    font=record['type']['name'] in ('TSlateMaterialShaderPSGrayscaleFont','TSlateMaterialShaderPSColorFont')
    elements=1
    roles=[];resources=[];seen=set()
    image=dict(sampled_type=dict(kind='float',bits=32),dimension=1,depth=2,arrayed=0,multisampled=0,sampled=1,format=0)
    for desc in reflected['descriptors']:
        reg=desc['d3d_register'];entries=desc['resource_table_entries']
        require(reg['space']==0 and desc['set']==0,'unsupported descriptor space/set')
        if reg['kind']=='srv':require(desc.get('image_type')==image,'sampled-image type changed')
        if entries:
            require(len(entries)==1 and entries[0]['resource'] in expected_resources,'unexpected Material resource-table entry')
            entry=entries[0];resource=entry['resource'];kind,bind,table,dtype=expected_resources[resource]
            require(resource not in seen and entry==dict(uniform_buffer=1,resource=resource,bind=bind,table=table)
                and reg==dict(kind=kind,space=0,index=bind) and desc['vulkan_descriptor_type']==dtype
                and layout['resource_types'][resource]==(5 if kind=='srv' else 8),'typed Material resource-table disagreement')
            seen.add(resource);role='MaterialResource'+str(resource)
            resources.append(dict(resource_index=resource,byte_offset=layout['resource_offsets'][resource],d3d_register=reg,descriptor_binding=desc['binding']))
        elif reg['kind']=='cbv' and desc['vulkan_descriptor_type']==6:
            expected={0:(80 if vertex else 64,'SlateLooseConstants')}
            if not vertex:expected.update({1:(112,'Material')})
            require(reg['index'] in expected and desc['block_span_bytes']==expected[reg['index']][0],'constant-buffer contract changed')
            role=expected[reg['index']][1]
        else:
            require(font and reg['index']==elements and reg['kind'] in ('srv','sampler')
                and desc['vulkan_descriptor_type']==(2 if reg['kind']=='srv' else 0),'unclassified loose texture descriptor')
            role='ElementTexture' if reg['kind']=='srv' else 'ElementTextureSampler'
        roles.append(role)
    expected_roles={'SlateLooseConstants'} if vertex else {'SlateLooseConstants','Material'}|{'MaterialResource'+str(i) for i in expected_resources}
    if font:expected_roles|={'ElementTexture','ElementTextureSampler'}
    require(seen==set(expected_resources) and len(roles)==len(set(roles)) and set(roles)==expected_roles,'incomplete/duplicate typed descriptor roles')
    return dict(roles=roles,material_resources=resources,requires_view=False)



def create_bundle(inventory=INVENTORY, frame_receipt=Path('/tmp/tracking-llama-texture-frame-1/receipt.json'), proof_path=Path('/tmp/tracking-llama-texture-context-1/proof-verified.json'), shader_directory=ROOT/'tracking-llama-shaders-1'):
    checkpoint=create_checkpoint(inventory,frame_receipt,proof_path)
    owner=checkpoint['owners'][0]; shaders,contracts=shader_evidence(checkpoint,shader_directory)[0]
    parsed=owner['decoded']; uniform=owner['numeric_contract']; binding=owner['texture_context_proof']['texture_2d_bindings'][0]
    expression=dict(expression_kind='Texture',parameter=None,texture_index=binding['texture_index'],texture_layer_index=-1,page_table_layer_index=-1,sampler_source=0,virtual_texture=False,default_texture_path=numeric.package_path(TEXTURE))
    fixture=dict(schema='fortnite.legacy-slate.v2',material={k:parsed[k] for k in ('friendly_name','material_path','quality_level','feature_level','cooked_shader_map_id_hash','debug_description')},expected_layout=copy.deepcopy(LAYOUT),**uniform,uniform_cases=copy.deepcopy(uniform['owner_uniform_cases']),shaders=shaders,texture_2d_parameters=[expression],ordered_texture_references=owner['ordered_texture_references'],texture_context_proof=owner['texture_context_proof'],shader_binding_contracts=contracts,texture_contract=dict(expression_kind='Texture',parameter=None,named_overrides_allowed=False,textures=checkpoint['textures'],scope='Original compressed mips and serialized properties; effective sampling defaults remain separately qualified'),provenance=dict(resource_sha256=RESOURCE_HASH,shader_library_sha256=ARCHIVE_SHA256))
    paths=[frame_receipt,proof_path,Path(__file__),shader_directory/'archive.json',shader_directory/'d3d.json',shader_directory/'mapped/mapped-translation-report.json',shader_directory/'reflection.json']
    paths += [resolve(r['path']) for r in owner['texture_context_proof']['inputs']]
    paths += [Path(r['path']) for r in checkpoint['textures'][0]['source_platform_data']['source_files']]
    paths += [inventory/(PACKAGE.replace('/','_')+suffix) for suffix in ('.inline-resource.json','.json')]
    return [dict(schema='fortnite.legacy-material-ingress.v2',owner_path=owner['owner_path'],owner_type='UMaterial',parent_path='',resource_owner_path=owner['owner_path'],shader_identity=parsed['material_path'],framing=owner['framing'],decoded=parsed,runtime_fixture=fixture,resource_raw_hex=owner['resource_raw_hex'],original_shader_code=owner['original_shader_code'],shader_binding_contract=contracts,provenance=dict(inputs=[source_receipt(p) for p in dict.fromkeys(paths)],resource_sha1=hashlib.sha1(bytes.fromhex(owner['resource_raw_hex'])).hexdigest(),shader_library=parsed['shader_library'],exporter_sha256=digest(Path(__file__).read_bytes())),limitations=['Offline source conversion only; runtime attachment, draw and original pixel parity unverified','Effective original sampling defaults remain unproven'])]

def write_bundle(output, data):
    require(GENERATED.resolve() in output.resolve().parents and not output.exists(),'choose fresh generated output directory')
    output.mkdir(parents=True); item=data[0]; path=output/'M_UI_TrackingLlama.json'
    raw=(json.dumps(item,indent=2,allow_nan=False)+'\n').encode();path.write_bytes(raw)
    entry=dict(owner_path=item['owner_path'],owner_type=item['owner_type'],parent_path='',resource_owner_path=item['resource_owner_path'],shader_identity=item['shader_identity'],resource_sha1=item['provenance']['resource_sha1'],resource_sha256=RESOURCE_HASH,bundle_path=str(path.resolve()),bundle_sha1=hashlib.sha1(raw).hexdigest(),bundle_sha256=digest(raw),bundle_schema=item['schema'],requires_view=False)
    manifest=output/'manifest.json';manifest.write_text(json.dumps(dict(schema='fortnite.legacy-material-manifest.v1',entries=[entry],limitations=['Offline source conversion; runtime validation remains required']),indent=2)+'\n')
    return dict(path=str(manifest.resolve()),sha256=digest(manifest.read_bytes()),owners=1)

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--inventory',type=Path,default=INVENTORY);parser.add_argument('--frame-receipt',type=Path,default=Path('/tmp/tracking-llama-texture-frame-1/receipt.json'));parser.add_argument('--proof',type=Path,default=Path('/tmp/tracking-llama-texture-context-1/proof-verified.json'));parser.add_argument('--shader-directory',type=Path,default=ROOT/'tracking-llama-shaders-1');parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    try: print(json.dumps(write_bundle(args.output,create_bundle(args.inventory,args.frame_receipt,args.proof,args.shader_directory))))
    except (OSError,ValueError,KeyError,TypeError,IndexError,struct.error,subprocess.SubprocessError) as error:parser.exit(1,'error: '+str(error)+'\n')
if __name__=='__main__':main()
