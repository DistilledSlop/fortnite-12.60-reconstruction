"""Original pre-freeze material/LocalVF shader records; no payload skipping.

Runtime dump SHA256 604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d:
FMaterialShader::Serialize 0x1434f9590; Slate VS 0x143cfa820;
Slate PS 0x143cfb4e0; FShader::SerializeBase 0x143aaae20;
FShaderResource::Serialize 0x143aaab80. Public 4.23 Shader.cpp,
ShaderParameters.h/.cpp and ShaderBaseClasses.cpp supply the field names.
The original PS has THREE scalar parameters, unlike public 4.23's two.
"""

SLATE_VS = {'TSlateMaterialShaderVStrue', 'TSlateMaterialShaderVSfalse'}
SLATE_PS = {'TSlateMaterialShaderPS' + suffix for suffix in
            ('GrayscaleFont', 'Default', 'Custom', 'ColorFont', 'Border')}
MESH_VS = {'TTranslucencyShadowDepthVS<' + mode + '>' for mode in
           ('TranslucencyShadowDepth_Standard', 'TranslucencyShadowDepth_PerspectiveCorrect')}
MESH_VS |= {'TBasePassVSFSimpleNoLightmapLightingPolicy', 'TBasePassVSFNoLightMapPolicy',
            'FConvertToUniformMeshVS', 'FHitProxyVS'}
MESH_PS = {'TTranslucencyShadowDepthPS<' + mode + '>' for mode in
           ('TranslucencyShadowDepth_Standard', 'TranslucencyShadowDepth_PerspectiveCorrect')}
MESH_PS |= {'TBasePassPS' + policy + sky for policy in
            ('FSimpleNoLightmapLightingPolicy', 'FNoLightMapPolicy') for sky in ('', 'Skylight')}
MESH_PS.add('FHitProxyPS')
MESH_GS = {'FConvertToUniformMeshGS'}


def hash_field(reader):
    offset = reader.cursor
    return {'offset': offset, 'hash': bytes(reader.read('20B')).hex()}


def uniform_buffer(reader):
    base, bound = reader.read('HI')
    if bound not in (0, 1):
        raise ValueError('invalid shader uniform-buffer bool')
    return {'base_index': base, 'bound': bool(bound)}


def parameters(reader, resource=False):
    fields = ('base_index', 'resource_count') if resource else ('base_index', 'byte_count', 'buffer_index')
    return dict(zip(fields, reader.read('HH' if resource else 'HHH')))


def array(reader, fmt):
    return [reader.read(fmt) for _ in range(reader.count())]


