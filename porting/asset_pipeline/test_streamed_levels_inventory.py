#!/usr/bin/env python3
"""Pure-Python tests for streamed_levels_inventory (no inspector, no extraction)."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import streamed_levels_inventory as inv


class InventoryTests(unittest.TestCase):
    def test_parse_tool_json_skips_banner(self):
        self.assertEqual(inv.parse_tool_json('Indexed 5 loose package(s).\n{"a": 1}'), {'a': 1})
        with self.assertRaises(ValueError):
            inv.parse_tool_json('no json here')

    def test_soft_level_refs(self):
        blob = (b'\0/Game/M/Parent\0/Game/M/Parent.Parent\0/Game/M/L/A1\0/Game/M/L/A1.A1\0'
                b'/Game/M/Other\0/Game/M/Aux\0/Game/M/Aux.Aux\0')
        self.assertEqual(inv.soft_level_refs(blob, '/Game/M/Parent'), ['/Game/M/Aux', '/Game/M/L/A1'])

    def test_path_mapping(self):
        p = inv.game_path_to_file(Path('/x'), '/Game/A/B', '.umap')
        self.assertEqual(p, Path('/x/FortniteGame/Content/A/B.umap'))

    def test_actor_summary(self):
        s = inv.summarize_actor_report({'exportTypes': [
            {'type': 'LandscapeStreamingProxy', 'count': 1}, {'type': 'BoxComponent', 'count': 4},
            {'type': 'Texture2D', 'count': 2}, {'type': 'Level', 'count': 1}]})
        self.assertEqual((s['export_count'], s['actor_count']), (8, 1))
        self.assertEqual(list(s['class_histogram'])[0], 'BoxComponent')

    def test_landscape_summary(self):
        s = inv.summarize_landscape_report({'actors': [{'components': [{'n': 1}, None, {'n': 2}]}, {'components': []}]})
        self.assertEqual(s, {'landscape_actors': 2, 'landscape_components': 2})

    def test_imports_summary(self):
        s = inv.summarize_imports({'imports': [
            {'name': 'Level', 'kind': 'Class', 'chain': [{'name': '/Script/Engine'}, {'name': 'Level'}]},
            {'name': 'Vec', 'kind': 'ScriptStruct', 'chain': [{'name': '/Script/CoreUObject'}]},
            {'name': '/Game/X/Y', 'kind': 'Package', 'chain': [{'name': '/Game/X/Y'}]},
            {'name': 'Foo', 'kind': 'Class', 'chain': [{'name': '/Game/X/Y'}]}]})
        self.assertEqual(s['script_classes'], ['/Script/Engine.Level'])
        self.assertEqual(s['script_structs'], ['/Script/CoreUObject.Vec'])
        self.assertEqual(s['game_packages'], ['/Game/X/Y'])

    def test_aggregate_gap_ranking(self):
        mk = lambda n, *c: {'package': n, 'imports': {'script_classes': list(c)}}
        levels = [mk('a', '/Script/L.Proxy', '/Script/E.Level'), mk('b', '/Script/L.Proxy'), mk('c', '/Script/N.Thing')]
        rows = inv.aggregate_gap(levels, native={'UThing'}, engine={'ULevel'})
        self.assertEqual([r['class'] for r in rows], ['/Script/L.Proxy', '/Script/E.Level', '/Script/N.Thing'])
        self.assertEqual([r['gap'] for r in rows], [True, False, False])
        self.assertEqual(rows[0]['levels'], 2)

    def test_scan_declarations(self):
        import tempfile
        with tempfile.TemporaryDirectory() as d:
            (Path(d) / 'a.h').write_text('class FOO_API AFoo : public AActor {};\nclass UFwd;\nstruct FBar {};\nclass NotPrefixed {};\n')
            self.assertEqual(inv.scan_declarations([Path(d)]), {'AFoo', 'FBar'})


if __name__ == '__main__':
    unittest.main()
