import unittest
import source_policy as p


class PolicyTest(unittest.TestCase):
    def setUp(self):
        self.model = {'name': p.CLASS, 'parent': 'AFortGameModeBase'}
        self.header = 'class FORTNITEGAME_API AFortGameMode : public AFortGameModeBase\n{\npublic:\n};\n'
        self.cpp = '#include "AFortGameMode.h"\n'

    def test_emits_typed_override_and_complete_class_include(self):
        h, c = p.finalize(self.model, self.header, self.cpp)
        self.assertIn('virtual TSubclassOf<AGameSession> GetGameSessionClass() const override;', h)
        self.assertIn('TSubclassOf<AGameSession> AFortGameMode::GetGameSessionClass() const', c)
        self.assertIn('return AFortGameSession::StaticClass();', c)
        self.assertEqual(c.count('#include "AFortGameSession.h"'), 1)
        with self.assertRaises(ValueError):
            p.finalize(self.model, h, c)

    def test_existing_include_and_conflicts(self):
        _, c = p.finalize(self.model, self.header, self.cpp + '#include "AFortGameSession.h"\n')
        self.assertEqual(c.count('#include "AFortGameSession.h"'), 1)
        with self.assertRaises(ValueError):
            p.finalize(self.model, self.header.replace('public:', 'public:\n void GetGameSessionClass();'), self.cpp)
        with self.assertRaises(ValueError):
            p.finalize(self.model, self.header, self.cpp + 'void AFortGameMode::GetGameSessionClass() {}')

    def test_rejects_unexpected_shape(self):
        with self.assertRaises(ValueError):
            p.finalize(dict(self.model, parent='AGameMode'), self.header, self.cpp)
        with self.assertRaises(ValueError):
            p.finalize(self.model, self.header.replace('public:', 'private:'), self.cpp)
        with self.assertRaises(ValueError):
            p.finalize(self.model, self.header, '')

    def test_other_class_untouched(self):
        self.assertEqual(p.finalize({'name': 'AFortGameModeAthena'}, self.header, self.cpp), (self.header, self.cpp))


if __name__ == '__main__':
    unittest.main()
