#!/usr/bin/env python3
"""Reject View reads outside the proven field, including hidden pointer uses."""
import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location('view_access', Path(__file__).with_name('inspect_view_access.py'))
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


def instruction(opcode, *operands):
    return [((len(operands)+1) << 16) | opcode, *operands]


def shader(component=16, vector=False, lane=2, escape=False, dynamic=False):
    words = [0x07230203, 0x10000, 0, 64, 0]
    instructions = [
        (17, 1), (14, 0, 1), (15, 4, 10, 0x6e69616d, 0),
        (71, 5, 6, 16), (72, 6, 0, 35, 0),
        (22, 1, 32), (21, 2, 32, 0), (23, 3, 1, 4),
        (43, 2, 4, 142), (28, 5, 3, 4), (30, 6, 5),
        (19, 7), (33, 8, 7), (32, 9, 2, 6), (59, 9, 11, 2),
        (32, 12, 2, 3), (32, 13, 2, 1),
        (43, 2, 14, 0), (43, 2, 15, 141), (43, 2, 16, 2), (43, 2, 17, 1),
        (54, 7, 10, 0, 8), (248, 19),
        (65, 12, 20, 11, 14, 63 if dynamic else 15),
    ]
    if vector:
        instructions += [(61, 3, 23, 20), (79, 3, 24, 23, 23, lane, lane, lane, lane)]
        if escape:
            instructions += [(83, 3, 25, 23)]  # OpCopyObject hides raw View lanes.
    else:
        instructions += [(66, 13, 21, 20, component), (61, 1, 22, 21)]
        if escape:
            instructions += [(83, 13, 25, 21)]  # Pointer alias must be rejected.
    instructions += [(253,), (56,)]
    for item in instructions:
        words += instruction(*item)
    return struct.pack('<'+'I'*len(words), *words)


class ViewAccessTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.grammar = module.Grammar()

    def test_exact_scalar_game_time(self):
        result = module.inspect_access(shader(), 11, self.grammar)
        self.assertEqual([r['byte_offset'] for r in result['accesses']], [2264])

    def test_other_scalar_field_rejected(self):
        with self.assertRaisesRegex(ValueError, 'proven GameTime'):
            module.inspect_access(shader(component=17), 11, self.grammar)

    def test_dynamic_index_rejected(self):
        with self.assertRaisesRegex(ValueError, 'constant'):
            module.inspect_access(shader(dynamic=True), 11, self.grammar)

    def test_pointer_alias_rejected(self):
        with self.assertRaisesRegex(ValueError, 'pointer escapes'):
            module.inspect_access(shader(escape=True), 11, self.grammar)

    def test_float4_load_only_observes_selected_field(self):
        result = module.inspect_access(shader(vector=True), 11, self.grammar)
        self.assertEqual(result['accesses'][0]['physical_load_bytes'], 16)
        self.assertEqual(result['accesses'][0]['byte_offset'], 2264)
        with self.assertRaisesRegex(ValueError, 'unproven field'):
            module.inspect_access(shader(vector=True, lane=1), 11, self.grammar)

    def test_float4_cannot_escape_through_copy(self):
        with self.assertRaisesRegex(ValueError, 'float4 value escapes'):
            module.inspect_access(shader(vector=True, escape=True), 11, self.grammar)

    def test_grammar_distinguishes_literal_from_id(self):
        # ExtInst instruction number and shuffle selectors can numerically
        # equal pointer IDs without referring to those pointers.
        self.assertEqual(self.grammar.references(12, (1, 99, 30, 20, 22)), {2, 4})
        self.assertEqual(self.grammar.references(79, (3, 24, 23, 23, 2, 2, 2, 2)), {2, 3})


if __name__ == '__main__':
    unittest.main()
