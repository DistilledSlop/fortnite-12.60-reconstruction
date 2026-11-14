"""Behavioral tests for constructor read/modify/write bit evidence."""
import unittest
from unittest.mock import patch

import ctor_defaults as scanner


class ConstructorRmwTests(unittest.TestCase):
    def scan(self, code):
        code = bytes.fromhex(code + ' c3')
        with patch.object(scanner, 'insns', side_effect=lambda va, limit: scanner.md.disasm(code, va)):
            return scanner.scan_ctor(0x1000)

    def bits(self, code):
        bits = [{'name': 'first', 'offset': 0x20, 'mask': 1},
                {'name': 'second', 'offset': 0x20, 'mask': 2},
                {'name': 'third', 'offset': 0x20, 'mask': 4}]
        return scanner.resolve('C', self.scan(code), {}, {'C': bits})

    def test_dword_preserves_unknown_and_disjoint_write(self):
        # mov eax,[rcx+20]; and eax,~6; mov [rcx+30],dl; or eax,1; mov [rcx+20],eax
        stores = self.scan('8b 41 20 83 e0 f9 88 51 30 83 c8 01 89 41 20')
        self.assertEqual([s[2:4] for s in stores], [('and', 0xfffffff9), ('or', 1)])
        self.assertEqual(self.bits('8b 41 20 83 e0 f9 88 51 30 83 c8 01 89 41 20')['first']['value'], True)

    def test_byte_does_not_claim_unmodified_neighbors_or_scalar(self):
        code = '8a 41 20 0c 01 88 41 20'
        stores = self.scan(code)
        props = {'C': [{'name': 'byte', 'offset': 0x20, 'kind': 'Byte'}]}
        self.assertEqual(scanner.resolve('C', stores, props, {}), {})
        self.assertEqual(set(self.bits(code)), {'first'})

    def test_rejects_clobbers_and_mismatched_targets(self):
        cases = [
            '8a 41 20 0c 01 b0 00 88 41 20',  # al clobber
            '8a 41 20 0c 01 66 b8 00 00 88 41 20',  # overlapping ax
            '8a 41 20 0c 01 88 41 21',  # different displacement
            '8a 41 20 0c 01 88 42 20',  # different base
            '8a 41 20 0c 01 c6 41 20 00 88 41 20',  # source overwritten
            '8a 41 20 0c 01 74 00 88 41 20',  # branch
            '8a 41 20 0c 01 e8 00 00 00 00 88 41 20',  # call
            '8a 41 20 0c 01 88 61 20',  # ah is not al
        ]
        for code in cases:
            with self.subTest(code=code):
                self.assertFalse(any(s[2] in ('and', 'or') for s in self.scan(code)))

    def test_masked_write_invalidates_earlier_overlapping_scalars(self):
        # Initial byte 2, then register RMW sets bit 0. Neither a prior byte nor
        # a wider integer covering that byte is allowed to survive as value 2.
        code = 'c6 41 20 02 8a 41 20 0c 01 88 41 20'
        stores = self.scan(code)
        for kind in ('Byte', 'Int'):
            with self.subTest(kind=kind):
                props = {'C': [{'name': 'scalar', 'offset': 0x20, 'kind': kind}]}
                self.assertNotIn('scalar', scanner.resolve('C', stores, props, {}))
        self.assertEqual(set(self.bits(code)), {'first', 'second'})

        # A direct memory mask has the same scalar invalidation requirement.
        stores = self.scan('c6 41 20 02 80 49 20 01')
        props = {'C': [{'name': 'scalar', 'offset': 0x20, 'kind': 'Byte'}]}
        self.assertNotIn('scalar', scanner.resolve('C', stores, props, {}))

    def test_partial_masked_write_invalidates_full_integer(self):
        # mov dword [rcx+20],2; or byte [rcx+21],1
        stores = self.scan('c7 41 20 02 00 00 00 80 49 21 01')
        props = {'C': [{'name': 'scalar', 'offset': 0x20, 'kind': 'Int'}]}
        self.assertNotIn('scalar', scanner.resolve('C', stores, props, {}))

    def test_alias_clobber_and_non_this_source(self):
        for code in ('8a 42 20 0c 01 88 41 20', '8a 41 20 0c 01 b1 00 88 41 20'):
            with self.subTest(code=code):
                self.assertFalse(any(s[2] in ('and', 'or') for s in self.scan(code)))


if __name__ == '__main__':
    unittest.main()
