#!/usr/bin/env python3
"""Do not propagate FortniteGame's private or client-only modules to other modules."""
import importlib.util
from pathlib import Path
import tempfile
import unittest

PORTING = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('generator', PORTING / 'gen_native_classes.py')
gen = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gen)


class PublicDependencyPropagation(unittest.TestCase):
    def parse(self, source):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'FortniteGame.Build.cs'
            path.write_text(source, encoding='utf-8')
            return gen.build_cs_public_modules(path), gen.build_cs_modules(path)

    def test_public_only_and_comments(self):
        public, all_listed = self.parse('''
            // PublicDependencyModuleNames.Add("CommentOnly"); "CommentRenderer"
            PublicDependencyModuleNames.AddRange(new[] { "Core", "Engine", /* "Ignored" */ "CoreUObject" });
            PublicDependencyModuleNames.Add("SlateCore");
            if (Target.Type != TargetType.Server) {
                PrivateDependencyModuleNames.Add("Renderer");
            }
            string Text = "PublicDependencyModuleNames.Add(\\\"Fake\\\")";
        ''')
        self.assertEqual(public, {'Core', 'Engine', 'CoreUObject', 'SlateCore'})
        self.assertIn('Renderer', all_listed)  # own dependency validation remains broad
        generated = gen.render_module_build_cs('FortniteUI', sorted(public), 'fixture')
        self.assertIn('"Engine"', generated)
        self.assertNotIn('"Renderer"', generated)
        self.assertNotIn('"Ignored"', generated)

    def test_literal_string_array_and_trailing_comma(self):
        public, _ = self.parse('PublicDependencyModuleNames.AddRange(new string[] { "Core", "Engine", });')
        self.assertEqual(public, {'Core', 'Engine'})

    def test_unsupported_public_expression_fails_closed(self):
        for source in ('PublicDependencyModuleNames.AddRange(GetModules());',
                       'PublicDependencyModuleNames.Add(SomeVariable);',
                       'PublicDependencyModuleNames.AddRange(new[] { "Core", ComputeModule() });',
                       'PublicDependencyModuleNames.AddRange(OtherModules);'):
            with self.subTest(source=source), self.assertRaisesRegex(ValueError, 'unsupported PublicDependencyModuleNames'):
                self.parse(source)

    def test_missing_public_declaration_fails_closed(self):
        with self.assertRaisesRegex(ValueError, 'no literal PublicDependencyModuleNames'):
            self.parse('PrivateDependencyModuleNames.Add("Renderer");')


if __name__ == '__main__':
    unittest.main()
