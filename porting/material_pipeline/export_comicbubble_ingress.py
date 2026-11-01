#!/usr/bin/env python3
"""Export a faithful, offline ComicBubble parent/instance source checkpoint.

The default mode uses an offline checkpoint schema. --bundles additionally
revalidates fourteen exact mapped programs and View access, then emits two v2
ingress bundles and a generic manifest. Original wrappers and compressed mips
are retained without recooking. Neither mode establishes runtime acceptance.
"""
import argparse
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
GENERATED = HERE.parent / 'generated'
ROOT = GENERATED / 'material_pipeline'
EXTRACTED = Path('/mnt/vibrato/fortnite-extracted-full')
INVENTORY = GENERATED / 'frontend-probes/frontend-material-inventory-textures'
PACKAGE = 'FortniteGame/Content/Environments/FrontEnd/BackPlates/UITakeovers/M_UI_ComicBubble'
TEXTURES = [
    'FortniteGame/Content/Packages/Fortnite_UniqueMaterials_1/PostProcess/T_TilingClouds_02',
    'FortniteGame/Content/Environments/FrontEnd/BackPlates/UITakeovers/T_UI_Bubble_Tail',
]
ORDERED = [p + '.' + Path(p).name for p in TEXTURES]
PROOFS = [
    (ROOT / 'comicbubble-texture-context-proof-4.json', 'e7cf1f59b06e26f3ca4b5191c44cd86cfdd56c6e402c3d2f683001cb948a6a51'),
    (ROOT / 'comicbubble-instance-texture-context-proof-1.json', 'dfe83f55271429ad438fc661de89c029c8a21cfcdaf9828f47920e3e98130487'),
]
FRAME_SHA256 = '6d58f503f6ff875ae5920025ad81806224c8bb0364f0ff7e11f38d97b56cfdcc'
RESOURCE_HASHES = ['5872292505951dccccc70f3475916f104d9fac1cc6a07df1ce447a7d4c5f8767',
                   'fab8a4749b2b9b619eb3d4233bdf9b02de19486f1a400e9a578d38e666cbd655']
LAYOUTS = [dict(constant_bytes=896, resource_offsets=[848,856,864,872,880,888], resource_types=[5,8,5,8,8,8]),
           dict(constant_bytes=368, resource_offsets=[336,344,352,360], resource_types=[5,8,8,8])]
SLATE_NAMES = ['TSlateMaterialShaderVStrue', 'TSlateMaterialShaderVSfalse', 'TSlateMaterialShaderPSGrayscaleFont',
               'TSlateMaterialShaderPSDefault', 'TSlateMaterialShaderPSCustom', 'TSlateMaterialShaderPSColorFont', 'TSlateMaterialShaderPSBorder']
ARCHIVE_SHA256 = '2e795bd21c5e9bd69a6fdb520ef73f6fd70822c011c4232ce1ba08ccf10aef21'


