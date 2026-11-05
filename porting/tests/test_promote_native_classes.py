import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

PORTING = Path(__file__).resolve().parents[1]


class PromotionTest(unittest.TestCase):
    def test_staging_and_merge(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for name in ('promote_native_classes.sh', 'merge_promoted_classes.py'):
                shutil.copy(PORTING / name, root / name)
            (root / 'resolve_class_refs.py').write_text('''
import pathlib, sys
pathlib.Path(sys.argv[sys.argv.index('--out') + 1]).write_text('refs')
''')
            (root / 'gen_native_classes.py').write_text('''
import os, pathlib, sys
source = pathlib.Path(sys.argv[sys.argv.index('--source-root') + 1])
reports = pathlib.Path(sys.argv[sys.argv.index('--report-dir') + 1])
def put(rel, value):
    path = source / rel
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(value)
put('M/Generated/same.h', 'same')
put('M/Generated/changed.h', 'new')
put('M/M.Build.cs', 'new build')
put('M/Private/M.cpp', 'new module')
(reports / 'native_classes_index.json').write_text('index')
(reports / 'behavior_splice.json').write_text('{"rejected": %s}' % ('1' if os.environ.get('REJECT_GENERATION') else '0'))
if os.environ.get('FAIL_GENERATION'):
    sys.exit(1)
''')
            source = root / 'native/Source'
            reports = root / 'generated/reflection'
            def put(path, value):
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(value)
            same = source / 'M/Generated/same.h'
            changed = source / 'M/Generated/changed.h'
            stale = source / 'Old/Generated/nested/stale.h'
            handwritten = source / 'M/Private/handwritten.cpp'
            for path, value in ((same, 'same'), (changed, 'old'), (stale, 'stale'),
                                (handwritten, 'handwritten'), (reports / 'class_ref_map.json', 'old refs')):
                put(path, value)
            os.utime(same, ns=(1_000_000_000, 1_000_000_000))
            env = dict(os.environ, FAIL_GENERATION='1')
            command = ['bash', str(root / 'promote_native_classes.sh')]
            self.assertNotEqual(subprocess.run(command, env=env, capture_output=True).returncode, 0)
            self.assertEqual(changed.read_text(), 'old')
            self.assertEqual((reports / 'class_ref_map.json').read_text(), 'old refs')
            self.assertTrue(stale.exists())
            rejected = dict(os.environ, REJECT_GENERATION='1')
            self.assertNotEqual(subprocess.run(command, env=rejected, capture_output=True).returncode, 0)
            self.assertEqual(changed.read_text(), 'old')
            self.assertEqual((reports / 'class_ref_map.json').read_text(), 'old refs')
            self.assertTrue(stale.exists())
            subprocess.run(command, check=True, capture_output=True)
            self.assertEqual(same.stat().st_mtime_ns, 1_000_000_000)
            self.assertEqual(changed.read_text(), 'new')
            self.assertFalse(stale.exists())
            self.assertEqual(handwritten.read_text(), 'handwritten')
            self.assertEqual((source / 'M/M.Build.cs').read_text(), 'new build')
            self.assertEqual((source / 'M/Private/M.cpp').read_text(), 'new module')
            self.assertEqual((reports / 'class_ref_map.json').read_text(), 'refs')
            self.assertEqual((reports / 'native_classes_index.json').read_text(), 'index')


if __name__ == '__main__':
    unittest.main()
