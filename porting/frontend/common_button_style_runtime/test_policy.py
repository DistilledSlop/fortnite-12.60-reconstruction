import copy,importlib.util,json,unittest
from pathlib import Path
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]
spec=importlib.util.spec_from_file_location('button_style_policy',HERE/'policy.py')
p=importlib.util.module_from_spec(spec);spec.loader.exec_module(p)
C=p.CONTRACT

def models():
    values={name:{'name':name,'package':'/Script/CommonUI','members_decl_order':[{'name':n,'decl':d} for n,d in expected['members'].items()]} for name,expected in C['classes'].items()}
    values['UCommonButton'].update(common_button_query_runtime=True,common_button_text_style_runtime=True,functions=[])
    return values

def optional():return {'members':[{'name':n,'decl':d} for n,d in C['optional_sound_members'].items()]}
def internal():return (HERE.parent/'common_panel_runtime/button-declarations.inc').read_text()

class PolicyTests(unittest.TestCase):
    def test_authentic_records_and_model_declarations(self):
        R=ROOT/'porting/generated/reflection'
        all_blocks=json.loads((R/'fortnite_game_reflection.json').read_text())['classes_in_groups_with_fort_names']
        needed={e['identity']['block_va'] for e in C['classes'].values()}
        blocks={b['block_va']:b for b in all_blocks if b['block_va'] in needed}
        names={v:k for k,v in json.loads((R/'class_name_map.json').read_text()).items()}
        sound=next(b for b in json.loads((R/'struct_blocks.json').read_text())['structs'] if b['name']=='CommonButtonStyleOptionalSlateSound')
        p.validate_classes(blocks,names,sound)
        p.validate_models(models(),optional(),internal())
        for name in C['classes']:
            bad=copy.deepcopy(blocks);bad[names[name]]['base_fn']='0x0'
            with self.subTest(name=name),self.assertRaises(ValueError):p.validate_classes(bad,names,sound)
        bad=copy.deepcopy(sound);bad['properties'][1]['property_flags']='0x0'
        with self.assertRaises(ValueError):p.validate_classes(blocks,names,bad)
    def test_member_and_required_adapter_guards(self):
        for name,expected in C['classes'].items():
            for member in expected['members']:
                bad=models();next(m for m in bad[name]['members_decl_order'] if m['name']==member)['decl']='int32 Wrong;'
                with self.subTest(name=name,member=member),self.assertRaises(ValueError):p.validate_models(bad,optional(),internal())
        for missing in ('void NativeSetMinimumWidth(int32 InWidth);','void NativeSetMinimumHeight(int32 InHeight);'):
            with self.subTest(missing=missing),self.assertRaises(ValueError):p.validate_models(models(),optional(),internal().replace(missing,''))
    def test_identity_foundations_collisions_and_no_reflection_addition(self):
        block={'block_va':'0x145a01210'};button=models()['UCommonButton'];owner={'0x140805cb0'}
        result=p.apply(button,block,owner)
        self.assertTrue(result['common_button_style_rebuild_runtime']);self.assertEqual(result['functions'],[])
        self.assertNotIn('UFUNCTION',p.declarations(result));self.assertNotIn('SetStyle(',p.declarations(result))
        with self.assertRaises(ValueError):p.apply(result,block,owner)
        for flag in ('common_button_query_runtime','common_button_text_style_runtime'):
            bad=models()['UCommonButton'];bad[flag]=False
            with self.subTest(flag=flag),self.assertRaises(ValueError):p.apply(bad,block,owner)
        for group in ('functions','members_table_order','members_decl_order','fields','helpers','static_helpers'):
            for helper in p.HELPERS:
                bad=models()['UCommonButton'];bad.setdefault(group,[]).append({'name':helper})
                with self.subTest(group=group,helper=helper),self.assertRaises(ValueError):p.apply(bad,block,owner)
        other={'name':'UOther'};self.assertIs(p.apply(other,{},set()),other)
if __name__=='__main__':unittest.main()
