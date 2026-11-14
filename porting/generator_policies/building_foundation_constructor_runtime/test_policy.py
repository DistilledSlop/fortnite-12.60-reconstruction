import re
import unittest
import source_policy as p


class PolicyTest(unittest.TestCase):
    def setUp(self):
        self.model = {'name': p.CLASS, 'parent': 'ABuildingSMActor', 'members_decl_order': [
            {'name': re.search(r'\b(\w+)\s*(?::\s*1)?\s*;$', d).group(1), 'decl': d} for d in p.OWN]}
        self.header = 'class X {\npublic:\n' + '\n'.join(p.OWN) + '\n};\n'
        self.cpp = '#include "test.h"\n'

    def test_new_constructor(self):
        h, c = p.finalize(self.model, self.header, self.cpp)
        self.assertEqual(h.count('ABuildingFoundation('), 1)
        self.assertIn('bConditionalFoundation = false;', c)
        self.assertIn('bOverrideNavigationGraphCells = true;', c)
        self.assertIn('bHLODMergeUseCustomTextureSize = false;', c)
        self.assertIn('SnapOffset.Y = 0.0f;', c)
        self.assertNotIn('SnapOffset.Z =', c)

    def test_preserves_subobjects_and_rejects_conflicts(self):
        h = self.header.replace('public:', 'public:\nABuildingFoundation(const FObjectInitializer& ObjectInitializer);')
        c = self.cpp + '\nABuildingFoundation::ABuildingFoundation(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {\nRootComponent = CreateThing();\n}\n'
        h, out = p.finalize(self.model, h, c)
        self.assertLess(out.index('RootComponent = CreateThing()'), out.index('FoundationType = BFT_None'))
        h2, out2 = p.finalize(self.model, h, c.replace('RootComponent =', 'FoundationType = BFT_None;\nRootComponent ='))
        self.assertEqual(out2.count('FoundationType = BFT_None;'), 1)
        with self.assertRaises(ValueError): p.finalize(self.model, h, c.replace('RootComponent =', 'FoundationType = BFT_5x5;\nRootComponent ='))

    def test_rejects_layout_and_parent_changes(self):
        with self.assertRaises(ValueError): p.finalize(self.model, self.header.replace('float ProxyMeshMaxDrawDistanceMultiplier;', 'double ProxyMeshMaxDrawDistanceMultiplier;'), self.cpp)
        self.model['parent'] = 'AOther'
        with self.assertRaises(ValueError): p.finalize(self.model, self.header, self.cpp)

    def test_other_class_untouched(self):
        self.assertEqual(p.finalize({'name':'AOther'}, self.header, self.cpp), (self.header, self.cpp))

if __name__ == '__main__': unittest.main()
