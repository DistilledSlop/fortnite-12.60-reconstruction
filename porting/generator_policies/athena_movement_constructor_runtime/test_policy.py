import unittest
import source_policy as p


class PolicyTest(unittest.TestCase):
    def setUp(self):
        self.model = {'name': p.CLASS, 'parent': 'UFortMovementComp_Character',
                      'members_decl_order': [{'name': d.split()[-1][:-1], 'decl': d} for d in p.OWN]}
        self.header = 'class X {\npublic:\n' + '\n'.join(p.OWN) + '\n};\n'
        self.cpp = '#include "test.h"\n'

    def test_creates_constructor_and_preserves_array_values(self):
        h, c = p.finalize(self.model, self.header, self.cpp)
        self.assertEqual(h.count('UFortMovementComp_CharacterAthena('), 1)
        self.assertEqual(c.count('JumpPenalties['), 10)
        self.assertIn('JumpPenalties[4].MovementScalar = 0.25f', c)
        self.assertIn('BrakingSubStepTime = 1.0f / 60.0f', c)

    def test_merges_existing_constructor_and_rejects_conflicts(self):
        h, c = p.finalize(self.model, self.header, self.cpp)
        with self.assertRaises(ValueError): p.finalize(self.model, h, c)
        h2, c2 = p.finalize(self.model, h, c.replace('    // Reflected-field projection', '    ExistingSideEffect();\n    // Reflected-field projection').split('    // Reflected-field projection')[0] + '}\n')
        self.assertIn('ExistingSideEffect();', c2)

    def test_rejects_changed_field_and_parent(self):
        with self.assertRaises(ValueError): p.finalize(self.model, self.header.replace('float MaxAccelerationFlying;', 'double MaxAccelerationFlying;'), self.cpp)
        self.model['parent'] = 'UOther'
        with self.assertRaises(ValueError): p.finalize(self.model, self.header, self.cpp)

    def test_other_class_untouched(self):
        self.assertEqual(p.finalize({'name': 'UOther'}, self.header, self.cpp), (self.header, self.cpp))

if __name__ == '__main__': unittest.main()
