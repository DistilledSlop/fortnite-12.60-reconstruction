import copy
import importlib.util
import json
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
R = ROOT / 'porting/generated/reflection'
s = importlib.util.spec_from_file_location('priority_policy', HERE / 'policy.py')
p = importlib.util.module_from_spec(s); s.loader.exec_module(p)
C = p.CONTRACT

class Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.block = next(b for b in json.loads((R/'fortnite_game_reflection.json').read_text())['classes_in_groups_with_fort_names'] if b['block_va'] == C['identity']['block_va'])
        cls.names = {v:k for k,v in json.loads((R/'class_name_map.json').read_text()).items()}
        cls.native = json.loads((R/'native_funcs.json').read_text())
        cls.enums = json.loads((R/'enum_blocks.json').read_text())['enums']

    def model(self):
        return {'name':'UFortGlobalUIContext','package':'/Script/FortniteUI','parent':'UFortLocalPlayerSubsystem','block_va':C['identity']['block_va'], 'functions':[], 'members_decl_order':[], 'members_table_order':[], 'fields':[], 'helpers':[], 'static_helpers':[], 'used':{}}

    def test_sources_and_rejection(self):
        blocks = {self.block['block_va']: self.block}
        p.validate_classes(blocks, self.names, self.native, self.enums)
        bad = copy.deepcopy(self.block); next(x for x in bad['functions'] if x['name']==p.NAME)['params'][0]['offset']=0
        with self.assertRaises(ValueError): p.validate_classes({bad['block_va']:bad},self.names,self.native,self.enums)
        bad = copy.deepcopy(self.enums); next(x for x in bad if x['name']=='EInputPriority')['enumerators'][5]['value']=0
        with self.assertRaises(ValueError): p.validate_classes(blocks,self.names,self.native,bad)
        p.validate_original_image('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')

    def test_model_and_render(self):
        model = p.apply(self.model(), self.block, {C['owner_fn']}, self.native['UFortGlobalUIContext'])
        self.assertEqual(model['used']['0x1451ecbb0'], {'Enum'})
        with self.assertRaises(ValueError): p.apply(model,self.block,{C['owner_fn']},self.native['UFortGlobalUIContext'])
        with self.assertRaises(ValueError): p.apply(self.model(),self.block,{C['owner_fn']},{})
        spec=importlib.util.spec_from_file_location('generator', ROOT/'porting/gen_native_classes.py'); g=importlib.util.module_from_spec(spec); spec.loader.exec_module(g)
        model.update(uclass='UCLASS()',forward_decls=[],cpp_includes=[],includes=['EInputPriority.h'],splice={'accepted':[]})
        header=g.render_promoted_header(model,'FORTNITEUI_API','UFortLocalPlayerSubsystem.h','test')
        cpp=g.render_promoted_cpp(model,'test')
        enum=(ROOT/'porting/native/Source/FortniteUI/Generated/EInputPriority.h').read_text()
        p.validate_output(model,header,cpp,enum)
        with self.assertRaises(ValueError): p.validate_output(model,header,cpp.replace('static_cast<uint8>', 'static_cast<int8>'),enum)
        with self.assertRaises(ValueError): p.validate_output(model,header,cpp,enum.replace('Error = 5','Error = 4'))

if __name__=='__main__': unittest.main()
