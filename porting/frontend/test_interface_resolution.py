import importlib.util
import unittest
from pathlib import Path

spec = importlib.util.spec_from_file_location('gen', str(Path(__file__).resolve().parents[1] / 'gen_native_classes.py'))
gen = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gen)

class InterfaceResolution(unittest.TestCase):
    def test_only_emitted_real_interfaces_get_construct_and_thunk_aliases(self):
        names = {'0xa': 'UActual', '0xb': 'UOrdinary', '0xc': 'UUnlisted', '0xd': 'UDerived'}
        blocks = {'0xa': {'class_flags': '0x4000'}, '0xb': {'class_flags': '0x0'},
                  '0xc': {'class_flags': '0x4000'}, '0xd': {'class_flags': '0x4000'}}
        parents = {b: {'parent_name': 'UInterface', 'package': '/Script/CommonUI'} for b in blocks}
        parents['0xd']['parent_name'] = 'UActual'
        aliases, packages = gen.build_interface_map(names, {'UActual', 'UOrdinary', 'UDerived'}, blocks, parents,
             {'0xa': ['0x11'], '0xb': ['0x22'], '0xc': ['0x33']},
             {'0xAA': '0xA', '0xBB': '0xB', '0xCC': '0xC', '0xDD': '0xD'})
        self.assertEqual(aliases, {'0x11': 'IActual', '0xaa': 'IActual'})
        self.assertEqual(packages, {'IActual': '/Script/CommonUI'})
        r = gen.Resolver({}, iface_map=aliases)
        self.assertEqual(r.ctype({'kind': 'Interface', 'type_refs': {'interface_class': '0xAA'}}, {}), 'TScriptInterface<IActual>')
        self.assertTrue(r.ctype({'kind': 'Interface', 'type_refs': {'interface_class': '0xBB'}}, {}).startswith('TODO_Interface_'))
    def test_includes_and_module_for_all_model_kinds(self):
        for kind in ('class', 'struct', 'delegate'):
            model = {'forward_decls': ['IActual', 'UOther'], 'includes': ['Base.h']}
            self.assertEqual(gen.include_interface_headers(model, {'IActual': '/Script/CommonUI'}), ['CommonUI'])
            self.assertEqual(model['includes'], ['Base.h', 'UActual.h'])
            self.assertEqual(model['forward_decls'], ['UOther'])
    def test_cross_module_gate(self):
        r = gen.Resolver({}, iface_map={'0xaa': 'IActual'})
        r.gate = type('Gate', (), {'iface_packages': {'IActual': '/Script/CommonUI'}, 'current_package': '/Script/FortniteGame'})()
        entry = {'kind': 'Interface', 'type_refs': {'interface_class': '0xaa'}}
        self.assertEqual(r.ctype(entry, {}), 'TScriptInterface<IActual>')
        r.gate.current_package = '/Script/CommonInput'
        self.assertTrue(r.ctype(entry, {}).startswith('TODO_InterfaceOtherModule_'))

if __name__ == '__main__': unittest.main()
