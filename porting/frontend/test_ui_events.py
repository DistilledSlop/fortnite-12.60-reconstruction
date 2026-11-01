#!/usr/bin/env python3
"""Check the observed CommonButton Blueprint imports without promotion or a UE build."""
import importlib.util
import json
from pathlib import Path
import sys
import unittest

PORTING = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('generator', PORTING / 'gen_native_classes.py')
gen = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gen)

class BlueprintEvents(unittest.TestCase):
    def test_observed_reflection_and_rendering(self):
        path = Path(sys.argv[1]) if len(sys.argv) > 1 else PORTING / 'generated/reflection/fortnite_game_reflection.json'
        data = json.loads(path.read_text())
        blocks = [b for b in data['classes_in_groups_with_fort_names']
                  if any(f.get('name') == 'BP_OnHovered' and f.get('outer_fn') == '0x140805cb0'
                         for f in b.get('functions', []))]
        self.assertEqual(len(blocks), 1)
        names = gen.UI_BLUEPRINT_EVENTS['UCommonButton']
        observed = [f for f in blocks[0]['functions'] if f.get('name') in names]
        self.assertEqual({f['name'] for f in observed}, names)
        resolver = gen.Resolver({})
        functions = gen.build_functions_promoted(observed, resolver, set(), {'0x140805cb0'}, blueprint_events=names)
        self.assertEqual({f['name'] for f in functions}, names)
        self.assertEqual(resolver.skipped, [])
        model = {'name':'UCommonButton', 'members_table_order':[], 'members_decl_order':[],
                 'forward_decls':[], 'uclass':'UCLASS()', 'parent':'UCommonUserWidget',
                 'functions':functions}
        header = gen.render_promoted_header(model, 'COMMONUI_API', 'UCommonUserWidget.h', 'reflection')
        cpp = gen.render_promoted_cpp(model, 'reflection')
        for name in names:
            self.assertIn('protected:\n\tUFUNCTION(BlueprintImplementableEvent)\n\tvoid ' + name + '();', header)
            self.assertNotIn('UCommonButton::' + name, cpp)
            self.assertNotIn(name + '_Implementation', header)

    def test_refuses_changed_flags_or_signature(self):
        names = gen.UI_BLUEPRINT_EVENTS['UCommonButton']
        for flags, params in [('0x8080c00', []), ('0x8080800', [{'name':'X', 'kind':'Int'}])]:
            resolver = gen.Resolver({})
            fn = {'name':'BP_OnHovered', 'flags':[flags], 'params':params}
            self.assertEqual(gen.build_functions_promoted([fn], resolver, set(), set(), blueprint_events=names), [])
            self.assertEqual(resolver.skipped[0]['category'], 'blueprint_event_mismatch')

if __name__ == '__main__':
    reflection = sys.argv[1:]
    sys.argv = sys.argv[:1]
    if reflection:
        sys.argv.append(reflection[0])
    unittest.main(argv=sys.argv[:1])