def sibling(name):
    spec = importlib.util.spec_from_file_location(name, HERE / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


numeric = sibling('export_numeric_ingress')
bundle = numeric.bundle


def digest(raw):
    return hashlib.sha256(raw).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def read_json(path):
    raw = path.read_bytes()
    return json.loads(raw[raw.index(b'{'):])


def resolve(path):
    path = Path(path)
    return path if path.is_absolute() else REPO / path


def verify_inputs(report):
    for item in report['inputs']:
        require(digest(resolve(item['path']).read_bytes()) == item['sha256'], 'proof input SHA mismatch: ' + item['path'])


def validate_compiled(parsed, instance=False):
    """Validate owner-specific serialized layouts, exact Texture fields and seven programs."""
    index = int(instance)
    require(parsed['complete_serialized_shader_map'] and parsed['undecoded_bytes'] == 0
            and parsed['shader_record_count'] == 19 and len(bundle.all_records(parsed)) == 19,
            'incomplete 19-record original resource')
    require(parsed['resource_sha256'] == RESOURCE_HASHES[index], 'original resource SHA mismatch')
    arrays = parsed['uniform_expression_arrays']
    expected = [(2294,0,0)] if instance else [(7837,1,2),(7861,0,0)]
    expressions = [dict(offset=o, type='FMaterialUniformExpressionTexture', end=o+24,
        texture=dict(texture_index=t, layer_index=-1, page_table_layer_index=-1, sampler_source=s, virtual_texture=False))
        for o,t,s in expected]
    require(arrays['texture_2d']['count'] == len(expected) and arrays['texture_2d']['expressions'] == expressions,
            'original regular Texture contract changed')
    require(all(v['count'] == 0 and v['expressions'] == [] for k,v in arrays.items()
                if k.startswith('texture_') and k != 'texture_2d')
            and parsed['virtual_texture_stacks']['count'] == 0 and parsed['parameter_collections']['count'] == 0,
            'unsupported texture arrays, stacks or collections')
    counts = (16,20) if instance else (38,59)
    require(tuple(arrays[k]['count'] for k in ('vector','scalar')) == counts, 'numeric root counts changed')
    records = parsed['material_shaders']['records']
    require([r['type'] for r in records] == [dict(name=n, number=0) for n in SLATE_NAMES], 'original seven Slate programs changed')
    for record in records:
        require(record['debug_uniform_buffer_layout'] == LAYOUTS[index] and record['layout_name'] == dict(name='Material',number=0),
                'owner-specific Material layout changed')
        vertex = record['frequency'] == 0
        require(record['frequency'] == (0 if record['type']['name'].startswith('TSlateMaterialShaderVS') else 3), 'Slate stage changed')
        require(record['material_uniform_buffer'] == dict(base_index=0 if vertex else 2,bound=not vertex), 'Material UB binding changed')
        font = record['type']['name'] in ('TSlateMaterialShaderPSGrayscaleFont','TSlateMaterialShaderPSColorFont')
        slot = 1 if instance else 2
        maps = dict(uniform_buffers=[] if vertex else [[1,0],[2,0]], samplers=[[slot,1]] if font else [], srvs=[[slot,1]] if font else [])
        require(record['resource']['parameter_map'] == maps, 'original resource parameter map changed')
        params = [] if vertex else [dict(base_index=slot if font else 0, resource_count=int(font))]*2
        require(record['resource_parameters'] == params, 'original material resource registers changed')
        require(record['serialized_expression_counts'] == [*counts,len(expected),0,0,0,0], 'serialized expression counts changed')
    return bundle.fixture_exporter.flatten_expressions(arrays)


def validate_context(proof, instance=False):
    require(proof['owner'] == PACKAGE + ('_Inst' if instance else '') and proof['ordered_texture_objects'] == ORDERED,
            'ordered original texture references changed')
    expected = [(0,0)] if instance else [(1,2),(0,0)]
    bindings = [dict(uniform_position=p, texture_index=t, layer_index=-1, page_table_layer_index=-1,
                     sampler_source=s, virtual_texture=False, texture_object=ORDERED[t]) for p,(t,s) in enumerate(expected)]
    require(proof['texture_2d_bindings'] == bindings and proof['original_resource_sha256'] == RESOURCE_HASHES[int(instance)],
            'compiled/context texture binding disagreement')
    if instance:
        require(proof['instance_layer_entries'] == 0 and proof['layer_array_tag_present'] is False, 'unproven instance layer ordering')


def validate_texture_frame(row, blobs):
    """Re-observe logical/stored dimensions, signed bulk offsets and every original BC1 mip."""
    header, export, bulk = blobs
    require([digest(x) for x in blobs] == [r['sha256'] for r in row['source_files']], 'texture source SHA mismatch')
    require([len(x) for x in blobs] == [r['bytes'] for r in row['source_files']], 'texture source length mismatch')
    require(row['texture_object'] in ORDERED, 'unrecognized original texture')
    dim, count, external = (1024,11,4) if row['texture_object'] == ORDERED[0] else (256,9,2)
    require((row['logical_width'],row['logical_height'],row['mip_count'],row['pixel_format'],row['first_mip']) == (dim,dim,count,'PF_DXT1',0), 'logical texture contract changed')
    require(struct.unpack_from('<6i',header) == (-1641380927,-7,0,0,0,0), 'package summary changed')
    start = struct.unpack_from('<q',header,169)[0]
    require(start == len(header)+len(export)-4 == row['bulk_data_start_offset'], 'bulk start changed')
    marker = b'PF_DXT1\0'
    require(export.count(marker) == 1, 'ambiguous DXT1 frame')
    position = export.index(marker)
    require(position-16 == row['platform_header_offset'] and struct.unpack_from('<IIIi',export,position-16) == (dim,dim,1,len(marker)), 'logical platform dimensions changed')
    cursor = position+len(marker)
    require(struct.unpack_from('<ii',export,cursor) == (0,count) and len(row['mips']) == count, 'mip count changed')
    cursor += 8
    result, end = [], 0
    for level, mip in enumerate(row['mips']):
        logical, physical = max(dim>>level,1), max(dim>>level,4)
        size = (physical//4)**2*8
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


def validate_instance_properties(owner, original):
    require(original['path']==PACKAGE+'_Inst' and not original['skippedMetadata'] and len(original['objects'])==1,'original instance property identity changed')
    properties=original['objects'][0]['Properties']
    require(properties.get('TextureParameterValues',[])==[] and properties['StaticParameters'].get('MaterialLayersParameters',[])==[], 'unproven texture overrides or layers')
    require(properties['Parent']==dict(ObjectName="Material'M_UI_ComicBubble'",ObjectPath=PACKAGE+'.0') and properties['bHasStaticPermutationResource'] is True,'instance inheritance changed')
    values=owner['instanceParameters']
    require(len(values)==1 and values[0]['textures']==[],'instance overrides changed')
    for kind,plural in [('Scalar','scalars'),('Vector','vectors')]:
        expected=[]
        for value in properties[kind+'ParameterValues']:
            info=value['ParameterInfo']
            require(info['Association']=='GlobalParameter','unsupported original parameter association')
            parameter=dict(name=dict(name=info['Name'],number=0),association=2,index=info['Index'])
            raw=value['ParameterValue']
            expected.append(dict(parameter=info['Name'],parameterInfo=parameter,value=raw if kind=='Scalar' else [raw[c] for c in 'RGBA']))
        require(values[0][plural]==expected,'numeric overrides disagree with original instance properties')


def numeric_contract(parsed, owner, instance=False, use_owner_overrides=True):
    nodes, roots = validate_compiled(parsed, instance)
    inherited = []
    if instance and use_owner_overrides:
        params = owner['instanceParameters']
        require(len(params)==1 and params[0]['name']=='M_UI_ComicBubble_Inst' and params[0]['textures']==[], 'unexpected instance texture overrides')
        require(len(params[0]['scalars'])==2 and len(params[0]['vectors'])==4, 'incomplete original numeric overrides')
        for kind, plural in [('scalar','scalars'),('vector','vectors')]:
            for value in params[0][plural]:
                require(value['parameterInfo'] is not None, 'missing original ParameterInfo')
                inherited.append(dict(type=kind,parameter=copy.deepcopy(value['parameterInfo']),value=copy.deepcopy(value['value'])))
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


def create_checkpoint(inventory=INVENTORY, frame_receipt=Path('/tmp/comicbubble-texture-framing-1/receipt.json')):
    proofs=[]
    for path,expected in PROOFS:
        require(digest(path.read_bytes())==expected,'unpinned context proof')
        proof=read_json(path);verify_inputs(proof);validate_context(proof,bool(proofs));proofs.append(proof)
    require(digest(frame_receipt.read_bytes())==FRAME_SHA256,'unpinned original texture framing proof')
    frame=read_json(frame_receipt)
    require([r['texture_object'] for r in frame['textures']]==ORDERED,'texture frame ordering changed')
    textures=[validate_texture_frame(row,[Path(r['path']).read_bytes() for r in row['source_files']]) for row in frame['textures']]
    owners=[]
    for index,proof in enumerate(proofs):
        package=proof['owner'];stem=package.replace('/','_')
        framing=read_json(inventory/(stem+'.inline-resource.json'))
        decoded=read_json(inventory/(stem+'.decoded.json'))
        owner=read_json(inventory/(stem+'.json'))
        raw=(EXTRACTED/(package+'.uexp')).read_bytes()
        require(framing['package']==decoded['package']==owner['path']==package,'owner source identity changed')
        payload=bundle.validate_source(raw,framing)
        require(digest(payload)==RESOURCE_HASHES[index],'original material resource changed')
        parsed=bundle.decoder.parse_prefix(payload,framing['names'])
        archive_path=Path(decoded['shader_library']['path'])
        require(bundle.library_reader.sha256(archive_path)==ARCHIVE_SHA256==decoded['shader_library']['sha256'],'original shader archive changed')
        codes=[]
        with bundle.library_reader.ShaderArchive(archive_path,layout='fortnite-12.60') as archive:
            bundle.decoder.associate_library(parsed,archive)
            for record in bundle.all_records(parsed):
                shader_index=record['library_association']['index'];code=archive.shader_code(shader_index)
                codes.append(dict(type=record['type'],library_index=shader_index,frequency=record['frequency'],sha256=digest(code),bytes=len(code),wrapper_hex=code.hex()))
        for key,value in parsed.items():
            if key=='original_serializer_evidence':
                require(all(decoded[key].get(k)==v for k,v in value.items()),'serializer evidence changed')
            else:
                require(decoded.get(key)==value,'fresh original parse changed: '+key)
        if index:
            properties_path=next(resolve(r['path']) for r in proof['inputs'] if 'instance-properties' in r['path'])
            validate_instance_properties(owner,read_json(properties_path))
        contract=numeric_contract(parsed,owner,bool(index))
        owners.append(dict(owner_path=numeric.package_path(package)+'.'+Path(package).name,
            parent_path=numeric.package_path(PACKAGE)+'.M_UI_ComicBubble' if index else '',
            resource_raw_hex=payload.hex(),decoded=decoded,owner_report=owner,framing=framing,
            expected_layout=LAYOUTS[index],ordered_texture_references=[numeric.package_path(p) for p in ORDERED],
            texture_context_proof=proof,numeric_contract=contract,original_shader_code=codes,
            slate_programs=copy.deepcopy(parsed['material_shaders']['records'])))
    return dict(schema='fortnite.comicbubble-source-checkpoint.v1',runtime_ready=False,owners=owners,textures=textures,
        required_before_runtime=['Seven exact mapped and reflected Slate programs per owner with original D3D resource-table provenance',
            'Proven effective original sRGB and sampler settings for source-zero Clouds and source-two Tail',
            'Reviewed runtime provider contract for per-owner multi-texture resources and inherited two-entry references'],
        limitations=['Offline source checkpoint only; no runtime fixture, native promotion, GPU resource, draw or parity claim',
            'Compressed original DXT1 mips and original shader wrappers retained; no resizing, recook, substituted program or invented descriptor map'])


def typed_contract(record, reflected, instance=False):
    """Distinguish compiled Material resource-table entries from loose ElementTexture."""
    vertex = record['frequency']==0
    layout=LAYOUTS[int(instance)]
    expected_resources={} if vertex else ({0:('srv',0,'texture',2),1:('sampler',0,'sampler',0)} if instance else
        {0:('srv',0,'texture',2),2:('srv',1,'texture',2),3:('sampler',0,'sampler',0),5:('sampler',1,'sampler',0)})
    font=record['type']['name'] in ('TSlateMaterialShaderPSGrayscaleFont','TSlateMaterialShaderPSColorFont')
    elements=1 if instance else 2
    roles=[];resources=[];seen=set()
    image=dict(sampled_type=dict(kind='float',bits=32),dimension=1,depth=2,arrayed=0,multisampled=0,sampled=1,format=0)
    for desc in reflected['descriptors']:
        reg=desc['d3d_register'];entries=desc['resource_table_entries']
        require(reg['space']==0 and desc['set']==0,'unsupported descriptor space/set')
        if reg['kind']=='srv':require(desc.get('image_type')==image,'sampled-image type changed')
        if entries:
            require(len(entries)==1 and entries[0]['resource'] in expected_resources,'unexpected Material resource-table entry')
            entry=entries[0];resource=entry['resource'];kind,bind,table,dtype=expected_resources[resource]
            require(resource not in seen and entry==dict(uniform_buffer=2,resource=resource,bind=bind,table=table)
                and reg==dict(kind=kind,space=0,index=bind) and desc['vulkan_descriptor_type']==dtype
                and layout['resource_types'][resource]==(5 if kind=='srv' else 8),'typed Material resource-table disagreement')
            seen.add(resource);role='MaterialResource'+str(resource)
            resources.append(dict(resource_index=resource,byte_offset=layout['resource_offsets'][resource],d3d_register=reg,descriptor_binding=desc['binding']))
        elif reg['kind']=='cbv' and desc['vulkan_descriptor_type']==6:
            expected={0:(80 if vertex else 64,'SlateLooseConstants')}
            if not vertex:expected.update({1:(2272,'View'),2:(336 if instance else 848,'Material')})
            require(reg['index'] in expected and desc['block_span_bytes']==expected[reg['index']][0],'constant-buffer contract changed')
            role=expected[reg['index']][1]
        else:
            require(font and reg['index']==elements and reg['kind'] in ('srv','sampler')
                and desc['vulkan_descriptor_type']==(2 if reg['kind']=='srv' else 0),'unclassified loose texture descriptor')
            role='ElementTexture' if reg['kind']=='srv' else 'ElementTextureSampler'
        roles.append(role)
    expected_roles={'SlateLooseConstants'} if vertex else {'SlateLooseConstants','View','Material'}|{'MaterialResource'+str(i) for i in expected_resources}
    if font:expected_roles|={'ElementTexture','ElementTextureSampler'}
    require(seen==set(expected_resources) and len(roles)==len(set(roles)) and set(roles)==expected_roles,'incomplete/duplicate typed descriptor roles')
    return dict(roles=roles,material_resources=resources,requires_view=not vertex)


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
    require(len(expected)==14 and all(set(t)==expected for t in tables),'incomplete/extra original fourteen Slate selection')
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
            contract=typed_contract(record,reflection,bool(instance));contracts.append(dict(shader_type=record['type'],library_index=index,**contract))
            shaders.append(dict(record=record,mapped=translated,reflection=reflection,spirv_sha1=hashlib.sha1(code).hexdigest()))
        result.append((shaders,contracts))
    return result


def source_receipt(path):
    path=Path(path);raw=path.read_bytes()
    return dict(path=str(path.resolve()),bytes=len(raw),sha256=digest(raw))


def create_bundle(inventory=INVENTORY, frame_receipt=Path('/tmp/comicbubble-texture-framing-1/receipt.json'), shader_directory=ROOT/'comicbubble-shaders-1'):
    checkpoint=create_checkpoint(inventory,frame_receipt)
    evidence=shader_evidence(checkpoint,shader_directory)
    view_tool=sibling('export_frontend_numeric_batch')
    view=view_tool.load_view_evidence(shader_directory/'view-access.json',ROOT/'view-identity/evidence.json')
    results=[]
    for index,(owner,(shaders,contracts)) in enumerate(zip(checkpoint['owners'],evidence)):
        parsed=owner['decoded'];uniforms=owner['numeric_contract'];expressions=[]
        for shader in shaders:
            if shader['record']['frequency']==3:view_tool.verify_view_shader(shader['reflection'],view)
        for binding in owner['texture_context_proof']['texture_2d_bindings']:
            expressions.append(dict(expression_kind='Texture',parameter=None,texture_index=binding['texture_index'],texture_layer_index=-1,
                page_table_layer_index=-1,sampler_source=binding['sampler_source'],virtual_texture=False,
                default_texture_path=numeric.package_path(binding['texture_object'])))
        fixture=dict(schema='fortnite.legacy-slate.v2',material={k:parsed[k] for k in ('friendly_name','material_path','quality_level','feature_level','cooked_shader_map_id_hash','debug_description')},
            expected_layout=copy.deepcopy(owner['expected_layout']),**uniforms,uniform_cases=numeric_contract(parsed,owner['owner_report'],bool(index),False)['owner_uniform_cases'],
            shaders=shaders,texture_2d_parameters=expressions,ordered_texture_references=owner['ordered_texture_references'],
            texture_context_proof=owner['texture_context_proof'],shader_binding_contracts=contracts,
            texture_contract=dict(expression_kind='Texture',parameter=None,named_overrides_allowed=False,textures=checkpoint['textures'],
                scope='Original compressed mips and serialized properties; effective sampling defaults remain separately qualified'),
            provenance=dict(resource_sha256=parsed['resource_sha256'],shader_library_sha256=ARCHIVE_SHA256))
        fixture['view_contract']=copy.deepcopy(view['contract'])
        fixture['view_contract']['shader_access_records']=[view['records'][x['record']['library_association']['index']] for x in shaders if x['record']['frequency']==3]
        paths=[shader_directory/'view-access.json',ROOT/'view-identity/evidence.json',frame_receipt,Path(__file__)] + [p for p,_ in PROOFS]
        paths += [shader_directory/n for n in ('archive.json','d3d.json','mapped/mapped-translation-report.json','reflection.json')]
        paths += [resolve(r['path']) for proof in [o['texture_context_proof'] for o in checkpoint['owners']] for r in proof['inputs']]
        paths += [Path(r['path']) for texture in checkpoint['textures'] for r in texture['source_platform_data']['source_files']]
        paths += [inventory/(owner['texture_context_proof']['owner'].replace('/','_')+suffix) for suffix in ('.inline-resource.json','.json')]
        results.append(dict(schema='fortnite.legacy-material-ingress.v2',owner_path=owner['owner_path'],owner_type='UMaterialInstanceConstant' if index else 'UMaterial',
            parent_path=owner['parent_path'],resource_owner_path=owner['owner_path'],shader_identity=parsed['material_path'],
            framing=owner['framing'],decoded=parsed,runtime_fixture=fixture,resource_raw_hex=owner['resource_raw_hex'],
            original_shader_code=owner['original_shader_code'],shader_binding_contract=contracts,
            provenance=dict(inputs=[source_receipt(p) for p in dict.fromkeys(paths)],resource_sha1=hashlib.sha1(bytes.fromhex(owner['resource_raw_hex'])).hexdigest(),
                shader_library=parsed['shader_library'],exporter_sha256=digest(Path(__file__).read_bytes())),
            limitations=['Offline faithful source conversion only; runtime attachment, draw and original pixel parity unverified',
                'Cloud address defaults and effective source sampling still require independent runtime evidence; no guessed defaults supplied']))
    return results


def write_bundles(output, data):
    require(GENERATED.resolve() in output.resolve().parents and not output.exists(),'choose fresh porting/generated output directory')
    output.mkdir(parents=True)
    entries=[]
    for item in data:
        path=output/(Path(item['owner_path'].split('.')[-1]).name+'.json')
        raw=(json.dumps(item,indent=2,allow_nan=False)+'\n').encode();path.write_bytes(raw)
        entries.append(dict(owner_path=item['owner_path'],owner_type=item['owner_type'],parent_path=item['parent_path'],
            resource_owner_path=item['resource_owner_path'],shader_identity=item['shader_identity'],resource_sha1=item['provenance']['resource_sha1'],
            resource_sha256=item['decoded']['resource_sha256'],bundle_path=str(path.resolve()),bundle_sha1=hashlib.sha1(raw).hexdigest(),bundle_sha256=digest(raw),
            bundle_schema=item['schema'],requires_view=True))
    manifest=dict(schema='fortnite.legacy-material-manifest.v1',entries=entries,limitations=['Offline source conversion; runtime validation remains required'])
    path=output/'manifest.json';path.write_text(json.dumps(manifest,indent=2)+'\n')
    return dict(path=str(path.resolve()),sha256=digest(path.read_bytes()),owners=len(entries))


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--inventory',type=Path,default=INVENTORY)
    parser.add_argument('--frame-receipt',type=Path,default=Path('/tmp/comicbubble-texture-framing-1/receipt.json'))
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--bundles',action='store_true',help='write two v2 ingress bundles and manifest')
    parser.add_argument('--shader-directory',type=Path,default=ROOT/'comicbubble-shaders-1')
    args=parser.parse_args()
    try:
        if args.bundles:
            print(json.dumps(write_bundles(args.output,create_bundle(args.inventory,args.frame_receipt,args.shader_directory))))
            return
        require(GENERATED.resolve() in args.output.resolve().parents and not args.output.exists(),'choose a fresh porting/generated output')
        data=create_checkpoint(args.inventory,args.frame_receipt)
        raw=(json.dumps(data,indent=2,allow_nan=False)+'\n').encode()
        args.output.parent.mkdir(parents=True,exist_ok=True)
        with args.output.open('xb') as stream:stream.write(raw)
        print(json.dumps(dict(path=str(args.output.resolve()),sha256=digest(raw),owners=len(data['owners']),runtime_ready=False)))
    except (OSError,ValueError,KeyError,TypeError,IndexError,struct.error,subprocess.SubprocessError) as error:
        parser.exit(1,'error: '+str(error)+'\n')


if __name__=='__main__':
    main()
