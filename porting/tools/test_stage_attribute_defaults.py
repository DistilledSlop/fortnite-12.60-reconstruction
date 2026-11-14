import subprocess
import tempfile
import unittest
from pathlib import Path
from stage_attribute_defaults import stage

TABLES = [f'/Game/Test/Table{i}.Table{i}'.encode() for i in range(11)]
SOURCE = (b'[/Script/GameplayAbilities.AbilitySystemGlobals]\n'
          + b''.join(b'+GlobalAttributeSetDefaultsTableNames=' + p + b'\n' for p in TABLES)
          + b'[/Script/FortniteGame.FortAbilitySystemGlobals]\n'
          + b''.join(b'+BROnlyAttributeSetDefaultsToExclude=' + p + b'\n' for p in TABLES[:3])
          + b'[/Script/FortniteGame.FortGlobals]\nbBROnly=True\nUnrelated=private\n')
TARGET = (b'[Other]\nValue=keep\n[/Script/GameplayAbilities.AbilitySystemGlobals]\n'
          b'AbilitySystemGlobalsClassName=/Script/FortniteGame.FortServerAbilitySystemGlobals\n'
          b'+GlobalAttributeSetDefaultsTableNames=/Game/Old.Old\n')

class TestStage(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.source = self.root / 'original.ini'
        self.target = self.root / 'DefaultGame.ini'
        self.out = self.root / 'evidence'
        self.source.write_bytes(SOURCE)
        self.target.write_bytes(TARGET)
        self.commands = []

    def run_stage(self, runner=None):
        return stage(self.source, self.target, self.out, '/content', '/stager',
                     runner or (lambda cmd, check: self.commands.append(cmd)))

    def test_stage_and_idempotence(self):
        receipt = self.run_stage()
        data = self.target.read_bytes()
        self.assertIn(b'FortServerAbilitySystemGlobals', data)
        self.assertIn(b'Value=keep', data)
        self.assertNotIn(b'/Game/Old.Old', data)
        self.assertNotIn(b'Unrelated=private', data)
        self.assertEqual(data.count(b'+GlobalAttributeSetDefaultsTableNames='), 11)
        self.assertIn(b'[/script/fortnitegame.fortglobals]\nbBROnly=True', data)
        self.assertNotIn(b'[/script/fortnitegame.fortabilitysystemglobals]\nbBROnly=True', data)
        self.assertEqual(receipt['exclusion_count'], 3)
        self.assertIn('--strict', self.commands[0])
        self.assertEqual(self.commands[0][-11:], [p.split(b'.')[0].decode() for p in TABLES])
        self.out = self.root / 'evidence2'
        self.run_stage()
        self.assertEqual(self.target.read_bytes(), data)

    def test_fail_closed(self):
        for bad in (SOURCE.replace(TABLES[-1], b'bad'), SOURCE.replace(b'+BROnlyAttributeSetDefaultsToExclude=', b'-BROnlyAttributeSetDefaultsToExclude=', 1)):
            self.source.write_bytes(bad)
            with self.assertRaises(ValueError): self.run_stage()
            self.assertEqual(self.target.read_bytes(), TARGET)
            self.assertFalse(self.out.exists())
        self.source.write_bytes(SOURCE)
        def fail(cmd, check): raise subprocess.CalledProcessError(1, cmd)
        with self.assertRaises(subprocess.CalledProcessError): self.run_stage(fail)
        self.assertEqual(self.target.read_bytes(), TARGET)
        self.assertFalse(self.out.exists())

    def test_existing_content_mismatch_fails_before_config_replacement(self):
        destination = self.root / 'content'
        stale = destination / 'Test/Table0.uasset'
        stale.parent.mkdir(parents=True)
        stale.write_bytes(b'stale')
        def populate(cmd, check):
            closure = Path(cmd[cmd.index('--dst') + 1])
            package = closure / 'Test/Table0.uasset'
            package.parent.mkdir(parents=True)
            package.write_bytes(b'correct')
        with self.assertRaisesRegex(ValueError, 'differs from selected source'):
            stage(self.source, self.target, self.out, '/content', '/stager', populate, destination)
        self.assertEqual(stale.read_bytes(), b'stale')
        self.assertEqual(self.target.read_bytes(), TARGET)
        self.assertFalse(self.out.exists())

    def test_optional_br_only_absent_and_duplicate_section(self):
        self.source.write_bytes(SOURCE.replace(b'bBROnly=True\n', b''))
        self.assertFalse(self.run_stage()['br_only_present'])
        self.out = self.root / 'other'
        self.target.write_bytes(TARGET + b'[/Script/GameplayAbilities.AbilitySystemGlobals]\n')
        with self.assertRaises(ValueError): self.run_stage()

if __name__ == '__main__': unittest.main()
