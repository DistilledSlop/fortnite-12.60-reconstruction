#!/usr/bin/env python3
"""Conservatively bound consumed scalar/vector lanes of mapped physical View loads.

Only explicit component operations and an allowlist of lane-wise core operations
can discard a lane. Unknown uses, memory stores, calls and cycles retain it.
This is a static use bound, not a rendering, ABI-value or runtime parity proof.
"""
import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
import struct

import paper2d_view_accesses as physical

HERE, ROOT = physical.HERE, physical.ROOT
PHYSICAL = ROOT / 'generated/paper2d-shader-semantics/view-accesses.json'
LANEWISE = set(('OpSNegate OpFNegate OpIAdd OpFAdd OpISub OpFSub OpIMul OpFMul '
    'OpUDiv OpSDiv OpFDiv OpUMod OpSRem OpSMod OpFRem OpFMod '
    'OpConvertFToU OpConvertFToS OpConvertSToF OpConvertUToF OpUConvert OpSConvert OpFConvert '
    'OpBitcast OpLogicalEqual OpLogicalNotEqual OpLogicalOr OpLogicalAnd OpLogicalNot '
    'OpIEqual OpINotEqual OpUGreaterThan OpSGreaterThan OpUGreaterThanEqual OpSGreaterThanEqual '
    'OpULessThan OpSLessThan OpULessThanEqual OpSLessThanEqual '
    'OpFOrdEqual OpFUnordEqual OpFOrdNotEqual OpFUnordNotEqual OpFOrdLessThan OpFUnordLessThan '
    'OpFOrdGreaterThan OpFUnordGreaterThan OpFOrdLessThanEqual OpFUnordLessThanEqual '
    'OpFOrdGreaterThanEqual OpFUnordGreaterThanEqual OpShiftRightLogical OpShiftRightArithmetic '
    'OpShiftLeftLogical OpBitwiseOr OpBitwiseXor OpBitwiseAnd OpNot OpIsNan OpIsInf').split())


