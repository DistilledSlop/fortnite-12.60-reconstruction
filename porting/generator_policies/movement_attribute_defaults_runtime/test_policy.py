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

class TestPolicy(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        refl = json.loads((PORTING / 'generated/reflection/fortnite_game_reflection.json').read_text())
        names = json.loads((PORTING / 'generated/reflection/class_name_map.json').read_text())
        va = next(k for k,v in names.items() if v == p.CLASS)
        block = next(b for b in refl['classes_in_groups_with_fort_names'] if b['block_va'] == va)
        parents = json.loads((PORTING / 'generated/reflection/class_parent_map.json').read_text())
        cls.model = gen.build_class(block, p.CLASS, gen.Resolver({'0x142841da0':'FFortGameplayAttributeData'}), 'declaration', parents[va])
        # Pin pre-integration inputs: the live generated files already contain this projection.
        root = Path(__file__).resolve().parent / 'fixtures'
        cls.header = (root / (p.CLASS + '.h')).read_text()
        cls.cpp = (root / (p.CLASS + '.cpp')).read_text()
        spec = importlib.util.spec_from_file_location('attribute', PORTING / 'generator_policies/fort_attribute_data_runtime/source_policy.py')
        cls.attribute = importlib.util.module_from_spec(spec); spec.loader.exec_module(cls.attribute)
    def test_actual_model(self):
        h,c = p.finalize(self.model, self.header, self.cpp)
        self.assertEqual(c.count('.SetBaseValue(1.0f)'), 4)
        self.assertEqual(c.count('.SetCurrentValue(1.0f)'), 4)
        with self.assertRaises(ValueError): p.finalize(self.model,h,c)
        bad = copy.deepcopy(self.model); bad['members_decl_order'][0]['offset'] += 4
        with self.assertRaises(ValueError): p.finalize(bad,self.header,self.cpp)
        with self.assertRaises(ValueError): p.finalize(self.model,self.header.replace('FFortGameplayAttributeData WalkSpeed;', 'float WalkSpeed;'),self.cpp)
    def test_struct_dependency(self):
        model = {'cpp_name':'FFortGameplayAttributeData'}
        decls = self.attribute.declarations(model)
        p.validate_struct(model,decls)
        with self.assertRaises(ValueError): p.validate_struct(model,decls[:-1])
    def test_compose_and_conflicts(self):
        h = self.header.replace('public:', 'public:\n    UFortMovementSet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());', 1)
        c = self.cpp + '\nUFortMovementSet::UFortMovementSet(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)\n{\n    ExistingSideEffect();\n}\n'
        _,out = p.finalize(self.model,h,c)
        self.assertLess(out.index('ExistingSideEffect();'),out.index('BackwardSpeedMultiplier.SetBaseValue'))
        with self.assertRaises(ValueError): p.finalize(self.model,h,c.replace('ExistingSideEffect();','WalkSpeed.SetBaseValue(2);'))
        with self.assertRaises(ValueError): p.finalize(self.model,h,c.replace(': Super(ObjectInitializer)',': Super(ObjectInitializer), WalkSpeed()'))
if __name__ == '__main__': unittest.main()
