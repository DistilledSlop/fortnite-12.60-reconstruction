import argparse
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

import runner
import verifier

ROOT = verifier.ROOT
ROWS = [
    'ROOTINSTANCE v=1 phase=registered timeout_seconds=180',
    'ROOTINSTANCE v=1 phase=prerequisites world=W game_instance=G player=P controller=C',
    f'ROOTINSTANCE v=1 phase=before_load path={ROOT}',
    f'ROOTINSTANCE v=1 phase=after_load class={ROOT} ok=1',
    'ROOTINSTANCE v=1 phase=before_create_widget',
    f'ROOTINSTANCE v=1 phase=after_create_widget widget=I class={ROOT} outer=G world=W player=P controller=C tree=T',
    'ROOTINSTANCE v=1 phase=observations ok=1 scope=create_return_and_ownership',
    'ROOTINSTANCE v=1 phase=complete ok=1',
]

V2_ROWS = [row.replace('v=1', 'v=2') for row in ROWS]
V2_ROWS[1] += (' game_instance_class=/Script/FortniteGame.FortGameInstance'
               ' player_class=/Script/FortniteGame.FortLocalPlayer'
               ' controller_class=/Script/Engine.PlayerController'
               ' viewport_class=/Script/FortniteGame.FortGameViewportClient')
V2_ROWS[5] += ' root_widget=Root widget_count=3'

class TranscriptTests(unittest.TestCase):
    def test_original_widget_name_with_spaces(self):
        rows = V2_ROWS.copy()
        rows[5] = rows[5].replace('root_widget=Root', 'root_widget=/Engine/Transient.Root.WidgetTree_0.Master Overlay')
        result = verifier.verify('\n'.join(rows))
        self.assertTrue(result['passed'])
        self.assertEqual(result['root_widget'], '/Engine/Transient.Root.WidgetTree_0.Master Overlay')

    def test_v2_observations_and_non_fort_controller(self):
        result = verifier.verify('\n'.join(V2_ROWS))
        self.assertTrue(result['passed'])
        self.assertTrue(result['creation_completed'])
        self.assertTrue(result['startup_classes_match'])
        self.assertEqual(result['startup_class_observations']['controller_class'], '/Script/Engine.PlayerController')
        self.assertTrue(result['tree_populated'])
        self.assertEqual(result['widget_count'], 3)
        historical = verifier.verify('\n'.join(ROWS))
        self.assertTrue(historical['passed'])
        self.assertIsNone(historical['startup_classes_match'])
        self.assertIsNone(historical['tree_populated'])
        self.assertIsNone(historical['widget_count'])

    def test_v2_failures_preserve_observations(self):
        cases = [
            (1, 'FortGameInstance', 'WrongGameInstance', 'startup_classes_match'),
            (1, '/Script/Engine.PlayerController', 'None', 'startup_classes_match'),
            (5, 'widget_count=3', 'widget_count=0', 'tree_populated'),
            (5, 'root_widget=Root', 'root_widget=None', 'tree_populated'),
        ]
        for index, old, new, key in cases:
            with self.subTest(new=new):
                rows = V2_ROWS.copy()
                rows[index] = rows[index].replace(old, new)
                result = verifier.verify('\n'.join(rows))
                self.assertFalse(result['passed'])
                self.assertTrue(result['creation_completed'])
                self.assertFalse(result[key])
        for error in ('Blueprint Runtime Error: Accessed None',
                      'Skipping import Function X, depends on missing native class',
                      'Missing RowStruct while loading DataTable X'):
            result = verifier.verify('\n'.join(V2_ROWS + [error]))
            self.assertFalse(result['passed'])
            self.assertTrue(result['creation_completed'])
            self.assertTrue(result['startup_classes_match'])
            self.assertTrue(result['tree_populated'])

    def test_v2_strict_rows_and_counts(self):
        cases = [V2_ROWS[:-1], V2_ROWS + [V2_ROWS[-1]],
                 [*V2_ROWS[:3], V2_ROWS[3].replace('v=2', 'v=1'), *V2_ROWS[4:]],
                 [*V2_ROWS[:5], V2_ROWS[5].replace('widget_count=3', 'widget_count=-1'), *V2_ROWS[6:]],
                 [*V2_ROWS[:5], V2_ROWS[5] + ' widget_count=3', *V2_ROWS[6:]],
                 [*V2_ROWS[:1], V2_ROWS[1].replace(' player_class=/Script/FortniteGame.FortLocalPlayer', ''), *V2_ROWS[2:]]]
        for rows in cases:
            with self.subTest(rows=rows):
                self.assertFalse(verifier.verify('\n'.join(rows))['passed'])

    def test_valid_and_rejections(self):
        self.assertTrue(verifier.verify('\n'.join(ROWS))['passed'])
        cases = [ROWS[:-1], ROWS + [ROWS[-1]], ROWS[1:] + ROWS[:1],
                 [*ROWS[:6], ROWS[6].replace('create_return_and_ownership', 'full_ui'), ROWS[7]],
                 [*ROWS[:2], ROWS[2].replace(ROOT, '/wrong'), *ROWS[3:]],
                 [*ROWS[:5], ROWS[5].replace('outer=G', 'outer=X'), *ROWS[6:]],
                 [*ROWS, 'Blueprint Runtime Error: Accessed None']]
        for rows in cases:
            with self.subTest(rows=rows):
                self.assertFalse(verifier.verify('\n'.join(rows))['passed'])
        self.assertEqual(verifier.verify('\n'.join(ROWS[:5]))['last_phase'], 'before_create_widget')

    def test_bytecode_error(self):
        result = verifier.verify('\n'.join(ROWS + ['Unable to resolve bytecode referenced field Foo']))
        self.assertFalse(result['passed'])
        self.assertEqual(len(result['blueprint_errors']), 1)

    def test_repetitive_error_excerpt_is_bounded(self):
        result = verifier.verify('\n'.join(ROWS + ['Accessed None'] * 100))
        self.assertFalse(result['passed'])
        self.assertEqual(result['blueprint_error_count'], 100)
        self.assertEqual(len(result['blueprint_errors']), 20)

    def test_creation_does_not_mask_missing_native_dependencies(self):
        for error in ('Skipping import Function /Script/FortniteUI.X:Y, depends on missing native class',
                      'Skipping import Enum /Script/CommonUI.InputActionState, depends on missing native class',
                      'Skipping import DelegateFunction /Script/FortniteUI.Result__DelegateSignature, depends on missing native class',
                      "Missing RowStruct while loading DataTable '/Game/UI/Input.Input'!"):
            with self.subTest(error=error):
                result = verifier.verify('\n'.join(ROWS + [error]))
                self.assertTrue(result['creation_completed'])
                self.assertFalse(result['passed'])
                self.assertEqual(result['dependency_error_count'], 1)

