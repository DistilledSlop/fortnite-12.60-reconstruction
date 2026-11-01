"""Validate observed ordinary Texture2D bindings against a pinned base-material fixture."""
import hashlib
from pathlib import Path


def require(value, message):
    if not value:
        raise ValueError('original texture bindings: ' + message)


def source_object(path):
    require(isinstance(path, str) and path.startswith('/Game/') and path.count('.') == 1
            and '..' not in path and '/' not in path.split('.')[1], 'invalid canonical object path')
    return 'FortniteGame/Content/' + path[6:]


def verify(owner, fixture, receipt, layout_hash):
    proof = fixture['texture_context_proof']
    require(proof['schema'] == 'fortnite.original-material-texture-context-proof.v1'
            and proof['owner'] == source_object(owner).split('.')[0]
            and fixture['material']['material_path'] == owner
            and proof['original_resource_sha256'] == fixture['provenance']['resource_sha256'], 'source owner/resource differs')
    refs = fixture['ordered_texture_references']
    require(isinstance(refs, list) and 1 <= len(refs) <= 128
            and len(set(refs)) == len(refs)
            and [source_object(p) for p in refs] == proof['ordered_texture_objects'], 'ordered source references differ')
    contract = fixture['texture_contract']
    require(contract['expression_kind'] == 'Texture' and contract['parameter'] is None
            and contract['named_overrides_allowed'] is False, 'source texture kind differs')
    textures = contract['textures']
    require(len(textures) == len(refs)
            and [t['texture_object'] for t in textures] == proof['ordered_texture_objects'], 'source texture inventory differs')
    entries = fixture['texture_2d_parameters']
    require(1 <= len(entries) <= 128 and len(proof['texture_2d_bindings']) == len(entries), 'source binding count differs')
    numeric = 16 * len(fixture['vector_roots']) + 16 * ((len(fixture['scalar_roots']) + 3) // 4)
    types = [5, 8] * len(entries) + [8, 8]
    offsets = [numeric + 8 * i for i in range(len(types))]
    size = numeric + len(types) * 8
    require(size <= 8192 and fixture['expected_layout'] == dict(constant_bytes=size, resource_offsets=offsets, resource_types=types), 'source layout differs')
    require(receipt.get('schema') == 'fortnite.original-texture-bindings.v1'
            and receipt.get('owner_path') == owner and receipt.get('ordered_texture_references') == refs
            and receipt.get('constant_bytes') == size and receipt.get('static_slot') == 255
            and receipt.get('layout_hash') == layout_hash and receipt.get('resource_offsets') == offsets
            and receipt.get('resource_types') == types and type(receipt.get('named_proxy_calls')) is int
            and receipt['named_proxy_calls'] == 0 and receipt.get('shared_wrap_nonnull') is True
            and receipt.get('shared_clamp_nonnull') is True, 'observed layout/reference/sampler differs')
    rows = receipt.get('uniforms')
    require(isinstance(rows, list) and len(rows) == len(entries), 'observed binding count differs')
    metadata = []
    for path, texture in zip(refs, textures):
        source = texture['source_platform_data']
        width, height, mips, fmt = (source[k] for k in ('logical_width', 'logical_height', 'mip_count', 'pixel_format'))
        require(source['texture_object'] == source_object(path) and fmt in ('PF_DXT1', 'PF_DXT3', 'PF_DXT5')
                and all(type(v) is int and v > 0 for v in (width, height, mips))
                and max(width, height) <= 16384 and mips <= 15
                and source['first_mip'] == 0 and source['platform_virtual_flag'] is False, 'source platform data differs')
        require(type(source['inspector_metadata']['srgb']) is bool, 'source sRGB metadata missing')
        files = source['source_files']
        require(1 <= len(files) <= 3 and len({Path(f['path']).suffix for f in files}) == len(files)
                and {Path(f['path']).suffix for f in files} >= {'.uasset', '.uexp'}, 'source package set differs')
        blobs = {}
        stem = source_object(path).split('.')[0]
        for item in files:
            file = Path(item['path']); data = file.read_bytes()
            require(file.as_posix().endswith(stem + file.suffix) and file.suffix in ('.uasset', '.uexp', '.ubulk')
                    and len(data) == item['bytes'] and hashlib.sha256(data).hexdigest() == item['sha256'], 'source package changed')
            blobs[file.suffix] = data
        require(len(texture['mips']) == mips and len(source['mips']) == mips, 'source mip count differs')
        for level, (mip, original) in enumerate(zip(texture['mips'], source['mips'])):
            require({k: v for k, v in mip.items() if k != 'compressed_hex'} == original, 'source mip frame differs')
            data = bytes.fromhex(mip['compressed_hex'])
            w, h = max(width >> level, 1), max(height >> level, 1)
            length = ((max(w, 4) + 3) // 4) * ((max(h, 4) + 3) // 4) * (8 if fmt == 'PF_DXT1' else 16)
            offset = mip['resolved_file_offset']; storage = '.' + mip['storage']
            require(mip['level'] == level and (mip['logical_width'], mip['logical_height']) == (w, h)
                    and mip['compressed_bytes'] == len(data) == length and storage in ('.uexp', '.ubulk')
                    and type(offset) is int and offset >= 0 and storage in blobs
                    and blobs[storage][offset:offset + length] == data
                    and hashlib.sha256(data).hexdigest() == mip['sha256'], 'source compressed mip differs')
        metadata.append((width, height, mips, fmt, source['inspector_metadata']['srgb']))
    for i, (entry, binding, row) in enumerate(zip(entries, proof['texture_2d_bindings'], rows)):
        index, sampler = entry['texture_index'], entry['sampler_source']
        require(type(index) is int and 0 <= index < len(refs) and type(sampler) is int and sampler in (0, 1, 2)
                and entry['expression_kind'] == 'Texture' and entry['parameter'] is None
                and entry['texture_layer_index'] == entry['page_table_layer_index'] == -1
                and entry['virtual_texture'] is False and entry['default_texture_path'] == refs[index]
                and binding == dict(uniform_position=i, texture_index=index, layer_index=-1,
                    page_table_layer_index=-1, sampler_source=sampler, virtual_texture=False,
                    texture_object=source_object(refs[index])), 'source indexed expression differs')
        sampled = i * 2 + 1 if sampler == 0 else len(entries) * 2 + sampler - 1
        expected = (i, index, sampler, i * 2, i * 2 + 1, sampled)
        values = tuple(row.get(k) for k in ('uniform_index', 'texture_index', 'sampler_source',
            'texture_resource_index', 'paired_sampler_resource_index', 'sampled_sampler_resource_index'))
        width, height, mips, fmt, srgb = metadata[index]
        require(all(type(v) is int for v in values) and values == expected and row.get('texture_path') == refs[index]
                and row.get('pixel_format') == fmt and row.get('srgb') is srgb
                and (row.get('logical_width'), row.get('logical_height'), row.get('source_mips')) == (width, height, mips)
                and all(row.get(k) is True for k in ('gt_indexed_match', 'rt_indexed_match',
                    'texture_binding_match', 'sampler_binding_match', 'nonwhite_texture')), 'observed indexed texture differs')
        resident = row.get('resident_mips')
        require(type(resident) is int and 1 <= resident <= mips
                and type(row.get('rhi_width')) is int and type(row.get('rhi_height')) is int
                and row['rhi_width'] == max(width >> (mips - resident), 1)
                and row['rhi_height'] == max(height >> (mips - resident), 1), 'resident dimensions differ')
