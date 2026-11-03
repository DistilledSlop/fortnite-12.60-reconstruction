import copy
import importlib.util
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('strict_panel', HERE / 'policy.py')
p = importlib.util.module_from_spec(spec)
spec.loader.exec_module(p)


class PanelPolicy(unittest.TestCase):
    def model(self):
        return {'name': 'UCommonActivatablePanel', 'parent': 'UCommonActivatableWidget',
                'package': '/Script/CommonUI', 'functions': [], 'panel_outro_runtime': True}

    def fixtures(self):
        return ({p.CONTRACT['block_va']: {'block_va': p.CONTRACT['block_va'],
                    'functions': copy.deepcopy(list(p.CONTRACT['functions'].values()))}},
                {'UCommonActivatablePanel': p.CONTRACT['block_va']},
                {'UCommonActivatablePanel': copy.deepcopy(p.CONTRACT['native_execs'])})

    def test_original(self):
        p.validate_original_image('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
        p.validate_continuation_original_image('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
        p.validate_classes(*self.fixtures())

    def test_all_function_and_exec_records_reject_mutation(self):
        for name in p.NAMES:
            blocks, mapping, native = self.fixtures()
            next(f for f in blocks[p.CONTRACT['block_va']]['functions'] if f['name'] == name)['create_func'] = 'wrong'
            with self.subTest(name=name), self.assertRaises(ValueError):
                p.validate_classes(blocks, mapping, native)
            blocks, mapping, native = self.fixtures()
            native['UCommonActivatablePanel'][name]['exec_addr'] = 'wrong'
            with self.subTest(exec=name), self.assertRaises(ValueError):
                p.validate_classes(blocks, mapping, native)

    def test_dependencies_collisions_and_splice(self):
        model = self.model()
        p.apply(model, {'block_va': p.CONTRACT['block_va']}, {p.CONTRACT['owner']})
        self.assertIn('FCommonActionCommited.h', p.header_prelude(model)['includes'])
        header = p.finalize_header(model, '    FCommonPanelActionMap NativeActionRecords;\n' + p.declarations(model))
        p.validate_output(model, header, p.definitions(model))
        with self.assertRaises(ValueError):
            p.validate_output(model, p.declarations(model), '')
        with self.assertRaises(ValueError):
            p.apply(model, {'block_va': p.CONTRACT['block_va']}, {p.CONTRACT['owner']})
        model = self.model()
        model['panel_outro_runtime'] = False
        with self.assertRaises(ValueError):
            p.apply(model, {'block_va': p.CONTRACT['block_va']}, {p.CONTRACT['owner']})
        with self.assertRaises(ValueError):
            p.validate_dependencies({})


if __name__ == '__main__':
    unittest.main()
