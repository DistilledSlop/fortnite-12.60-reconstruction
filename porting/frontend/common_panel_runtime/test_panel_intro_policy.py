import copy
import importlib.util
from pathlib import Path
import unittest

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('intro_policy', HERE / 'panel_intro_policy.py')
p = importlib.util.module_from_spec(spec)
spec.loader.exec_module(p)


class PanelIntroPolicy(unittest.TestCase):
    def panel(self):
        return {'name': 'UCommonActivatablePanel', 'package': '/Script/CommonUI', 'functions': []}

    def block(self):
        return {'block_va': '0x145a00360', 'functions': copy.deepcopy(list(p.EXPECTED.values()))}

    def apply(self, model=None, block=None, owners=None):
        return p.apply(self.panel() if model is None else model,
                       self.block() if block is None else block,
                       {'0x140807980'} if owners is None else owners)

    def test_model_shape_and_module(self):
        m = self.apply()
        self.assertEqual([(f['name'], f['access'], f['specifiers'], f['blueprint_event'], f['runtime_implementation'], f['args']) for f in m['functions']], [
            ('OnBeginIntro', 'protected', ['BlueprintNativeEvent'], True, False, []),
            ('EndIntro', 'protected', ['BlueprintCallable'], False, True, []),
            ('OnInputModeChanged', 'protected', ['BlueprintImplementableEvent'], True, False, ['bool bUsingGamepad'])])
        self.assertEqual(m['runtime_module_dependencies'], ['CommonInput'])
        self.assertIn('UCommonInputManager', m['forward_decls'])
        self.assertIn('CommonPanelActivationNative.h', p.header_prelude(m)['includes'])
        self.assertTrue(p.declarations(m)); self.assertTrue(p.definitions(m))

    def test_original_signatures(self):
        for name, expected in p.EXPECTED.items():
            for field in expected:
                b = self.block()
                f = next(f for f in b['functions'] if f['name'] == name)
                f[field] = 'invalid mutation'
                with self.subTest(name=name, field=field), self.assertRaises(ValueError):
                    self.apply(block=b)
            for kind in ('missing', 'duplicate'):
                b = self.block()
                if kind == 'missing': b['functions'] = [f for f in b['functions'] if f['name'] != name]
                else: b['functions'].append(copy.deepcopy(expected))
                with self.subTest(name=name, kind=kind), self.assertRaises(ValueError): self.apply(block=b)

    def test_panel_identity_and_collisions(self):
        for field, value in [('name', 'UOther'), ('package', '/Script/Other')]:
            m = self.panel(); m[field] = value
            if field == 'name': self.assertIs(p.apply(m, {}, set()), m)
            else:
                with self.assertRaises(ValueError): self.apply(model=m)
        b = self.block(); b['block_va'] = 'wrong'
        with self.assertRaises(ValueError): self.apply(block=b)
        with self.assertRaises(ValueError): self.apply(owners=set())
        for group in ('functions', 'members_decl_order', 'fields', 'helpers'):
            for name in ('bNativeIsIntroed', 'NativeActivateRequest', 'NativeInputManager', 'NativeInputMethodHandle', 'NativePreActivation', 'NativePostActivation', 'NativeHandleActivated', 'NativeIntroComplete', 'NativeInputMethodChangedHook', 'NativeGetGlobalInputPriorityFilter', 'NativeInputMethodChanged'):
                m = self.panel(); m[group] = [{'name': name}]
                with self.subTest(group=group, name=name), self.assertRaises(ValueError): self.apply(model=m)
        for name in p.EXPECTED:
            m = self.panel(); m['functions'] = [{'name': name}]
            with self.subTest(name=name), self.assertRaises(ValueError): self.apply(model=m)
        m = self.panel(); m['panel_intro_activation_runtime'] = True
        with self.assertRaises(ValueError): self.apply(model=m)

    def test_input_owners(self):
        for name, package, va, owner, native in [
            ('UCommonInputSubsystem', '/Script/CommonInput', '0x145a0e0d0', '0x14081e010', 'NativeInputMethodChanged'),
            ('UCommonGlobalInputHandler', '/Script/CommonUI', '0x145a058e0', '0x14080fa30', 'NativePriorityFilter')]:
            def model(): return {'name': name, 'package': package, 'functions': []}
            def block(): return copy.deepcopy(p.INPUT_OWNERS[va])
            m = p.apply(model(), block(), {owner})
            self.assertEqual(m['panel_activation_input_native'], name)
            self.assertTrue(p.declarations(m))
            if name == 'UCommonInputSubsystem':
                self.assertIn('CommonInputMethodNative.h', p.header_prelude(m)['includes'])
            for bad_model, bad_block, owners in [({**model(), 'package': '/Script/Other'}, block(), {owner}), (model(), {**block(), 'block_va': 'wrong'}, {owner}), (model(), {**block(), 'properties': None}, {owner}), (model(), block(), set())]:
                with self.subTest(name=name), self.assertRaises(ValueError): p.apply(bad_model, bad_block, owners)
            for group in ('functions', 'members_decl_order', 'fields', 'helpers'):
                bad = model(); bad[group] = [{'name': native}]
                with self.subTest(name=name, group=group), self.assertRaises(ValueError): p.apply(bad, block(), {owner})
            bad = model(); bad['panel_activation_input_native'] = name
            with self.assertRaises(ValueError): p.apply(bad, block(), {owner})

if __name__ == '__main__': unittest.main()
