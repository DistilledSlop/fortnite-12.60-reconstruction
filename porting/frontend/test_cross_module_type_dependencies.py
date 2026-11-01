import importlib.util
from pathlib import Path
from types import SimpleNamespace
import unittest

spec = importlib.util.spec_from_file_location('gen', Path(__file__).resolve().parents[1] / 'gen_native_classes.py')
gen = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gen)


class CrossModuleTypeDependencies(unittest.TestCase):
    def test_existing_dependency_direction_and_cycle_guard(self):
        ui, game = '/Script/FortniteUI', '/Script/FortniteGame'
        self.assertTrue(gen.cross_module_allowed(ui, game))
        self.assertFalse(gen.cross_module_allowed(game, ui))
        self.assertFalse(gen.cross_module_allowed('/Script/CommonUI', game))
        self.assertTrue(gen.cross_module_allowed(game, '/Script/CommonUI'))
        self.assertIsNone(gen.find_module_cycle({'FortniteUI': {'FortniteGame'},
            'FortniteGame': {'CommonUI'}, 'CommonUI': set()}, {'FortniteUI', 'FortniteGame', 'CommonUI'}))
        self.assertIsNotNone(gen.find_module_cycle({'FortniteUI': {'FortniteGame'},
            'FortniteGame': {'FortniteUI'}}, {'FortniteUI', 'FortniteGame'}))

    def test_only_reachable_foreign_structs_need_exports(self):
        game, ui = '/Script/FortniteGame', '/Script/FortniteUI'
        infos = {k: SimpleNamespace(fn=k, package=game, kind=kind) for k, kind in
            [('parent', 'struct'), ('value', 'struct'), ('unused', 'struct'), ('enum', 'enum')]}
        infos['child'] = SimpleNamespace(fn='child', package=ui, kind='struct')
        gate = SimpleNamespace(info=lambda k: infos.get(k))
        models = {k: {'used': {}} for k in infos}
        models['child']['used']['parent'] = {''}
        consumer = {'package': ui, 'used': {'value': {''}, 'enum': {''}}}
        self.assertEqual(gen.cross_module_struct_exports(gate, models, [consumer]), {'parent', 'value'})
        self.assertEqual(gen.cross_module_struct_exports(gate, {k: {'used': {}} for k in infos},
            [{'package': game, 'used': {'value': {''}}}]), set())

    def test_struct_export_does_not_change_declaration_or_parent(self):
        model = dict(cpp_name='FTestExport', parent='FTestParent', members=[], forward_decls=[])
        local = gen.render_struct_header(model, 'source.json')
        exported = gen.render_struct_header(dict(model, api='FORTNITEGAME_API '), 'source.json')
        self.assertEqual(local.replace('struct FTestExport :', 'struct FORTNITEGAME_API FTestExport :'), exported)


if __name__ == '__main__':
    unittest.main()
