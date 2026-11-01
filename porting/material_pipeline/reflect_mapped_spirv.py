#!/usr/bin/env python3
"""Reflect actual mapped SPIR-V types, block spans and graphics interfaces.

Only explicit SPIR-V declarations and layout decorations are interpreted.
Runtime values, UE layout identities and visual equivalence are not inferred.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct

HERE = Path(__file__).resolve().parent
GENERATED = HERE.parent / 'generated'
spec = importlib.util.spec_from_file_location('binding_inventory', HERE / 'inspect_spirv_bindings.py')
inventory = importlib.util.module_from_spec(spec)
spec.loader.exec_module(inventory)
VK_TYPES = {0: 'sampler', 1: 'combined_image_sampler', 2: 'sampled_image', 3: 'storage_image',
            4: 'uniform_texel_buffer', 5: 'storage_texel_buffer', 6: 'uniform_buffer',
            7: 'storage_buffer', 10: 'input_attachment'}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def string_words(operands):
    raw = struct.pack('<' + 'I' * len(operands), *operands)
    if b'\0' not in raw:
        raise ValueError('unterminated SPIR-V string')
    size = raw.index(0)
    return raw[:size].decode('utf-8'), (size + 4) // 4


class Module:
    def __init__(self, data):
        self.descriptors = inventory.spirv_descriptors(data, include_offsets=True)
        self.types, self.constants, self.variables = {}, {}, {}
        self.decorations, self.members, self.entries, self.names = {}, {}, [], {}
        words = struct.unpack('<' + 'I' * (len(data) // 4), data)
        cursor = 5
        while cursor < len(words):
            count, opcode = words[cursor] >> 16, words[cursor] & 0xffff
            operands = words[cursor + 1:cursor + count]
            if 19 <= opcode <= 33:
                self.types[operands[0]] = (opcode, operands[1:])
            elif opcode == 43:  # OpConstant; resolve integer widths when used.
                self.constants[operands[1]] = (operands[0], operands[2:])
            elif opcode == 59:
                self.variables[operands[1]] = (operands[0], operands[2])
            elif opcode == 71:
                target, decoration = operands[:2]
                self.decorations.setdefault(target, {})[decoration] = operands[2:]
            elif opcode == 72:
                target, member, decoration = operands[:3]
                self.members.setdefault((target, member), {})[decoration] = operands[3:]
            elif opcode == 15:
                name, length = string_words(operands[2:])
                self.entries.append(dict(execution_model=operands[0], function_id=operands[1],
                                         name=name, interfaces=list(operands[2 + length:])))
            elif opcode == 5:
                self.names[operands[0]] = string_words(operands[1:])[0]
            cursor += count
        if len(self.entries) != 1:
            raise ValueError('only single-entry original modules are supported')

    def constant(self, identifier):
        type_id, literal = self.constants[identifier]
        opcode, operands = self.types[type_id]
        if opcode != 21 or operands[0] != 32 or len(literal) != 1:
            raise ValueError('unsupported array length constant')
        value = literal[0]
        if not 0 < value <= 1048576:
            raise ValueError('invalid bounded array length')
        return value

    def type_summary(self, identifier, depth=0):
        if depth > 32:
            raise ValueError('type nesting limit')
        op, args = self.types[identifier]
        if op in (21, 22):
            return dict(kind='int' if op == 21 else 'float', bits=args[0],
                        **({'signed': bool(args[1])} if op == 21 else {}))
        if op == 20:
            return dict(kind='bool')
        if op in (23, 24):
            return dict(kind='vector' if op == 23 else 'matrix', count=args[1],
                        element=self.type_summary(args[0], depth + 1))
        if op in (28, 29):
            return dict(kind='array' if op == 28 else 'runtime_array',
                        count=self.constant(args[1]) if op == 28 else None,
                        element=self.type_summary(args[0], depth + 1))
        if op == 30:
            return dict(kind='struct', members=[self.type_summary(x, depth + 1) for x in args])
        if op == 32:
            return dict(kind='pointer', storage_class=args[0], target=self.type_summary(args[1], depth + 1))
        return dict(kind={25: 'image', 26: 'sampler', 27: 'sampled_image'}.get(op, 'unsupported'),
                    opcode=op, operands=list(args))

    def byte_span(self, identifier, depth=0):
        if depth > 32:
            raise ValueError('block layout nesting limit')
        op, args = self.types[identifier]
        if op in (21, 22) and args[0] in (8, 16, 32, 64):
            return args[0] // 8
        if op == 23:
            return self.byte_span(args[0], depth + 1) * args[1]
        if op == 28:
            stride, = self.decorations.get(identifier, {}).get(6, ())  # ArrayStride
            count = self.constant(args[1])
            if stride < self.byte_span(args[0], depth + 1):
                raise ValueError('array stride smaller than element')
            return stride * count
        if op == 30:
            extent = 0
            for index, member_type in enumerate(args):
                annotations = self.members.get((identifier, index), {})
                if 7 in annotations or 4 in annotations or 5 in annotations:
                    raise ValueError('matrix member layout requires a separate decoder')
                offset, = annotations.get(35, ())  # Offset, never infer C layout.
                extent = max(extent, offset + self.byte_span(member_type, depth + 1))
            return extent
        raise ValueError('unsupported finite block layout')

    def reflect(self):
        descriptors = []
        by_location = {}
        for identifier, decorations in self.decorations.items():
            if 33 in decorations and 34 in decorations:
                by_location[decorations[34][0], decorations[33][0]] = identifier
        for item in self.descriptors:
            identifier = by_location[item['set'], item['binding']]
            pointer, storage = self.variables[identifier]
            op, args = self.types[pointer]
            if op != 32 or args[0] != storage:
                raise ValueError('descriptor pointer storage mismatch')
            target = args[1]
            kind, arguments = self.types[target]
            row = dict(item, variable_id=identifier, storage_class=storage)
            if storage in (2, 12) and kind == 30:
                markers = self.decorations.get(target, {})
                if storage == 12 or 3 in markers:
                    vk_type = 7
                elif 2 in markers:
                    vk_type = 6
                else:
                    raise ValueError('uniform struct lacks explicit Block decoration')
                row['block_span_bytes'] = self.byte_span(target)
            elif storage == 0 and kind in (25, 26, 27):
                if kind == 26:
                    vk_type = 0
                elif kind == 27:
                    vk_type = 1
                else:
                    dimension, sampled = arguments[1], arguments[5]
                    row['image_type'] = dict(sampled_type=self.type_summary(arguments[0]),
                        dimension=dimension, depth=arguments[2], arrayed=arguments[3],
                        multisampled=arguments[4], sampled=sampled, format=arguments[6])
                    if dimension == 6:
                        vk_type = 10
                    elif sampled in (1, 2):
                        vk_type = (4 if sampled == 1 else 5) if dimension == 5 else (2 if sampled == 1 else 3)
                    else:
                        raise ValueError('image sampled/storage usage is unknown')
            else:
                raise ValueError('unsupported descriptor pointer type')
            row.update(vulkan_descriptor_type=vk_type, vulkan_descriptor_name=VK_TYPES[vk_type])
            descriptors.append(row)
        entry, = self.entries
        interfaces = []
        for identifier in entry['interfaces']:
            pointer, storage = self.variables[identifier]
            if storage not in (1, 3):
                raise ValueError('entry interface is not input/output')
            op, args = self.types[pointer]
            if op != 32 or args[0] != storage:
                raise ValueError('interface pointer storage mismatch')
            annotations = self.decorations.get(identifier, {})
            row = dict(variable_id=identifier, name=self.names.get(identifier),
                       direction='input' if storage == 1 else 'output', type=self.type_summary(args[1]))
            for field, decoration in [('location', 30), ('builtin', 11), ('component', 31)]:
                if decoration in annotations:
                    row[field], = annotations[decoration]
            interfaces.append(row)
        return dict(entry_point=entry, descriptors=descriptors, interfaces=interfaces)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mapped-report', type=Path, required=True)
    parser.add_argument('--report', type=Path, default=GENERATED / 'material_pipeline/mapped-spirv-reflection.json')
    args = parser.parse_args()
    try:
        if GENERATED.resolve() not in args.report.resolve().parents:
            raise ValueError('report must stay in porting/generated')
        raw = args.mapped_report.read_bytes()
        source = json.loads(raw)
        records = []
        for shader in source['records']:
            path = Path(shader['output']).resolve()
            if GENERATED.resolve() not in path.parents or not shader['validated']:
                raise ValueError('unvalidated/outside-generated shader')
            data = path.read_bytes()
            if digest(data) != shader['output_sha256']:
                raise ValueError('mapped SPIR-V identity changed')
            row = Module(data).reflect()
            assignments = {(b['set'], b['binding']): b for b in shader['bindings']}
            if assignments.keys() != {(d['set'], d['binding']) for d in row['descriptors']}:
                raise ValueError('reflected descriptors differ from explicit assignment locations')
            compatible = {'cbv': {6}, 'sampler': {0}, 'srv': {2, 4}, 'uav': {3, 5, 7}}
            for descriptor in row['descriptors']:
                binding = assignments[descriptor['set'], descriptor['binding']]
                if descriptor['vulkan_descriptor_type'] not in compatible[binding['kind']]:
                    raise ValueError('reflected descriptor type differs from D3D register kind')
                descriptor['d3d_register'] = dict(kind=binding['kind'],
                    space=binding['register_space'], index=binding['register_index'])
                descriptor['resource_table_entries'] = binding['resource_table_entries']
            row.update(shader_index=shader['shader_index'], frequency=shader['frequency'],
                       spirv_sha256=digest(data), output=str(path))
            records.append(row)
        result = dict(mapped_report_sha256=digest(raw), reflector_sha256=digest(Path(__file__).read_bytes()),
                      inventory_sha256=digest((HERE / 'inspect_spirv_bindings.py').read_bytes()), records=records,
                      limitations=['Declared block spans do not supply runtime values or UE layout identities',
                                   'Graphics interface declarations do not prove paired shader or draw correctness'])
        args.report.parent.mkdir(parents=True, exist_ok=True)
        temporary = args.report.with_suffix('.tmp')
        temporary.write_text(json.dumps(result, indent=2, allow_nan=False) + '\n')
        temporary.replace(args.report)
        print(f'reflected {len(records)} hash-verified shaders: {args.report}')
    except (OSError, ValueError, KeyError, TypeError, IndexError) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