def shader_record(reader, map_hash, expression_counts, vertex_factory=None):
    start = reader.cursor
    shader_type = reader.name()
    kind = shader_type['name']
    supported = SLATE_VS | SLATE_PS if vertex_factory is None else MESH_VS | MESH_PS | MESH_GS
    if shader_type['number'] or kind not in supported:
        raise ValueError(f'unsupported material shader type {kind} at {start}')
    end = reader.read('q')
    if not reader.cursor < end <= reader.limit:
        raise ValueError(f'invalid shader end offset at {start + 8}')
    old_limit, reader.limit = reader.limit, end
    try:
        result = {'offset': start, 'type': shader_type, 'serialized_end_offset': end,
                  'body_offset': reader.cursor}
        result['scene_textures_uniform_buffer'] = uniform_buffer(reader)
        result['material_uniform_buffer'] = uniform_buffer(reader)
        result['collection_uniform_buffers'] = [uniform_buffer(reader) for _ in range(reader.count())]
        counts = [reader.count() for _ in range(7)]
        if counts != expression_counts:
            raise ValueError('shader debug expression counts disagree with material')
        result['serialized_expression_counts'] = counts
        result['layout_name'] = reader.name()
        if result['layout_name'] != {'name': 'Material', 'number': 0}:
            raise ValueError('unexpected material shader buffer layout name')
        size = reader.read('I')
        offsets, types = array(reader, 'H'), array(reader, 'B')
        if size > 65536 or len(offsets) != len(types) or any(x + 8 > size for x in offsets):
            raise ValueError('invalid shader debug uniform-buffer layout')
        result['debug_uniform_buffer_layout'] = {'constant_bytes': size,
                                                'resource_offsets': offsets, 'resource_types': types}
        result['debug_description'] = reader.string()
        result['vt_feedback_buffer'] = parameters(reader, resource=True)
        if vertex_factory is None:
            result['scalar_parameters'] = [parameters(reader) for _ in range(2 if kind in SLATE_VS else 3)]
            result['resource_parameters'] = [parameters(reader, resource=True) for _ in range(0 if kind in SLATE_VS else 2)]
        else:
            # FMeshMaterialShader::Serialize 0x1434f99c0 and
            # FVertexFactoryParameterRef serializer 0x143a7b010.
            result['pass_uniform_buffer'] = uniform_buffer(reader)
            vf = {'offset': reader.cursor, 'type': reader.name(),
                  'frequency': reader.read('B'), 'platform': reader.read('B'),
                  'source_hash': hash_field(reader), 'serialized_end_offset': reader.read('q')}
            expected_frequency = 0 if kind in MESH_VS else 3 if kind in MESH_PS else 4
            if vf['type'] != vertex_factory or vf['frequency'] != expected_frequency or vf['platform'] != 0:
                raise ValueError('vertex-factory parameter identity/target mismatch')
            vf_end = vf['serialized_end_offset']
            if not reader.cursor <= vf_end <= reader.limit:
                raise ValueError('invalid vertex-factory parameter end offset')
            record_limit, reader.limit = reader.limit, vf_end
            try:
                # Original LocalVF constructor 0x1442f01e0 supplies parameters
                # for VS, compute and ray hit group; the latter two are outside
                # this verified material grammar. PS/GS return nullptr.
                if expected_frequency == 0:
                    speedtree = reader.read('I')
                    if speedtree not in (0, 1):
                        raise ValueError('invalid LocalVF speedtree bool')
                    vf['any_speedtree_parameter_bound'] = bool(speedtree)
                    vf['lod_parameter'] = parameters(reader)
                if reader.cursor != vf_end:
                    raise ValueError('vertex-factory parameters do not end at declared offset')
            finally:
                reader.limit = record_limit
            vf['end'] = reader.cursor
            result['vertex_factory_parameters'] = vf
            if kind.startswith('TBasePass'):
                # All six observed base-pass types share 0x1431e2ea0.
                result['base_pass_uniform_buffers'] = [uniform_buffer(reader) for _ in range(4)]
            elif kind.startswith('TTranslucencyShadowDepthPS'):
                # Original 0x143237540 adds one scalar FShaderParameter.
                result['shadow_depth_parameter'] = parameters(reader)
            elif kind == 'FHitProxyVS':
                # Paper2D's original record requires four bytes here. The
                # current FHitProxyVS declares a resource parameter at this
                # position; its original semantic identity is not verified.
                result['hit_proxy_resource_parameter'] = parameters(reader, resource=True)
            elif kind == 'FHitProxyPS':
                # The original record requires six bytes here, consistent
                # with the current FHitProxyPS scalar parameter declaration.
                result['hit_proxy_scalar_parameter'] = parameters(reader)
        result['output_hash'] = hash_field(reader)
        result['material_shader_map_hash'] = hash_field(reader)
        if result['material_shader_map_hash']['hash'] != map_hash:
            raise ValueError('shader material hash disagrees with shader-map identity')
        result['pipeline'], result['vertex_factory'] = reader.name(), reader.name()
        if vertex_factory is not None and result['vertex_factory'] != vertex_factory:
            raise ValueError('shader vertex factory disagrees with containing mesh map')
        result['vertex_factory_source_hash'] = hash_field(reader)
        serialized_type = reader.name()
        if serialized_type != shader_type:
            raise ValueError('shader type disagrees with record envelope')
        result['permutation_id'] = reader.read('i')
        result['source_hash'] = hash_field(reader)
        result['frequency'], platform = reader.read('II')
        expected_frequency = 0 if kind in SLATE_VS | MESH_VS else 3 if kind in SLATE_PS | MESH_PS else 4
        if result['frequency'] != expected_frequency or platform != 0:
            raise ValueError('unexpected original material shader target')
        result['uniform_parameters'] = [
            {'struct_name': reader.name(), 'binding': uniform_buffer(reader)} for _ in range(reader.count())]
        resource = {'specific_type': reader.name(), 'specific_permutation_id': reader.read('i')}
        resource['frequency'], resource['platform'] = reader.read('II')
        resource['output_hash'] = hash_field(reader)
        if (resource['output_hash']['hash'] != result['output_hash']['hash']
                or resource['frequency'] != result['frequency'] or resource['platform'] != 0):
            raise ValueError('inline shader resource identity disagrees with shader')
        resource['instruction_count'] = reader.read('I')
        resource['parameter_map'] = {name: array(reader, 'HH') for name in ('uniform_buffers', 'samplers', 'srvs')}
        resource['loose_parameter_buffers'] = [
            {'buffer_index_and_size': reader.read('HH'), 'parameters': array(reader, 'HH')}
            for _ in range(reader.count())]
        resource['uncompressed_code_bytes'] = reader.read('I')
        if resource['uncompressed_code_bytes'] > 64 * 1024 * 1024:
            raise ValueError('shader resource code exceeds size bound')
        if reader.read('I') != 1:
            raise ValueError('only shared-library original shader resources are supported')
        resource['code_in_shared_library'] = True
        result['resource'] = resource
        # Exact original SerializeBase tail: three arrays, uint32, uint16.
        # Nonempty binding arrays require their own verified element grammar.
        binding_counts = [reader.count() for _ in range(3)]
        if any(binding_counts):
            raise ValueError('unsupported nonempty shader parameter-binding arrays')
        result['parameter_bindings'] = {'array_counts': binding_counts,
                                       'serialized_word': reader.read('I'),
                                       'root_parameter_buffer_index': reader.read('H')}
        if reader.cursor != end:
            raise ValueError(f'shader record ended at {reader.cursor}, declared {end}')
        result['end'] = reader.cursor
        return result
    finally:
        reader.limit = old_limit


