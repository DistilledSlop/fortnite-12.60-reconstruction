import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location("stage_character_content", Path(__file__).with_name("stage_character_content.py"))
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
select_roots = module.select_roots


class CharacterContentSelectionTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.manifest = Path(self.tmp.name) / 'manifest.json'
        self.census = Path(self.tmp.name) / 'classes.tsv'
        self.character = '/Game/Athena/Items/Cosmetics/Characters/CID_Test'
        self.set_manifest([self.character + '.CID_Test'])
        self.set_census('FortniteGame/Content/Athena/HS_Test\tFortHeroSpecialization\n'
                        'FortniteGame/Content/Characters/CP_Test\tCustomCharacterPart,Other\n'
                        'FortniteGame/Content/Other/Ignore\tTexture2D\n'
                        'FortniteGame/Plugins/P/Content/CP_Plugin\tCustomCharacterPart\n')

    def set_manifest(self, paths):
        data = json.dumps({'rows': [{'objectPath': p} for p in paths]}).encode()
        self.manifest.write_bytes(data)
        self.manifest_hash = hashlib.sha1(data).hexdigest()

    def set_census(self, text):
        data = text.encode()
        self.census.write_bytes(data)
        self.census_hash = hashlib.sha256(data).hexdigest()

    def select(self, count=1):
        return select_roots(self.manifest, self.census, self.manifest_hash, self.census_hash,
                            count, {'FortHeroSpecialization': 1, 'CustomCharacterPart': 1})

    def test_selects_semantic_roots_and_excludes_unrelated_and_plugin_mounts(self):
        self.assertEqual(self.select(), ['/Game/Athena/HS_Test', self.character, '/Game/Characters/CP_Test'])

    def test_refuses_changed_inputs(self):
        self.census.write_text('changed')
        with self.assertRaisesRegex(ValueError, 'digest mismatch'):
            self.select()

    def test_rejects_missing_class_even_with_valid_digest(self):
        self.set_census('FortniteGame/Content/Athena/HS_Test\tFortHeroSpecialization\n')
        with self.assertRaisesRegex(ValueError, 'class census counts'):
            self.select()

    def test_rejects_duplicate_and_unsafe_character_paths(self):
        self.set_manifest([self.character + '.CID_Test'] * 2)
        with self.assertRaisesRegex(ValueError, 'duplicate'):
            self.select(2)
        self.set_manifest(['/Game/Athena/Items/Cosmetics/Characters/../Escape.Escape'])
        with self.assertRaisesRegex(ValueError, 'unsafe'):
            self.select()


if __name__ == '__main__':
    unittest.main()
