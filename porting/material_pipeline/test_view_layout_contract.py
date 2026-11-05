"""Reject incomplete member recovery and disagreement with original code tables."""
import copy
import json
import unittest

import view_layout_contract as contract


class ViewLayoutContractTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        chain = contract.EVIDENCE / 'member-chain.json'
        if not chain.exists():
            raise unittest.SkipTest('original metadata capture required')
        cls.chain = json.loads(chain.read_text())
        cls.initializer = (contract.EVIDENCE / '140555220.asm').read_text()

    def test_missing_metadata_field_is_rejected(self):
        chain = copy.deepcopy(self.chain)
        chain[0]['asm'] = [line for line in chain[0]['asm'] if 'MOV byte ptr [RSP + 0x34]' not in line]
        with self.assertRaisesRegex(ValueError, 'incomplete metadata member fields'):
            contract.original(chain, self.initializer)

    def test_original_resource_hash_matches_wrappers_and_rejects_offset_change(self):
        path = contract.ROOT / 'generated/paper2d-shader-semantics/conversion/d3d.json'
        if not path.exists():
            self.skipTest('original shader wrappers required')
        original = contract.original(self.chain, self.initializer)
        report = json.loads(path.read_text())
        associations = contract.verify_shader_views(original, report)
        self.assertTrue(any(row['resource_table_active'] for row in associations))
        primitive = next(member for member in original['members'] if member['name'] == 'PrimitiveSceneData')
        primitive['offset'] += 8
        with self.assertRaisesRegex(ValueError, 'differs from original shader wrapper'):
            contract.verify_shader_views(original, report)


if __name__ == '__main__':
    unittest.main()
