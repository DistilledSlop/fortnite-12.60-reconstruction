#!/usr/bin/env python3
"""Inventory physical View loads in the 33 mapped Paper2D modules.

A complete record proves only that the physical SPIR-V loads are constant and
map to same-offset, same-shape metadata members. It does not prove values,
resource contents, padding semantics, rendering or runtime parity. Unhandled
pointer operations and dynamic indexing explicitly invalidate that proof.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
MANIFEST = ROOT / 'generated/paper2d-shader-semantics/shader-manifest.json'
LAYOUT = ROOT / 'generated/material_pipeline/view-identity/full-layout-contract.json'
spec = importlib.util.spec_from_file_location('inspect_view_access', HERE / 'inspect_view_access.py')
inspector = importlib.util.module_from_spec(spec)
spec.loader.exec_module(inspector)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def inspect(data, variable, grammar):
    module = inspector.reflection.Module(data)
    pointer, storage = module.variables[variable]
    op, args = module.types[pointer]
    if storage != 2 or op != 32 or args[0] != 2:
        raise ValueError('View descriptor is not a Uniform pointer')
    pointers = {variable: (args[1], 0)}
    accesses, issues = [], []
    words = struct.unpack('<' + 'I' * (len(data)//4), data)
    cursor, inside = 5, False
    while cursor < len(words):
        count, op = words[cursor] >> 16, words[cursor] & 0xffff
        if count < 1 or cursor + count > len(words):
            raise ValueError('invalid instruction framing')
        a = words[cursor+1:cursor+count]
        if op == 54:
            inside = True
        if inside:
            refs = grammar.references(op, a)
            uses = [i for i in sorted(refs) if a[i] in pointers]
            if uses:
                try:
                    if op in (65, 66) and uses == [2]:
                        target, offset = pointers[a[2]]
                        if offset is None:
                            raise ValueError('depends on unresolved pointer')
                        for ident in a[3:]:
                            if ident not in module.constants:
                                raise ValueError('dynamic index id %d' % ident)
                            typ, literal = module.constants[ident]
                            kind, parts = module.types[typ]
                            if kind != 21 or parts[0] != 32 or len(literal) != 1 or literal[0] > 1048576:
                                raise ValueError('unsupported index constant')
                            value = literal[0]
                            kind, parts = module.types[target]
                            if kind == 30:
                                if value >= len(parts):
                                    raise ValueError('struct index out of bounds')
                                ann = module.members.get((target, value), {})
                                if any(x in ann for x in (4, 5, 7)):
                                    raise ValueError('matrix layout unsupported')
                                offset += ann[35][0]
                                target = parts[value]
                            elif kind == 28:
                                if value >= module.constant(parts[1]):
                                    raise ValueError('array index out of bounds')
                                stride, = module.decorations[target][6]
                                offset += value * stride
                                target = parts[0]
                            elif kind == 23:
                                if value >= parts[1]:
                                    raise ValueError('vector index out of bounds')
                                offset += value * module.byte_span(parts[0])
                                target = parts[0]
                            else:
                                raise ValueError('unsupported pointer traversal')
                        if module.types[a[0]] != (32, (2, target)):
                            raise ValueError('pointer type mismatch')
                        pointers[a[1]] = target, offset
                    elif op == 61 and uses == [2]:
                        target, offset = pointers[a[2]]
                        if offset is None:
                            raise ValueError('load from unresolved pointer')
                        if target != a[0]:
                            raise ValueError('load type mismatch')
                        size = module.byte_span(target)
                        if offset % 4 or size % 4:
                            raise ValueError('non-word load')
                        accesses.append(dict(instruction_word=cursor, byte_offset=offset,
                            byte_count=size, word_offsets=list(range(offset//4, (offset+size)//4)),
                            type=module.type_summary(target)))
                    else:
                        raise ValueError('unsupported pointer use opcode %d' % op)
                except (ValueError, KeyError, IndexError, TypeError) as error:
                    issues.append(dict(instruction_word=cursor, opcode=op, reason=str(error)))
                    if op in (65, 66) and len(a) >= 2:
                        # Continue tainting subsequent uses so no unresolved load is hidden.
                        result = module.types.get(a[0])
                        if result and result[0] == 32:
                            pointers[a[1]] = (result[1][1], None)
        if op == 56:
            inside = False
        cursor += count
    return accesses, issues


def map_word(offset, original, current):
    matches = [m for m in original if m['offset'] <= offset and offset + 4 <= m['offset'] + m['size']]
    if len(matches) != 1:
        return dict(byte_offset=offset, status='unmapped_or_ambiguous', original_members=[m['name'] for m in matches])
    orig = matches[0]
    found = [m for m in current if m['name'] == orig['name']]
    keys = ('base_type', 'precision', 'rows', 'columns', 'elements', 'size', 'shader_type')
    cur = found[0] if len(found) == 1 else None
    same_shape = cur is not None and all(orig[k] == cur[k] for k in keys)
    compatible = same_shape and orig['offset'] == cur['offset']
    return dict(byte_offset=offset, original_member={k: orig[k] for k in ('name', 'offset', *keys)},
        current_member=({k: cur[k] for k in ('name', 'offset', *keys)} if cur else None),
        current_byte_offset=(cur['offset'] + offset - orig['offset'] if same_shape else None),
        status='same_offset_and_shape' if compatible else 'changed_or_missing')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--evidence', type=Path, help='also write the bounded checked-in evidence')
    args = parser.parse_args()
    if ROOT.joinpath('generated').resolve() not in args.output.resolve().parents:
        parser.error('output must be under porting/generated')
    manifest_raw, layout_raw = MANIFEST.read_bytes(), LAYOUT.read_bytes()
    manifest, layout = json.loads(manifest_raw), json.loads(layout_raw)
    if manifest['schema'] != 'paper2d-original-shader-manifest-v1' or len(manifest['shaders']) != 33:
        raise ValueError('unexpected manifest')
    if (layout['original']['struct_size'], layout['current']['struct_size']) != (3952, 4032):
        raise ValueError('unexpected View layout sizes')
    grammar = inspector.Grammar()
    associations = {a['shader_index']: a for a in layout['original_shader_view_associations']}
    records = []
    for row in manifest['shaders']:
        index = row['archive_index']
        raw = Path(row['mapped']['output']).read_bytes()
        if sha(raw) != row['mapped']['output_sha256'] or not row['mapped']['validated']:
            raise ValueError('mapped module provenance mismatch: %d' % index)
        names = [o['uniform_buffer_names'] for o in row['d3d']['optional_data'] if 'uniform_buffer_names' in o]
        if len(names) > 1 or (names and names[0].count('View') > 1):
            raise ValueError('ambiguous wrapper View index: %d' % index)
        indices = [i for i, n in enumerate(names[0] if names else []) if n == 'View']
        record = dict(shader_index=index, frequency=row['frequency'], spirv_sha256=sha(raw),
                      wrapper_sha256=row['d3d']['wrapper_sha256'], uniform_buffer_names=names[0] if names else [],
                      resource_bindings=row['d3d']['resource_bindings'])
        if not indices:
            if index in associations:
                raise ValueError('View association disagrees with wrapper: %d' % index)
            record.update(status='view_not_identified_in_wrapper', accesses=[], issues=[])
        else:
            ub = indices[0]
            association = associations.get(index)
            if not association or association['uniform_buffer'] != ub:
                raise ValueError('View association mismatch: %d' % index)
            descriptors = [d for d in row['reflection']['descriptors'] if d['d3d_register'] == dict(kind='cbv', space=0, index=ub)]
            if len(descriptors) != 1:
                raise ValueError('missing View descriptor: %d' % index)
            descriptor = descriptors[0]
            record.update(view_association=association, view_descriptor=descriptor)
            try:
                accesses, issues = inspect(raw, descriptor['variable_id'], grammar)
                offsets = sorted({w*4 for a in accesses for w in a['word_offsets']})
                mapping = [map_word(o, layout['original']['members'], layout['current']['members']) for o in offsets]
                status = 'unsupported' if issues else ('same_offset_and_shape' if all(m['status'] == 'same_offset_and_shape' for m in mapping) else 'layout_mismatch_or_unmapped')
                record.update(status=status, accesses=accesses, issues=issues, word_mapping=mapping)
            except (ValueError, KeyError, IndexError, TypeError, struct.error) as error:
                record.update(status='unsupported', accesses=[], issues=[dict(reason=str(error))], word_mapping=[])
        records.append(record)
    result = dict(schema='paper2d-view-physical-access-v1', manifest_sha256=sha(manifest_raw),
        layout_sha256=sha(layout_raw), grammar_sha256=grammar.sha256,
        inspector_sha256=sha((HERE/'inspect_view_access.py').read_bytes()), analyzer_sha256=sha(Path(__file__).read_bytes()),
        original_struct_bytes=3952, current_struct_bytes=4032,
        differences=len(layout['comparison']['differences']), runtime_verified=False,
        scope='Physical mapped SPIR-V loads, including all lanes of vector loads; same-offset metadata is necessary but not sufficient for runtime compatibility.', records=records)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2)+'\n')
    from collections import Counter
    if args.evidence:
        expected = ROOT / 'material_pipeline/evidence/paper2d_view_accesses_runtime/evidence.json'
        if args.evidence.resolve() != expected.resolve():
            raise ValueError('evidence destination must be ' + str(expected))
        compact = []
        for r in records:
            item = {k: r[k] for k in ('shader_index', 'status')}
            if r['issues']:
                item['issues'] = r['issues']
            if 'view_descriptor' in r:
                desc = r['view_descriptor']
                ub = r['view_association']['uniform_buffer']
                item['view_binding'] = dict(uniform_buffer=ub, set=desc['set'],
                    binding=desc['binding'], resource_table_active=r['view_association']['resource_table_active'],
                    original_layout_hash=r['view_association']['original_layout_hash'])
                item['view_resource_bindings'] = {k: [(v['resource'], v['bind']) for v in values
                    if v['uniform_buffer'] == ub] for k, values in r['resource_bindings'].items()
                    if any(v['uniform_buffer'] == ub for v in values)}
                item['physical_load_count'] = len(r['accesses'])
                item['changed_or_missing_members'] = sorted({m['original_member']['name']
                    for m in r['word_mapping'] if m['status'] == 'changed_or_missing'})
                item['unmapped_or_ambiguous_word_count'] = sum(
                    m['status'] == 'unmapped_or_ambiguous' for m in r['word_mapping'])
            compact.append(item)
        evidence = dict(schema='paper2d-view-access-evidence-v1',
            command='python3 porting/material_pipeline/paper2d_view_accesses.py --output porting/generated/paper2d-shader-semantics/view-accesses.json --evidence porting/material_pipeline/evidence/paper2d_view_accesses_runtime/evidence.json',
            report_sha256=sha(args.output.read_bytes()),
            provenance={k: result[k] for k in ('manifest_sha256', 'layout_sha256', 'grammar_sha256', 'inspector_sha256', 'analyzer_sha256')},
            status_counts=dict(Counter(r['status'] for r in records)), original_struct_bytes=3952,
            current_struct_bytes=4032, full_layout_differences=result['differences'],
            physical_load_count=sum(len(r['accesses']) for r in records),
            scope='Physical loads include all vector lanes, including potentially unused or padding lanes. Same offset and shape are necessary only; values, resources and runtime parity are not proven. No identified View does not prove no View accesses. Unresolved pointer uses and dynamic indices are unsupported.',
            shaders=compact)
        args.evidence.parent.mkdir(parents=True, exist_ok=True)
        args.evidence.write_text(json.dumps(evidence, indent=2) + '\n')
    print(dict(Counter(r['status'] for r in records)))


if __name__ == '__main__':
    main()
