import copy
import importlib.util
import json
from pathlib import Path
import sys
import unittest
import source_policy as p

PORTING = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PORTING))
import gen_native_classes as gen

class PolicyTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        reflection = json.loads((PORTING / 'generated/reflection/fortnite_game_reflection.json').read_text())
        names = json.loads((PORTING / 'generated/reflection/class_name_map.json').read_text())
        va = next(k for k, v in names.items() if v == p.CLASS)
        block = next(b for b in reflection['classes_in_groups_with_fort_names'] if b['block_va'] == va)
        parents = json.loads((PORTING / 'generated/reflection/class_parent_map.json').read_text())
        cls.model = gen.build_class(block, p.CLASS, gen.Resolver({'0x1429f7970': 'FAirControlParams'}), 'declaration', parents[va])
        cls.model['package'] = parents[va]['package']
        root = PORTING / 'native/Source/FortniteGame/Generated'
        cls.header = (root / (p.CLASS + '.h')).read_text()
        cls.cpp = (root / (p.CLASS + '.cpp')).read_text()
        spec = importlib.util.spec_from_file_location('air_ctor', PORTING / 'generator_policies/air_control_params_runtime/source_policy.py')
        cls.air = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(cls.air)

    def test_actual_reflection_model_and_generated_source(self):
        h, c = p.finalize(self.model, self.header, self.cpp)
        self.assertEqual(len(p.WRITES), 76)
        self.assertEqual(len({(f, m) for f, m, _, _ in p.WRITES}), 76)
        self.assertEqual(len(p.FIELDS) * len(p.MEMBERS) - len(p.WRITES), 36)
        self.assertEqual(c.count(' = FScalableFloat('), 76)
        self.assertIn('SkydivingControlParamsPassive.GravityScalar = FScalableFloat(1.0f)', c)
        self.assertIn('VortexParachuteControlParams.GravityScalar = FScalableFloat(-1.5f)', c)
        self.assertNotIn('BalloonControlParams.GravityScalar =', c)
        with self.assertRaises(ValueError): p.finalize(self.model, h, c)

    def test_composes_existing_constructor_and_rejects_conflicts(self):
        h = self.header.replace('public:', 'public:\n    UFortMovementComp_Character(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());', 1)
        c = self.cpp + '\nUFortMovementComp_Character::UFortMovementComp_Character(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)\n{\n    ExistingSubobject();\n}\n'
        h2, c2 = p.finalize(self.model, h, c)
        self.assertEqual(h2, h)
        self.assertLess(c2.index('ExistingSubobject();'), c2.index('SkydivingControlParamsPassive.MaxAcceleration ='))
        with self.assertRaises(ValueError): p.finalize(self.model, h, c.replace('ExistingSubobject();', 'SkydivingControlParamsPassive.GravityScalar.Value = 3;'))
        with self.assertRaises(ValueError): p.finalize(self.model, h, c.replace(': Super(ObjectInitializer)', ': Super(ObjectInitializer), SkydivingControlParamsPassive()'))

    def test_rejects_changed_layout(self):
        bad = copy.deepcopy(self.model)
        next(m for m in bad['members_decl_order'] if m['name'] == p.FIELDS[0][0])['offset'] += 4
        with self.assertRaises(ValueError): p.finalize(bad, self.header, self.cpp)
        with self.assertRaises(ValueError): p.finalize(self.model, self.header.replace('FAirControlParams BalloonControlParams;', 'int BalloonControlParams;'), self.cpp)
        bad = copy.deepcopy(self.model); bad['parent'] = 'UOther'
        with self.assertRaises(ValueError): p.finalize(bad, self.header, self.cpp)

    def test_struct_contract(self):
        model = {'cpp_name': 'FAirControlParams', 'package': '/Script/FortniteGame', 'size': 256,
                 'members': [{'offset': i * 32, 'decl': 'FScalableFloat %s;' % n} for i, n in enumerate(p.MEMBERS)]}
        declarations = self.air.declarations(model)
        p.validate_struct(model, declarations)
        with self.assertRaises(ValueError): p.validate_struct(model, [])
        model['members'][1]['offset'] += 4
        with self.assertRaises(ValueError): p.validate_struct(model, declarations)

    def test_other_class_untouched(self):
        self.assertEqual(p.finalize({'name': 'UOther'}, self.header, self.cpp), (self.header, self.cpp))

if __name__ == '__main__': unittest.main()
