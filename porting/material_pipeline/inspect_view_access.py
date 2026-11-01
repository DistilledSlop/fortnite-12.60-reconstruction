#!/usr/bin/env python3
"""Prove the exact scalar View loads in hash-checked mapped SPIR-V.

Only constant-index struct/array/vector pointer chains ending in float32 loads
or float4 loads whose uses select only GameTime are accepted. Pointer copies,
calls, writes and other escapes fail closed. The
SPIR-V grammar distinguishes actual ID references from coincident literal words.
This proves accesses, not equivalence of the entire original/current View ABI.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct

HERE = Path(__file__).resolve().parent
GRAMMAR = Path('/opt/fortnite/UnrealEngine/Engine/Source/ThirdParty/ShaderConductor/ShaderConductor/External/SPIRV-Headers/include/spirv/1.0/spirv.core.grammar.json')
spec = importlib.util.spec_from_file_location('view_reflection', HERE/'reflect_mapped_spirv.py')
reflection = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reflection)


class Grammar:
    def __init__(self, path=GRAMMAR):
        raw = Path(path).read_bytes()
        doc = json.loads(raw)
        self.sha256 = hashlib.sha256(raw).hexdigest()
        self.instructions = {r['opcode']: r for r in doc['instructions']}
        self.kinds = {r['kind']: r for r in doc['operand_kinds']}

    def references(self, opcode, words):
        """Return operand positions holding IDs, excluding result IDs/types."""
        cursor, refs = 0, set()

        def kind(name):
            nonlocal cursor
            if cursor >= len(words):
                raise ValueError('truncated grammar operand')
            row = self.kinds[name]
            category = row['category']
            if category == 'Composite':
                for base in row['bases']:
                    kind(base)
                return
            pos, value = cursor, words[cursor]
            cursor += 1
            if category == 'Id':
                if name not in ('IdResult', 'IdResultType'):
                    refs.add(pos)
            elif name == 'LiteralString':
                while not any(b == 0 for b in struct.pack('<I', value)):
                    if cursor >= len(words):
                        raise ValueError('unterminated grammar string')
                    value = words[cursor]
                    cursor += 1
            elif category in ('ValueEnum', 'BitEnum'):
                remaining = value
                matched = False
                for item in row['enumerants']:
                    number = int(str(item['value']), 0)
                    selected = (value == number if category == 'ValueEnum'
                                else number != 0 and value & number == number)
                    if selected:
                        matched = True
                        remaining &= ~number
                        for parameter in item.get('parameters', []):
                            kind(parameter['kind'])
                        if category == 'ValueEnum':
                            break
                if category == 'BitEnum' and remaining or category == 'ValueEnum' and not matched:
                    raise ValueError('unknown grammar enumeration')
            elif name == 'LiteralContextDependentNumber':
                raise ValueError('context-dependent literal unsupported in function')
            elif category != 'Literal':
                raise ValueError('unsupported grammar operand kind')

        try:
            operands = self.instructions[opcode].get('operands', [])
            for operand in operands:
                repeat = operand.get('quantifier')
                if repeat == '*':
                    while cursor < len(words):
                        kind(operand['kind'])
                elif repeat != '?' or cursor < len(words):
                    kind(operand['kind'])
        except KeyError as error:
            raise ValueError('unknown SPIR-V grammar instruction or operand') from error
        if cursor != len(words):
            raise ValueError('unconsumed grammar operands')
        return refs


def inspect_access(data, variable_id, grammar):
    module = reflection.Module(data)
    pointer_type, storage = module.variables[variable_id]
    opcode, args = module.types[pointer_type]
    if storage != 2 or opcode != 32 or args[0] != 2:
        raise ValueError('View must be a Uniform pointer')
    pointers = {variable_id: (args[1], 0)}
    accesses, vector_loads = [], {}
    words = struct.unpack('<'+'I'*(len(data)//4), data)
    cursor, inside = 5, False

    def index(identifier):
        if identifier not in module.constants:
            raise ValueError('View index must be constant')
        type_id, literal = module.constants[identifier]
        op, operands = module.types[type_id]
        if op != 21 or operands[0] != 32 or len(literal) != 1:
            raise ValueError('View index must be a constant uint32/int32')
        value = literal[0]
        if value > 1048576:
            raise ValueError('View index exceeds bounded range')
        return value

    while cursor < len(words):
        count, op = words[cursor] >> 16, words[cursor] & 0xffff
        if count < 1 or cursor + count > len(words):
            raise ValueError('invalid instruction framing')
        operands = words[cursor+1:cursor+count]
        if op == 54:  # OpFunction
            inside = True
        if inside:
            references = grammar.references(op, operands)
            value_uses = [i for i in references if operands[i] in vector_loads]
            if value_uses:
                if op == 79 and all(i in (2, 3) for i in value_uses):
                    # OpVectorShuffle: examine every selected source lane. An
                    # unselected lane from a float4 load never reaches an operation.
                    for lane in operands[4:]:
                        if lane == 0xffffffff:
                            raise ValueError('undefined lane in View shuffle')
                        source, component = (operands[2], lane) if lane < 4 else (operands[3], lane-4)
                        if component >= 4:
                            raise ValueError('View shuffle index out of bounds')
                        if source in vector_loads and vector_loads[source]['physical_byte_offset']+4*component != 2264:
                            raise ValueError('View shuffle observes an unproven field')
                elif op == 81 and value_uses == [2] and len(operands) == 4 and operands[3] == 2:
                    pass  # OpCompositeExtract of GameTime from the final float4.
                else:
                    raise ValueError('View float4 value escapes verified component selection')
            uses = [i for i in references if operands[i] in pointers]
            if uses:
                if op in (65, 66) and uses == [2]:
                    target, offset = pointers[operands[2]]
                    for identifier in operands[3:]:
                        value = index(identifier)
                        kind, parts = module.types[target]
                        if kind == 30:
                            if value >= len(parts):
                                raise ValueError('View struct index out of bounds')
                            annotations = module.members.get((target, value), {})
                            if any(k in annotations for k in (4, 5, 7)):
                                raise ValueError('View matrix layout unsupported')
                            offset += annotations[35][0]
                            target = parts[value]
                        elif kind == 28:
                            if value >= module.constant(parts[1]):
                                raise ValueError('View array index out of bounds')
                            stride, = module.decorations[target][6]
                            offset += value * stride
                            target = parts[0]
                        elif kind == 23:
                            if value >= parts[1]:
                                raise ValueError('View vector index out of bounds')
                            offset += value * module.byte_span(parts[0])
                            target = parts[0]
                        else:
                            raise ValueError('unsupported View pointer traversal')
                    pointer_op, pointer_args = module.types[operands[0]]
                    if pointer_op != 32 or pointer_args != (2, target):
                        raise ValueError('View pointer result type mismatch')
                    pointers[operands[1]] = target, offset
                elif op == 61 and uses == [2]:  # OpLoad
                    target, offset = pointers[operands[2]]
                    if target != operands[0]:
                        raise ValueError('View load result type mismatch')
                    kind, parts = module.types[target]
                    if kind == 23 and parts[1] == 4 and module.types[parts[0]] == (22, (32,)) and offset == 2256:
                        access = dict(instruction_word=cursor, byte_offset=2264,
                                      byte_count=4, scalar_type='float32',
                                      physical_byte_offset=2256, physical_load_bytes=16,
                                      scope='Float4 load; all uses select only GameTime')
                        vector_loads[operands[1]] = access
                        accesses.append(access)
                        cursor += count
                        continue
                    if module.types[target] != (22, (32,)):
                        raise ValueError(f'View load must be float32 scalar at word {cursor}: '
                                         f'target {module.types[target]}, result {module.types[operands[0]]}, offset {offset}')
                    if offset != 2264:
                        raise ValueError('View load does not access proven GameTime field')
                    accesses.append(dict(instruction_word=cursor, byte_offset=offset,
                                         byte_count=4, scalar_type='float32'))
                else:
                    raise ValueError('View pointer escapes constant access/load contract')
        if op == 56:
            inside = False
        cursor += count
    if not accesses:
        raise ValueError('View has no verified scalar loads')
    return dict(field_name='GameTime', byte_offset=2264, scalar_type='float32',
                accesses=accesses, pointer_escapes=0, grammar_sha256=grammar.sha256)


def verify_record(row, grammar):
    path = Path(row['output'])
    data = path.read_bytes()
    if hashlib.sha256(data).hexdigest() != row['spirv_sha256']:
        raise ValueError('mapped SPIR-V provenance mismatch')
    view = [d for d in row['descriptors'] if d['d3d_register'] == dict(kind='cbv', space=0, index=1)]
    material = [d for d in row['descriptors'] if d['d3d_register'] == dict(kind='cbv', space=0, index=2)]
    if row['frequency'] != 3 or len(view) != 1 or len(material) != 1 or view[0]['block_span_bytes'] != 2272:
        raise ValueError('not a supported View b1/Material b2 pixel shader')
    result = inspect_access(data, view[0]['variable_id'], grammar)
    return dict(shader_index=row['shader_index'], spirv_sha256=row['spirv_sha256'],
                output=str(path.resolve()), shader_visible_bytes=2272, **result)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reflection-report', type=Path, required=True)
    parser.add_argument('--shader-index', type=int, action='append', required=True)
    parser.add_argument('--grammar', type=Path, default=GRAMMAR)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    try:
        raw = args.reflection_report.read_bytes()
        doc = json.loads(raw)
        by_index = {r['shader_index']: r for r in doc['records']}
        grammar = Grammar(args.grammar)
        result = dict(schema='fortnite.shader-view-access.v1',
                      inspector_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                      reflection_report_sha256=hashlib.sha256(raw).hexdigest(),
                      grammar_path=str(args.grammar.resolve()), grammar_sha256=grammar.sha256,
                      records=[verify_record(by_index[i], grammar) for i in args.shader_index],
                      scope='Only observed scalar GameTime loads; full View ABI and original pixels unverified')
        generated = HERE.parent/'generated'
        if generated.resolve() not in args.report.resolve().parents or args.report.exists():
            raise ValueError('choose a fresh report under generated')
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(result, indent=2)+'\n')
        print('verified GameTime-only loads in', len(result['records']), 'mapped pixel shaders')
    except (OSError, ValueError, KeyError, TypeError, struct.error) as error:
        parser.exit(1, f'error: {error}\n')


if __name__ == '__main__':
    main()
