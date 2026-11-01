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


def expected_uniform_bindings(record, shader, material_hash, view_hash):
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
            if descriptor['binding'] in bindings:
                raise ValueError('mapped uniform register roles overlap')
            bindings[descriptor['binding']] = role
    if len(bindings) != len(roles):
        raise ValueError('original uniform roles do not match mapped descriptors')
    return bindings


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
        fixture = json.loads(fixture_bytes)
        fixture = fixture.get('runtime_fixture', fixture)
        layout = fixture['expected_layout']
        if layout['resource_types'] != [8, 8]:
            raise ValueError('unsupported checked material resource layout')
        layout_hash = uniform_layout_hash(layout['constant_bytes'], layout['resource_offsets'], layout['resource_types'])
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
            expected_uniforms = expected_uniform_bindings(record, shader, layout_hash, view_hash)
            if len(expected_uniforms) != len(header['uniform_buffers']):
                raise ValueError('native uniform count differs from original typed bindings')
            for uniform, offsets in zip(header['uniform_buffers'], header['uniform_offsets']):
                role = expected_uniforms.get(uniform['binding'])
                if role is None or uniform['layout_hash'] != role[1]:
                    raise ValueError('native uniform layout differs from its original typed register role')
                uniform['verified_role'] = role[0]
                references.append((*offsets, uniform['binding']))
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
        args.report.parent.mkdir(parents=True, exist_ok=True)
        temporary = args.report.with_suffix('.tmp')
        temporary.write_text(json.dumps(report, indent=2) + '\n'); temporary.replace(args.report)
        print(f'verified seven modules/payloads, source layout hash {layout_hash}: {args.report}')
    except (OSError, ValueError, KeyError, TypeError, IndexError, subprocess.TimeoutExpired) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
