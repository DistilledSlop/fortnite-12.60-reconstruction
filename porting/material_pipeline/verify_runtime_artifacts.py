#!/usr/bin/env python3
"""Independently verify original Slate modules and UE Vulkan payload artifacts."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import zlib

HERE = Path(__file__).resolve().parent
GENERATED = HERE.parent / 'generated'
spec = importlib.util.spec_from_file_location('spirv_reflection', HERE / 'reflect_mapped_spirv.py')
reflection = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reflection)
spec = importlib.util.spec_from_file_location('original_texture_bindings', HERE / 'verify_original_texture_bindings.py')
original_textures = importlib.util.module_from_spec(spec)
spec.loader.exec_module(original_textures)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def uniform_layout_hash(size, offsets, types, static_slot=255):
    """FRHIUniformBufferLayout::ComputeHash, including reverse packed type groups."""
    if (not 0 < size <= 65536 or not 0 <= static_slot <= 255 or len(offsets) != len(types)
            or any(not 0 <= value <= 255 for value in types)
            or any(offset < 0 or offset % 8 or offset + 8 > size for offset in offsets)):
        raise ValueError('invalid uniform layout shape')
    result = ((size << 16) | static_slot) & 0xffffffff
    for offset in offsets:
        result ^= offset
    remaining = len(types)
    while remaining >= 4:
        for shift in (0, 8, 16, 24):
            remaining -= 1
            result ^= types[remaining] << shift
    if remaining >= 2:
        remaining -= 1
        result ^= types[remaining]
        remaining -= 1
        result ^= types[remaining] << 16
    if remaining:
        result ^= types[0]
    return result & 0xffffffff



COMIC_PREFIX = '/Game/Environments/FrontEnd/BackPlates/UITakeovers/'
COMIC_OWNERS = {
    COMIC_PREFIX + 'M_UI_ComicBubble.M_UI_ComicBubble': (896, [848, 856, 864, 872, 880, 888],
        [5, 8, 5, 8, 8, 8], [(0, 1, 2, 0, 1, 5), (1, 0, 0, 2, 3, 3)]),
    COMIC_PREFIX + 'M_UI_ComicBubble_Inst.M_UI_ComicBubble_Inst': (368, [336, 344, 352, 360],
        [5, 8, 8, 8], [(0, 0, 0, 0, 1, 1)]),
}
COMIC_TEXTURES = [
    ('/Game/Packages/Fortnite_UniqueMaterials_1/PostProcess/T_TilingClouds_02.T_TilingClouds_02', 1024, 1024, 11, True),
    (COMIC_PREFIX + 'T_UI_Bubble_Tail.T_UI_Bubble_Tail', 256, 256, 9, False),
]


def verify_comic_texture_bindings(owner, fixture, receipt):
    """Require source-indexed references and observed GT/RT/RHI bindings for this owner."""
    if owner not in COMIC_OWNERS:
        raise ValueError('unsupported multi-texture owner')
    size, offsets, types, indices = COMIC_OWNERS[owner]
    layout = fixture['expected_layout']
    if layout != dict(constant_bytes=size, resource_offsets=offsets, resource_types=types):
        raise ValueError('ComicBubble source material layout differs')
    paths = [item[0] for item in COMIC_TEXTURES]
    if (receipt.get('schema') != 'fortnite.comicbubble-texture-bindings.v1' or
            receipt.get('owner_path') != owner or receipt.get('ordered_texture_references') != paths or
            receipt.get('constant_bytes') != size or receipt.get('static_slot') != 255 or
            receipt.get('resource_offsets') != offsets or receipt.get('resource_types') != types or
            receipt.get('layout_hash') != uniform_layout_hash(size, offsets, types) or
            receipt.get('named_proxy_calls') != 0 or receipt.get('shared_wrap_nonnull') is not True or
            receipt.get('shared_clamp_nonnull') is not True):
        raise ValueError('ComicBubble observed layout or references differ')
    contract = fixture['texture_contract']
    if (contract.get('expression_kind') != 'Texture' or contract.get('parameter') is not None or
            contract.get('named_overrides_allowed') is not False or len(contract['textures']) != 2):
        raise ValueError('ComicBubble source texture contract differs')
    for path, width, height, mips, srgb in COMIC_TEXTURES:
        source_name = 'FortniteGame/Content/' + path[len('/Game/'):]
        matches = [item for item in contract['textures'] if item.get('texture_object') == source_name]
        if len(matches) != 1:
            raise ValueError('ComicBubble source texture reference differs')
        source = matches[0]['source_platform_data']
        if (source['texture_object'] != source_name or source['pixel_format'] != 'PF_DXT1' or
                (source['logical_width'], source['logical_height'], source['mip_count']) != (width, height, mips) or
                source['inspector_metadata']['srgb'] is not srgb or source['first_mip'] != 0 or
                source['platform_virtual_flag'] is not False or len(source['source_files']) != 3):
            raise ValueError('ComicBubble source texture metadata differs')
        source_files = source['source_files']
        expected_stem = source_name.split('.', 1)[0]
        if {str(Path(e['path']))[-len(expected_stem + suffix):] for e in source_files for suffix in (Path(e['path']).suffix,)} != {expected_stem + suffix for suffix in ('.uasset', '.uexp', '.ubulk')}:
            raise ValueError('ComicBubble source texture files differ')
        for entry in source_files:
            data = Path(entry['path']).read_bytes()
            if len(data) != entry['bytes'] or digest(data) != entry['sha256']:
                raise ValueError('ComicBubble source texture package changed')
    uniforms = fixture['texture_2d_parameters']
    rows = receipt.get('uniforms')
    if len(uniforms) != len(indices) or not isinstance(rows, list) or len(rows) != len(indices):
        raise ValueError('ComicBubble texture uniform count differs')
    for uniform, row, expected in zip(uniforms, rows, indices):
        ui, ti, sampler, texture_slot, pair_slot, sampled_slot = expected
        path, width, height, mips, srgb = COMIC_TEXTURES[ti]
        if (uniform.get('expression_kind') != 'Texture' or uniform.get('parameter') is not None or
                uniform.get('texture_index') != ti or uniform.get('sampler_source') != sampler or
                uniform.get('texture_layer_index') != -1 or uniform.get('page_table_layer_index') != -1 or
                uniform.get('virtual_texture') is not False or uniform.get('default_texture_path') != path):
            raise ValueError('ComicBubble source uniform differs')
        actual = tuple(row.get(k) for k in ('uniform_index', 'texture_index', 'sampler_source',
                       'texture_resource_index', 'paired_sampler_resource_index', 'sampled_sampler_resource_index'))
        if (actual != expected or any(type(value) is not int for value in actual) or row.get('texture_path') != path or row.get('pixel_format') != 'PF_DXT1' or
                (row.get('logical_width'), row.get('logical_height'), row.get('source_mips')) != (width, height, mips) or
                row.get('srgb') is not srgb or any(row.get(k) is not True for k in
                    ('gt_indexed_match', 'rt_indexed_match', 'texture_binding_match', 'sampler_binding_match', 'nonwhite_texture'))):
            raise ValueError('ComicBubble observed texture uniform differs')
        resident = row.get('resident_mips')
        if (type(resident) is not int or not 1 <= resident <= mips or
                row.get('rhi_width') != max(1, width >> (mips - resident)) or
                row.get('rhi_height') != max(1, height >> (mips - resident))):
            raise ValueError('ComicBubble resident texture dimensions differ')


def expected_uniform_bindings(record, shader, material_hash, view_hash, layout=None):
    """Original typed registers determine roles; header order/debug names do not."""
    roles = {}
    material = record['material_uniform_buffer']
    if material['bound']:
        roles[material['base_index']] = ('Material', material_hash)
    for parameter in record['uniform_parameters']:
        if parameter['struct_name']['name'] == 'FViewUniformShaderParameters' and parameter['binding']['bound']:
            if view_hash is None:
                raise ValueError('original View binding requires actual current metadata receipt')
            index = parameter['binding']['base_index']
            if index in roles:
                raise ValueError('original uniform register roles overlap')
            roles[index] = ('View', view_hash)
    bindings = {}
    for descriptor in shader['reflection']['descriptors']:
        register = descriptor.get('d3d_register', {})
        if register.get('kind') == 'cbv' and register.get('space') == 0 and register['index'] in roles:
            role = roles[register['index']]
            if role[0] == 'Material' and layout is not None and layout['resource_types'] == [8, 8]:
                span = descriptor['block_span_bytes']
                if not 0 < span <= layout['resource_offsets'][0] or span % 16:
                    raise ValueError('Material descriptor span outside numeric layout')
            if descriptor['binding'] in bindings:
                raise ValueError('mapped uniform register roles overlap')
            bindings[descriptor['binding']] = role
    if len(bindings) != len(roles):
        raise ValueError('original uniform roles do not match mapped descriptors')
    return bindings


def check_material_resource_links(record, shader, layout, header):
    """Match each original UB texture/sampler entry to the emitted Global index."""
    expected = []
    for descriptor in shader['reflection']['descriptors']:
        entries = descriptor.get('resource_table_entries', [])
        if not entries:
            continue
        if len(entries) != 1:
            raise ValueError('ambiguous original texture resource entry')
        entry = entries[0]
        register = descriptor['d3d_register']
        resource = entry['resource']
        base_type = 8 if register['kind'] == 'sampler' else 5 if register['kind'] == 'srv' else -1
        table = 'sampler' if base_type == 8 else 'texture'
        if (entry['uniform_buffer'] != record['material_uniform_buffer']['base_index'] or
                entry['bind'] != register['index'] or entry['table'] != table or
                not 0 <= resource < len(layout['resource_types']) or layout['resource_types'][resource] != base_type):
            raise ValueError('original texture entry disagrees with typed Material layout')
        matches = [(index, value) for index, value in enumerate(header['globals']) if value['binding'] == descriptor['binding']]
        if len(matches) != 1:
            raise ValueError('original texture descriptor lacks a unique Global link')
        index, info = matches[0]
        if (not 0 <= info['type_index'] < len(header['global_types']) or
                header['global_types'][info['type_index']] != descriptor['vulkan_descriptor_type']):
            raise ValueError('native Global descriptor type differs from original resource')
        expected.append([resource, entry['bind'], index, base_type])
    actual = [resource for uniform in header['uniform_buffers'] if uniform.get('verified_role') == 'Material'
              for resource in uniform['resources']]
    if len(expected) != len(actual) or sorted(expected) != sorted(actual):
        raise ValueError('native Material resource-table links differ from original shader')


class Reader:
    def __init__(self, data):
        self.data, self.cursor = data, 0

    def read(self, fmt):
        size = struct.calcsize('<' + fmt)
        if self.cursor + size > len(self.data):
            raise ValueError('truncated native shader header')
        result = struct.unpack_from('<' + fmt, self.data, self.cursor)
        self.cursor += size
        return result[0] if len(result) == 1 else list(result)

    def count(self):
        value = self.read('i')
        if not 0 <= value <= 4096:
            raise ValueError('native header array exceeds bound')
        return value

    def string(self):
        length = self.read('i')
        if not 0 <= length <= 4096 or self.cursor + length > len(self.data):
            raise ValueError('invalid native header debug string')
        data = self.data[self.cursor:self.cursor + length]
        self.cursor += length
        if length and (data[-1] or b'\0' in data[:-1]):
            raise ValueError('invalid native header string terminator')
        return data[:-1].decode() if length else ''

    def array(self, fn):
        return [fn() for _ in range(self.count())]


def read_payload(data):
    r = Reader(data)
    def resource():
        values = r.read('HHHB'); r.string()
        return values
    def uniform():
        layout, binding, only_resources = r.read('IHB')
        resources = r.array(resource); name = r.string()
        return dict(layout_hash=layout, binding=binding, only_resources=only_resources, resources=resources, name=name)
    def global_info():
        binding, alias, type_index, immutable = r.read('HHHB'); name = r.string()
        return dict(binding=binding, alias=alias, type_index=type_index, immutable=immutable, name=name)
    def packed_global():
        result = r.read('HbB'); name = r.string()
        return dict(values=result, name=name)
    header = dict(uniform_buffers=r.array(uniform), globals=r.array(global_info),
                  global_types=r.array(lambda: r.read('B')), packed_globals=r.array(packed_global),
                  packed_buffers=r.array(lambda: r.read('IHbII')))
    for name in ('input_attachments', 'emulated_ranges', 'emulated_copies'):
        if r.count():
            raise ValueError('unsupported nonempty native shader header ' + name)
    header.update(inout_mask=r.read('I'), real_uniform_buffers=r.read('I'),
                  source_hash=bytes(r.read('20B')).hex(), crc=r.read('I'),
                  uniform_offsets=r.array(lambda: r.read('II')),
                  global_offsets=r.array(lambda: r.read('II')), debug_name=r.string())
    words = r.read('i')
    if not 5 <= words <= 1048576:
        raise ValueError('invalid native SPIR-V word count')
    end = r.cursor + words * 4
    if end + 4 != len(data) or struct.unpack_from('<I', data, end)[0] != 4:
        raise ValueError('native payload/code optional-data framing mismatch')
    return header, data[r.cursor:end]


def normalize_entrypoint(data):
    if len(data) < 20 or len(data) % 4:
        raise ValueError('invalid SPIR-V module length')
    words = struct.unpack('<' + 'I' * (len(data) // 4), data)
    result, entries, cursor = [], [], 5
    while cursor < len(words):
        count, opcode = words[cursor] >> 16, words[cursor] & 65535
        if not count or cursor + count > len(words):
            raise ValueError('invalid SPIR-V instruction')
        args = words[cursor + 1:cursor + count]
        if opcode == 15:
            name, length = reflection.string_words(args[2:])
            entries.append(name)
            result.append((opcode, args[:2], args[2 + length:]))
        else:
            result.append((opcode, args))
        cursor += count
    if len(entries) != 1:
        raise ValueError('expected exactly one shader entry point')
    return words[:5], result, entries[0]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--fixture', type=Path, required=True)
    parser.add_argument('--artifacts', type=Path, required=True)
    parser.add_argument('--validator', type=Path, default=GENERATED / 'material_pipeline/tools/root/usr/bin/spirv-val')
    parser.add_argument('--view-layout', type=Path, help='actual compiled View metadata receipt (defaults to artifacts/GTViewLayout.json)')
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    try:
        if GENERATED.resolve() not in args.report.resolve().parents:
            raise ValueError('verification report must stay in porting/generated')
        fixture_bytes = args.fixture.read_bytes()
        bundle = json.loads(fixture_bytes)
        fixture = bundle.get('runtime_fixture', bundle)
        layout = fixture['expected_layout']
        ordinary = (bundle.get('schema') == 'fortnite.legacy-material-ingress.v2'
                    and bundle.get('owner_type') == 'UMaterial' and bundle.get('parent_path') == ''
                    and bundle.get('resource_owner_path') == bundle.get('owner_path')
                    and bundle.get('shader_identity') == bundle.get('owner_path')
                    and fixture.get('texture_context_proof', {}).get('schema') == 'fortnite.original-material-texture-context-proof.v1'
                    and 'textures' in fixture.get('texture_contract', {}))
        ordinary_types = [5, 8] * len(fixture.get('texture_2d_parameters', [])) + [8, 8]
        if (layout['resource_types'] not in ([8, 8], [5, 8, 8, 8]) and not
                (layout['resource_types'] == [5, 8, 5, 8, 8, 8] and
                 bundle.get('owner_path') in COMIC_OWNERS and
                 'textures' in fixture.get('texture_contract', {})) and not
                (ordinary and layout['resource_types'] == ordinary_types)):
            raise ValueError('unsupported checked material resource layout')
        layout_hash = uniform_layout_hash(layout['constant_bytes'], layout['resource_offsets'], layout['resource_types'])
        texture_receipt = None
        if bundle.get('owner_path') in COMIC_OWNERS and 'textures' in fixture.get('texture_contract', {}):
            path = args.artifacts / 'ComicBubbleTextureBindings.json'
            texture_receipt = json.loads(path.read_text(encoding='utf-8-sig'))
            verify_comic_texture_bindings(bundle.get('owner_path'), fixture, texture_receipt)
        elif ordinary:
            path = args.artifacts / 'OriginalTextureBindings.json'
            texture_receipt = json.loads(path.read_text(encoding='utf-8-sig'))
            original_textures.verify(bundle['owner_path'], fixture, texture_receipt, layout_hash)
        elif fixture.get('texture_contract'):
            path = args.artifacts / 'DonutMaterialLayout.json'
            texture_receipt = json.loads(path.read_text(encoding='utf-8-sig'))
            resources = texture_receipt['resources']
            if (texture_receipt['layout_hash'] != layout_hash or texture_receipt['constant_bytes'] != layout['constant_bytes'] or
                    texture_receipt['static_slot'] != 255 or
                    [row['offset'] for row in resources] != layout['resource_offsets'] or
                    [row['type'] for row in resources] != layout['resource_types'] or
                    texture_receipt['indexed_texture'] != fixture['texture_contract']['default_texture_path']):
                raise ValueError('compiled texture layout/reference differs from original source contract')
        view_hash, view_receipt = None, None
        if fixture.get('view_contract'):
            path = args.view_layout or args.artifacts / 'GTViewLayout.json'
            view_receipt = json.loads(path.read_text(encoding='utf-8-sig'))
            resources = view_receipt['resources']
            view_hash = uniform_layout_hash(view_receipt['constant_bytes'],
                [row['offset'] for row in resources], [row['type'] for row in resources], view_receipt['static_slot'])
            contract = fixture['view_contract']
            fields = {field['name']: field for field in view_receipt['fields']}
            if (view_hash != view_receipt['layout_hash'] or
                    view_receipt['game_time_offset'] != contract['expected_current_byte_offset'] or
                    view_receipt['real_time_offset'] != 2268 or
                    view_receipt['constant_bytes'] < contract['shader_visible_bytes'] or
                    any(fields.get(name) != dict(name=name, offset=offset, type=4, size=4)
                        for name, offset in [('GameTime', 2264), ('RealTime', 2268)])):
                raise ValueError('compiled View metadata differs from proven original field contract')
        records = []
        for shader in fixture['shaders']:
            record = shader['record']; name = record['type']['name']
            if not name.isalnum():
                raise ValueError('invalid shader artifact name')
            original = Path(shader['mapped']['output']).read_bytes()
            if digest(original) != shader['mapped']['output_sha256']:
                raise ValueError('mapped original hash changed')
            module_path, payload_path = args.artifacts / (name + '.spv'), args.artifacts / (name + '.ue-vulkan')
            module, payload = module_path.read_bytes(), payload_path.read_bytes()
            before, after = normalize_entrypoint(original), normalize_entrypoint(module)
            crc = zlib.crc32(original)
            if before[:2] != after[:2] or before[2] != 'main' or after[2] != f'main_{len(module):08x}_{crc:08x}':
                raise ValueError('original shader instructions changed beyond entry-point name')
            validation = subprocess.run([str(args.validator), '--target-env', 'vulkan1.0', str(module_path)],
                                        capture_output=True, text=True, timeout=30)
            if validation.returncode:
                raise ValueError('renamed module failed validation: ' + validation.stderr)
            header, embedded = read_payload(payload)
            if (embedded != module or header['crc'] != crc or header['real_uniform_buffers'] != 1
                    or header['source_hash'] != record['source_hash']['hash']):
                raise ValueError('native shader header/code identity differs')
            reflected = reflection.Module(module).reflect()
            expected_mask = 0
            direction = 'input' if record['frequency'] == 0 else 'output'
            for interface in reflected['interfaces']:
                if interface['direction'] == direction and 'location' in interface:
                    expected_mask |= 1 << interface['location']
            if header['inout_mask'] != expected_mask:
                raise ValueError('native vertex-input/pixel-output mask mismatch')
            references = [(b[3], b[4], b[1]) for b in header['packed_buffers']]
            if len(header['uniform_buffers']) != len(header['uniform_offsets']) or len(header['globals']) != len(header['global_offsets']):
                raise ValueError('native header descriptor metadata count mismatch')
            expected_uniforms = expected_uniform_bindings(record, shader, layout_hash, view_hash, layout)
            if len(expected_uniforms) != len(header['uniform_buffers']):
                raise ValueError('native uniform count differs from original typed bindings')
            for uniform, offsets in zip(header['uniform_buffers'], header['uniform_offsets']):
                role = expected_uniforms.get(uniform['binding'])
                if role is None or uniform['layout_hash'] != role[1]:
                    raise ValueError('native uniform layout differs from its original typed register role')
                uniform['verified_role'] = role[0]
                references.append((*offsets, uniform['binding']))
            check_material_resource_links(record, shader, layout, header)
            references += [(*offsets, info['binding']) for info, offsets in zip(header['globals'], header['global_offsets'])]
            declared = {(d['set_word_offset'], d['binding_word_offset'], d['binding']) for d in reflected['descriptors']}
            if len(references) != len(declared) or set(references) != declared:
                raise ValueError('native descriptor patch offsets differ from actual SPIR-V')
            records.append(dict(shader_type=name, spirv_sha256=digest(module), payload_sha256=digest(payload),
                                instructions_preserved=True, validator_exit=validation.returncode,
                                descriptor_count=len(declared), header=header))
        if len(records) != 7:
            raise ValueError('expected all seven Slate shader artifacts')
        report = dict(fixture_sha256=digest(fixture_bytes), verifier_sha256=digest(Path(__file__).read_bytes()),
                      validator_sha256=digest(args.validator.read_bytes()), layout_hash=layout_hash, records=records,
                      limitations=['Header/module artifacts verified; actual pipeline execution and visual parity are separate runtime gates'])
        if view_receipt is not None:
            report['view_layout'] = view_receipt
        if texture_receipt is not None:
            report['texture_layout'] = texture_receipt
        args.report.parent.mkdir(parents=True, exist_ok=True)
        temporary = args.report.with_suffix('.tmp')
        temporary.write_text(json.dumps(report, indent=2) + '\n'); temporary.replace(args.report)
        print(f'verified seven modules/payloads, source layout hash {layout_hash}: {args.report}')
    except (OSError, ValueError, KeyError, TypeError, IndexError, subprocess.TimeoutExpired) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