class LaneUses:
    def __init__(self, data, grammar):
        self.module = physical.inspector.reflection.Module(data)
        self.uses, self.types, self.instructions = defaultdict(list), {}, {}
        words = struct.unpack('<' + 'I' * (len(data)//4), data)
        cursor, inside = 5, False
        while cursor < len(words):
            count, op = words[cursor] >> 16, words[cursor] & 65535
            if not count or cursor + count > len(words):
                raise ValueError('invalid instruction framing')
            a = words[cursor+1:cursor+count]
            if op == 54:
                inside = True
            if inside:
                refs = grammar.references(op, a)
                desc = grammar.instructions[op]
                operands = desc.get('operands', [])
                result = (a[1] if len(operands) >= 2 and operands[0]['kind'] == 'IdResultType'
                          and operands[1]['kind'] == 'IdResult' else None)
                if result is not None:
                    self.types[result] = a[0]
                record = (desc['opname'], a, result, refs)
                self.instructions[cursor] = record
                for pos in refs:
                    self.uses[a[pos]].append((cursor, pos))
            if op == 56:
                inside = False
            cursor += count
        # Constants can be scalar operands of lane-wise operations.
        self.types.update({ident: typ for ident, (typ, _) in self.module.constants.items()})
        self.memo = {}

    def lanes(self, ident):
        typ = self.types.get(ident)
        op, a = self.module.types.get(typ, (None, ()))
        if op == 23:
            child, parts = self.module.types.get(a[0], (None, ()))
            return a[1] if child == 20 or child in (21, 22) and parts[0] == 32 else None
        return 1 if op == 20 or op in (21, 22) and a[0] == 32 else None

    def successors(self, ident, lane, record, pos):
        """None means an observable or unsupported use: retain the input lane."""
        name, a, result, refs = record
        n, out = self.lanes(ident), self.lanes(result)
        if n is None or lane >= n or result is None or out is None:
            return None
        if name == 'OpCopyObject' and pos == 2 and self.types[ident] == a[0]:
            return [(result, lane)]
        if name == 'OpCompositeExtract' and pos == 2 and len(a) == 4 and n > 1 and out == 1 and a[3] < n:
            return [(result, 0)] if lane == a[3] else []
        if name == 'OpVectorShuffle' and pos in (2, 3):
            left, right = self.lanes(a[2]), self.lanes(a[3])
            if left is None or right is None or len(a[4:]) != out:
                return None
            if any(x != 0xffffffff and x >= left + right for x in a[4:]):
                return None
            selected = lane + (left if pos == 3 else 0)
            return [(result, i) for i, x in enumerate(a[4:]) if x == selected]
        if name == 'OpCompositeConstruct' and pos >= 2:
            widths = [self.lanes(a[i]) for i in range(2, len(a))]
            if all(x is not None for x in widths) and sum(widths) == out:
                return [(result, sum(widths[:pos-2]) + lane)]
        if name in LANEWISE and pos >= 2 and refs == set(range(2, len(a))):
            if all(self.lanes(a[i]) == out for i in refs):
                return [(result, lane)]
        if name == 'OpSelect' and refs == {2, 3, 4} and self.lanes(a[3]) == out and self.lanes(a[4]) == out:
            cond = self.lanes(a[2])
            if cond in (1, out):
                return ([(result, i) for i in range(out)] if pos == 2 and cond == 1
                        else [(result, lane)])
        return None

    def consumed(self, ident, lane, visiting=None):
        key = ident, lane
        if key in self.memo:
            return self.memo[key]
        visiting = set() if visiting is None else visiting
        if key in visiting or len(visiting) >= 256:
            return True  # No proof of non-observation through cycles/deep graphs.
        visiting.add(key)
        value = False
        for cursor, pos in self.uses.get(ident, []):
            following = self.successors(ident, lane, self.instructions[cursor], pos)
            if following is None or any(self.consumed(i, l, visiting) for i, l in following):
                value = True
                break
        visiting.remove(key)
        self.memo[key] = value
        return value


def mapping_summary(mapping):
    return dict(word_count=len(mapping), mismatch_words=[m['byte_offset'] for m in mapping
        if m['status'] == 'changed_or_missing'], unknown_words=[m['byte_offset'] for m in mapping
        if m['status'] == 'unmapped_or_ambiguous'], changed_or_missing_members=sorted({
        m['original_member']['name'] for m in mapping if m['status'] == 'changed_or_missing'}))


def conservative_check():
    """Focused graph check: ignored padding versus unknown/dynamic/cyclic uses."""
    from types import SimpleNamespace
    grammar = physical.inspector.Grammar()
    assert grammar.references(81, (1, 99, 10, 3)) == {2}  # 3 is a literal.
    graph = LaneUses.__new__(LaneUses)
    graph.module = SimpleNamespace(types={1: (22, (32,)), 2: (23, (1, 4)), 3: (23, (1, 3))})
    graph.types = {10: 2, 11: 3, 12: 1, 13: 2}
    graph.uses, graph.instructions, graph.memo = defaultdict(list), {}, {}

    def add(word, name, operands, result, refs):
        graph.instructions[word] = name, operands, result, refs
        for pos in refs:
            graph.uses[operands[pos]].append((word, pos))

    # Same input in both shuffle slots exercises duplicate operand ID uses.
    add(1, 'OpVectorShuffle', (3, 11, 10, 10, 0, 1, 2), 11, {2, 3})
    add(2, 'OpUnknownConsumer', (11,), None, {0})
    assert [graph.consumed(10, i) for i in range(4)] == [True, True, True, False]
    graph.memo.clear()
    add(3, 'OpVectorExtractDynamic', (1, 12, 10, 13), 12, {2, 3})
    assert all(graph.consumed(10, i) for i in range(4))
    graph.uses, graph.instructions, graph.memo = defaultdict(list), {}, {}
    add(4, 'OpCopyObject', (2, 13, 10), 13, {2})
    add(5, 'OpCopyObject', (2, 10, 13), 10, {2})
    assert all(graph.consumed(10, i) for i in range(4))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--evidence', type=Path)
    parser.add_argument('--self-check', action='store_true')
    args = parser.parse_args()
    if args.self_check:
        conservative_check()
        print('conservative lane-use graph check passed')
        if args.output is None and args.evidence is None:
            return
    if args.output is None:
        parser.error('--output is required unless running only --self-check')
    if ROOT.joinpath('generated').resolve() not in args.output.resolve().parents:
        parser.error('output must be under porting/generated')
    evidence_path = ROOT / 'behavior/paper2d_view_lanes_runtime/evidence.json'
    if args.evidence and args.evidence.resolve() != evidence_path.resolve():
        parser.error('unexpected evidence destination')
    raw_report = PHYSICAL.read_bytes()
    report = json.loads(raw_report)
    manifest_raw, layout_raw = physical.MANIFEST.read_bytes(), physical.LAYOUT.read_bytes()
    grammar = physical.inspector.Grammar()
    expected = dict(manifest_sha256=physical.sha(manifest_raw), layout_sha256=physical.sha(layout_raw),
        grammar_sha256=grammar.sha256,
        inspector_sha256=physical.sha((HERE/'inspect_view_access.py').read_bytes()),
        analyzer_sha256=physical.sha((HERE/'paper2d_view_accesses.py').read_bytes()))
    if report['schema'] != 'paper2d-view-physical-access-v1' or any(report[k] != v for k, v in expected.items()):
        raise ValueError('stale physical report provenance')
    rows = {r['archive_index']: r for r in json.loads(manifest_raw)['shaders']}
    if set(rows) != {r['shader_index'] for r in report['records']}:
        raise ValueError('physical/manifest shader set mismatch')
    records = []
    for source in report['records']:
        index = source['shader_index']
        row = rows[index]
        data = Path(row['mapped']['output']).read_bytes()
        if not row['mapped']['validated'] or physical.sha(data) != row['mapped']['output_sha256'] or physical.sha(data) != source['spirv_sha256']:
            raise ValueError('mapped module provenance mismatch: %d' % index)
        record = dict(shader_index=index, physical_status=source['status'], spirv_sha256=source['spirv_sha256'])
        if 'view_descriptor' not in source:
            record.update(status='view_not_identified_in_wrapper', loads=[])
        else:
            # Re-run pointer inventory rather than trusting unverified report payloads.
            accesses, issues = physical.inspect(data, source['view_descriptor']['variable_id'], grammar)
            if accesses != source['accesses'] or issues != source['issues']:
                raise ValueError('physical access inventory mismatch')
            if issues:
                raise ValueError('unsupported physical inventory: %d' % index)
            uses, loads, consumed = LaneUses(data, grammar), [], set()
            for access in accesses:
                name, operands, result, _ = uses.instructions[access['instruction_word']]
                if name != 'OpLoad' or uses.lanes(result) != len(access['word_offsets']):
                    raise ValueError('unsupported load shape')
                kept = [lane for lane in range(len(access['word_offsets'])) if uses.consumed(result, lane)]
                offsets = [access['word_offsets'][lane] * 4 for lane in kept]
                consumed.update(offsets)
                loads.append(dict(instruction_word=access['instruction_word'], result_id=result,
                    physical_byte_offsets=[w*4 for w in access['word_offsets']], consumed_lanes=kept,
                    consumed_byte_offsets=offsets))
            mapping = source['word_mapping']
            live_mapping = [m for m in mapping if m['byte_offset'] in consumed]
            if len(live_mapping) != len(consumed):
                raise ValueError('consumed word missing physical mapping')
            ok = all(m['status'] == 'same_offset_and_shape' for m in live_mapping)
            record.update(status='consumed_words_same_offset_and_shape' if ok else 'consumed_layout_mismatch_or_unmapped',
                loads=loads, physical=mapping_summary(mapping), consumed=mapping_summary(live_mapping),
                consumed_word_mapping=live_mapping,
                discarded_byte_offsets=sorted({m['byte_offset'] for m in mapping} - consumed))
        records.append(record)
    provenance = dict(physical_report_sha256=physical.sha(raw_report), **expected)
    provenance['physical_analyzer_sha256'] = provenance.pop('analyzer_sha256')
    provenance['lane_analyzer_sha256'] = physical.sha(Path(__file__).read_bytes())
    result = dict(schema='paper2d-view-consumed-lanes-v1', provenance=provenance,
        runtime_verified=False, scope=__doc__, status_counts=dict(Counter(r['status'] for r in records)), records=records)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2)+'\n')
    if args.evidence:
        compact = []
        for r in records:
            item = {k: r[k] for k in ('shader_index', 'physical_status', 'status')}
            if 'physical' in r:
                item.update(physical_load_count=len(r['loads']), physical=r['physical'], consumed=r['consumed'],
                    discarded_word_count=len(r['discarded_byte_offsets']))
            compact.append(item)
        evidence = dict(schema='paper2d-view-consumed-lanes-evidence-v1',
            command='python3 porting/material_pipeline/paper2d_view_lanes.py --output porting/generated/paper2d-shader-semantics/view-lanes.json --evidence porting/behavior/paper2d_view_lanes_runtime/evidence.json',
            report_sha256=physical.sha(args.output.read_bytes()), provenance=provenance,
            runtime_verified=False, scope=__doc__, status_counts=result['status_counts'], shaders=compact)
        args.evidence.parent.mkdir(parents=True, exist_ok=True)
        args.evidence.write_text(json.dumps(evidence, indent=2)+'\n')
    print(result['status_counts'])


if __name__ == '__main__':
    main()
