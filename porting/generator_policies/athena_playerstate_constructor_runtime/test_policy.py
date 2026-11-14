import unittest
import source_policy as p


class PolicyTest(unittest.TestCase):
    def setUp(self):
        self.model = {'name': p.CLASS, 'parent': 'AFortPlayerStateZone', 'members_decl_order': [
            {'name': __import__('re').search(r'\b(\w+)\s*(?::\s*1)?\s*;$', d).group(1), 'decl': d}
            for d in p.OWN]}
        self.header = 'class X {\npublic:\n' + '\n'.join(p.OWN) + '\n};\n'
        self.cpp = '#include "test.h"\n'

    def test_create_and_expected_assignments(self):
        h, c = p.finalize(self.model, self.header, self.cpp)
        self.assertEqual(h.count('AFortPlayerStateAthena('), 1)
        for statement in ('TeamScore = -1;', 'SecondsAlive = 0;', 'TimeOfPawnCreation = 0;',
                          'SetShouldUpdateReplicatedPing(false);', 'bIsAnAthenaGameParticipant = true;',
                          'bSkipReplicatedStats = true;', 'NetUpdateFrequency = 1.0f;'):
            self.assertIn(statement, c)
        self.assertNotIn('TeamIndex', c)
        self.assertNotIn('SquadId', c)

    def test_preserve_constructor_and_reject_competing_or_repeat(self):
        h = self.header.replace('public:', 'public:\nAFortPlayerStateAthena(const FObjectInitializer& ObjectInitializer);')
        c = self.cpp + '\nAFortPlayerStateAthena::AFortPlayerStateAthena(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) { ExistingSideEffect(); }\n'
        h, c = p.finalize(self.model, h, c)
        self.assertIn('ExistingSideEffect();', c)
        with self.assertRaises(ValueError): p.finalize(self.model, h, c)
        with self.assertRaises(ValueError): p.finalize(self.model, h, c.replace('ExistingSideEffect();', 'TeamScore = 7;'))

    def test_reject_drift_and_unmatched_constructor(self):
        with self.assertRaises(ValueError): p.finalize(self.model, self.header.replace('int32 TeamScore;', 'float TeamScore;'), self.cpp)
        with self.assertRaises(ValueError): p.finalize(self.model, self.header, self.cpp + 'AFortPlayerStateAthena::AFortPlayerStateAthena() {}')
        self.model['parent'] = 'AOther'
        with self.assertRaises(ValueError): p.finalize(self.model, self.header, self.cpp)

    def test_multiple_access_sections(self):
        h, _ = p.finalize(self.model, self.header.replace('};', 'private:\nint X;\npublic:\nvoid F();\n};'), self.cpp)
        self.assertEqual(h.count('AFortPlayerStateAthena('), 1)

    def test_other_class_untouched(self):
        self.assertEqual(p.finalize({'name': 'AOther'}, self.header, self.cpp), (self.header, self.cpp))

if __name__ == '__main__': unittest.main()