class RunnerTests(unittest.TestCase):
    def test_flags_and_timeout_receipt(self):
        with tempfile.TemporaryDirectory() as tmp:
            d = Path(tmp)
            flags = ['-FortniteLegacyMaterialIngress=a', '-FortniteLegacyMaterialManifest=b',
                     '-FortniteLegacyMaterialManifestSHA256=c']
            f = d / 'args.json'
            f.write_text(json.dumps(flags))
            self.assertEqual(runner.runtime_args(f)[0], flags)
            for bad in ['-ExecCmds=Quit', '-FortniteProbeAsset=x', '-stdout', '-RootWidgetInstanceProbe']:
                f.write_text(json.dumps(flags + [bad]))
                with self.assertRaises(ValueError):
                    runner.runtime_args(f)
            f.write_text(json.dumps(flags))
            source = d / 'child.c'
            source.write_text('#include <stdio.h>\n#include <unistd.h>\nint main(void){puts("ROOTINSTANCE v=1 phase=registered timeout_seconds=180"); fflush(stdout); sleep(10); return 0;}\n')
            binary = d / 'child'
            subprocess.run(['cc', str(source), '-o', str(binary)], check=True)
            build = d / 'build.json'
            build.write_text(json.dumps([{'target': 'FortniteLinux', 'exit': 0, 'source_tree_unchanged': True,
                                          'source_head': 'test', 'binary': {'path': str(binary),
                                          'size': binary.stat().st_size, 'sha256': runner.digest(binary)}}]))
            out = d / 'out'
            config = d / 'DefaultEngine.ini'
            config.write_text('; captured staged config\n')
            args = argparse.Namespace(binary=binary, cwd=d, timeout=0.3, build_results=build,
                                      out=out, runtime_args=f, config=config)
            self.assertFalse(runner.run(args))
            receipt = json.loads((out / 'receipt.json').read_text())
            self.assertTrue(receipt['timed_out'])
            self.assertEqual(receipt['transcript']['last_phase'], 'registered')
            self.assertTrue(receipt['proc_exe_inode_verified'])
            self.assertEqual(receipt['config']['sha256'], runner.digest(config))
            self.assertTrue(receipt['config_after_verified'])
            self.assertTrue((out / 'runtime.log').exists())
            self.assertNotIn('-ExecCmds=Quit', receipt['argv'])

if __name__ == '__main__':
    unittest.main()
