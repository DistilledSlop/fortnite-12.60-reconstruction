#!/usr/bin/env python3
"""Guard the source-evidenced parameterized Blueprint event without local assets."""
import copy
import importlib.util
import json
from pathlib import Path
import unittest

PORTING = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('generator', PORTING / 'gen_native_classes.py')
gen = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gen)

# Literal bounded transcription of original FFunctionParams/property records:
# see frontend/login_status_event_evidence.md for raw bytes and ownership chain.
PARAMETER = dict(index=0, name='ContinueText', kind='Text', kind_raw='0x1d',
                 property_flags='0x10000008000182', replicated=False, rep_notify=None,
                 array_dim=1, object_flags='0x45', offset=0)
OTHER_PARAMETER = dict(PARAMETER, name='InText')
FUNCTION = dict(name='SetContinueButtonText', create_func='0x1452825a0', flags=['0x8480800'],
    candidates=[dict(owning_class=None, outer_fn='0x14522c030', flags='0x8480800', nprop=1,
                     params=[OTHER_PARAMETER]),
                dict(owning_class=None, outer_fn='0x145282560', flags='0x8480800', nprop=1,
                     params=[PARAMETER])])
OWNER = {'0x145282560'}


class ParameterizedBlueprintEvents(unittest.TestCase):
    def build(self, function, owners=OWNER):
        resolver = gen.Resolver({})
        actual = gen.build_functions_promoted([function], resolver, set(), owners,
            parameterized_blueprint_events=gen.UI_PARAMETERIZED_BLUEPRINT_EVENTS['UFortLoginStatus'])
        return actual, resolver.skipped

    def test_evidenced_signature_and_rendering(self):
        functions, skipped = self.build(FUNCTION)
        self.assertEqual(skipped, [])
        self.assertEqual(len(functions), 1)
        self.assertEqual(functions[0]['args'], ['const FText& ContinueText'])
        self.assertEqual(functions[0]['specifiers'], ['BlueprintImplementableEvent'])
        model = dict(name='UFortLoginStatus', members_table_order=[], members_decl_order=[],
                     forward_decls=[], uclass='UCLASS()', parent='UFortActivatablePanel', functions=functions)
        header = gen.render_promoted_header(model, 'FORTNITEUI_API', 'UFortActivatablePanel.h', 'reflection')
        cpp = gen.render_promoted_cpp(model, 'reflection')
        self.assertIn('protected:\n\tUFUNCTION(BlueprintImplementableEvent)\n\tvoid SetContinueButtonText(const FText& ContinueText);', header)
        self.assertNotIn('BlueprintCallable', header)
        self.assertNotIn('UFortLoginStatus::SetContinueButtonText', cpp)
        self.assertNotIn('SetContinueButtonText_Implementation', header)

    def test_wrong_owner_or_constructor_fails_closed(self):
        self.assertFalse(self.build(FUNCTION, {'0x14522c030'})[0])
        modified = copy.deepcopy(FUNCTION)
        modified['create_func'] = '0x14522c120'
        self.assertFalse(self.build(modified)[0])

    def test_wrong_parameter_signature_fails_closed(self):
        for key, value in [('name', 'InText'), ('kind', 'Str'), ('kind_raw', '0x1c'),
                           ('property_flags', '0x10000000000080'), ('array_dim', 2),
                           ('offset', 4), ('object_flags', '0x41')]:
            with self.subTest(key=key):
                modified = copy.deepcopy(FUNCTION)
                modified['candidates'][1]['params'][0][key] = value
                self.assertFalse(self.build(modified)[0])

    def test_wrong_function_flags_or_missing_owner_join_fails_closed(self):
        modified = copy.deepcopy(FUNCTION)
        modified['candidates'][1]['flags'] = '0xc480800'  # adds unobserved BlueprintCallable
        self.assertFalse(self.build(modified)[0])
        modified = copy.deepcopy(FUNCTION)
        del modified['candidates']
        modified['params'] = [copy.deepcopy(PARAMETER)]
        self.assertFalse(self.build(modified)[0])

    def test_existing_no_parameter_event_unchanged(self):
        resolver = gen.Resolver({})
        fn = dict(name='BP_OnHovered', flags=['0x8080800'], params=[])
        result = gen.build_functions_promoted([fn], resolver, set(), set(),
            blueprint_events=gen.UI_BLUEPRINT_EVENTS['UCommonButton'])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0]['args'], [])
        self.assertEqual(result[0]['specifiers'], ['BlueprintImplementableEvent'])

    def test_local_reflection_when_available(self):
        path = PORTING / 'generated/reflection/fortnite_game_reflection.json'
        if not path.is_file():
            self.skipTest('optional local reflection dump unavailable')
        data = json.loads(path.read_text())
        block = next(b for b in data['classes_in_groups_with_fort_names'] if b.get('block_va') == '0x146f6d830')
        function = next(f for f in block['functions'] if f.get('name') == 'SetContinueButtonText')
        result, skipped = self.build(function)
        self.assertFalse(skipped)
        self.assertEqual(result[0]['args'], ['const FText& ContinueText'])


if __name__ == '__main__':
    unittest.main()
