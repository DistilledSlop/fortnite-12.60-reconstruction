import copy
import importlib.util
import json
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[2]
R = REPO / 'porting/generated/reflection'
spec = importlib.util.spec_from_file_location('fort_globals_policy', HERE / 'policy.py')
p = importlib.util.module_from_spec(spec)
spec.loader.exec_module(p)


def fixture():
    c = p.CONTRACT
    model = {'name':'UFortGlobals','block_va':c['class_block_va'], 'package':'/Script/FortniteGame',
             'parent':'UObject','functions':[], 'members_decl_order':[], 'members_table_order':[],
             'fields':[], 'helpers':[], 'static_helpers':[]}
    block = {k:c[k] for k in ('class_block_va','class_noregister_fn','base_fn','package_fn')}
    block['block_va'] = block.pop('class_block_va')
    block['functions'] = [copy.deepcopy(c['function'])]
    return model, block, {c['owner_fn']}, {p.NAME:copy.deepcopy(c['exec'])}


class PolicyTests(unittest.TestCase):
    def test_registration_and_definitions(self):
        a = fixture(); model = p.apply(*a); f = model['functions'][0]
        self.assertEqual((f['name'],f['ret'],f['static'],f['const'],f['args'],f['specifiers']),
                         ('IsErebus','bool',True,False,[],['BlueprintCallable','BlueprintPure']))
        self.assertTrue(f['runtime_implementation'])
        self.assertEqual(p.definitions(model), (HERE/'definitions.inc').read_text())
        with self.assertRaises(ValueError): p.apply(*a)
        other = {'name':'UOther'}
        self.assertIs(p.apply(other, {}, set(), None), other)
        self.assertEqual(p.definitions(other), '')

    def test_rejects_mismatches_and_collisions(self):
        mutations = [
            lambda a:a[0].update(package='/Script/Wrong'), lambda a:a[0].update(parent='UWrong'),
            lambda a:a[0].update(block_va='0x0'), lambda a:a[1].update(block_va='0x0'),
            lambda a:a[1].update(class_noregister_fn='0x0'), lambda a:a[1].update(base_fn='0x0'),
            lambda a:a[1].update(package_fn='0x0'), lambda a:a[2].clear(),
            lambda a:a[1]['functions'][0]['params'][0].update(property_flags='0x0'),
            lambda a:a[1]['functions'][0]['params'][0].update(set_bit_fn='0x0'),
            lambda a:a[1]['functions'][0].update(flags=['0x0']),
            lambda a:a[1]['functions'].append(copy.deepcopy(a[1]['functions'][0])),
            lambda a:a[3][p.NAME].update(exec_addr='0x0'),
            lambda a:a[3][p.NAME]['params'][0].update(element_size=2),
            lambda a:a[3].clear(),
        ]
        for group in ('functions','members_decl_order','members_table_order','fields','helpers','static_helpers'):
            mutations.append(lambda a,group=group:a[0][group].append({'name':'IsErebus'}))
        for mutate in mutations:
            a=fixture(); mutate(a)
            with self.subTest(mutate=mutate), self.assertRaises(ValueError): p.apply(*a)

    def test_reflection_snapshot(self):
        c=p.CONTRACT
        blocks=json.loads((R/'fortnite_game_reflection.json').read_text())['classes_in_groups_with_fort_names']
        block=next(x for x in blocks if x['block_va']==c['class_block_va'])
        parent=json.loads((R/'class_parent_map.json').read_text())[c['class_block_va']]
        ev=json.loads((R/'class_name_map.evidence.json').read_text())['blocks']
        entry=next(x for x in ev if x['block_va']==c['class_block_va'])
        owner={'0x'+x.removeprefix('0x').lower() for x in entry['construct_fns']}
        native=json.loads((R/'native_funcs.json').read_text())['UFortGlobals']
        model=fixture()[0]; model.update(package=parent['package'],parent=parent['parent_name'])
        self.assertEqual(parent['name'],'UFortGlobals')
        self.assertTrue(p.apply(model,block,owner,native)['fort_globals_erebus_runtime'])

    def test_generator_static_header_and_single_definition(self):
        path=REPO/'porting/gen_native_classes.py'
        spec=importlib.util.spec_from_file_location('erebus_generator_test', path)
        gen=importlib.util.module_from_spec(spec); spec.loader.exec_module(gen)
        model=p.apply(*fixture())
        model.update(uclass='UCLASS()',forward_decls=[],cpp_includes=[],splice={'accepted':[]})
        header=gen.render_promoted_header(model,'FORTNITEGAME_API','UObject.h','test')
        self.assertIn('UFUNCTION(BlueprintCallable, BlueprintPure)',header)
        self.assertIn('static bool IsErebus();',header)
        self.assertEqual(gen.render_promoted_cpp(model,'test').count('bool UFortGlobals::IsErebus()'), 1)

if __name__ == '__main__': unittest.main()
