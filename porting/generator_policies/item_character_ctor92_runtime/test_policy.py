import copy
import json
from pathlib import Path
import sys
import unittest
import source_policy as p

PORTING = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PORTING))
import gen_native_classes as gen

class TestPolicy(unittest.TestCase):
    def test_models(self):
        reflection = json.loads((PORTING / 'generated/reflection/fortnite_game_reflection.json').read_text())
        names = json.loads((PORTING / 'generated/reflection/class_name_map.json').read_text())
        parents = json.loads((PORTING / 'generated/reflection/class_parent_map.json').read_text())
        resolver = gen.Resolver({'0x142b241d0':'EFortCustomGender::Type', '0x142b24170':'EFortCustomBodyType::Type'})
        fixtures = PORTING / 'generated/frontend-super-events-1/Source/FortniteGame/Generated'
        for cls in p.FIELDS:
            va = next(k for k,v in names.items() if v == cls)
            block = next(b for b in reflection['classes_in_groups_with_fort_names'] if b['block_va'] == va)
            model = gen.build_class(block, cls, resolver, 'declaration', parents[va])
            h = (fixtures / (cls + '.h')).read_text(); c = (fixtures / (cls + '.cpp')).read_text()
            _, out = p.finalize(model, h, c)
            if cls in p.BODIES:
                for statement in p.BODIES[cls][1]: self.assertIn(statement, out)
                hh, cc = p.finalize(model,h,c)
                with self.assertRaises(ValueError): p.finalize(model,hh,cc)
            bad = copy.deepcopy(model)
            target = p.FIELDS[cls][1][0][0]
            next(m for m in bad['members_decl_order'] if m['name'] == target)['offset'] = -1
            with self.assertRaises(ValueError): p.finalize(bad,h,c)
if __name__ == '__main__': unittest.main()