def material_shader_records(reader, map_hash, expression_counts):
    start = reader.cursor
    count = reader.count()
    records = [shader_record(reader, map_hash, expression_counts) for _ in range(count)]
    pipeline_count = reader.count()
    if pipeline_count:
        raise ValueError('unsupported nonempty material shader pipelines')
    mesh_count_offset = reader.cursor
    mesh_count = reader.count()
    return {'offset': start, 'count': count, 'records': records, 'pipeline_count': pipeline_count,
            'mesh_shader_maps_count_offset': mesh_count_offset, 'mesh_shader_maps_count': mesh_count,
            'end': reader.cursor}


def mesh_shader_maps(reader, count, map_hash, expression_counts):
    maps = []
    for _ in range(count):
        start = reader.cursor
        vf = reader.name()
        if vf != {'name': 'FLocalVertexFactory', 'number': 0}:
            raise ValueError('unsupported mesh shader-map vertex factory')
        shader_count = reader.count()
        records = [shader_record(reader, map_hash, expression_counts, vf) for _ in range(shader_count)]
        pipeline_count = reader.count()
        if pipeline_count:
            raise ValueError('unsupported nonempty mesh shader pipelines')
        maps.append({'offset': start, 'vertex_factory': vf, 'count': shader_count,
                     'records': records, 'pipeline_count': pipeline_count, 'end': reader.cursor})
    return maps
