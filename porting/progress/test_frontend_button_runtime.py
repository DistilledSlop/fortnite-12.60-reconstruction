#!/usr/bin/env python3
"""Guard the bounded button runtime evidence importer."""
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('button_recorder', Path(__file__).with_name('record_frontend_button_runtime.py'))
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


def fixture():
    c = json.loads(Path(m.CHECKPOINT).read_text())
    m.PINS[m.ROOT] = c['root_report']['sha256']
    root = m.read_ref(c['root_report'])
    builds, sources, stage, joins = (m.read_ref(c[k]) for k in ('build_results', 'source_snapshot', 'stage', 'joins'))
    processes = {n: m.read_ref(row['receipt']) for n, row in c['processes'].items()}
    transcripts = {n: m.read_ref(row['transcript']) for n, row in c['processes'].items()}
    return [c, root, builds, sources, stage, processes, transcripts, joins]


class ButtonImportTests(unittest.TestCase):
    def test_evidence_and_negative_guards(self):
        args = fixture()
        m.validate(*args)
        mutations = [
            lambda a: a[0].update(phase_passed=False),
            lambda a: a[1].update(passed=False),
            lambda a: a[2][0].update(source_tree_unchanged=False),
            lambda a: a[3].update(commit='wrong'),
            lambda a: a[5]['button'].update(proc_exe_inode_verified=False),
            lambda a: a[5]['button'].update(pid=a[5]['activation']['pid']),
            lambda a: a[6]['button']['detail'].update(rows=33),
            lambda a: a[6]['activation']['detail'].update(pass_=False),
            lambda a: a[6]['registry']['detail']['loaded_reflection'].update(passed=False),
            lambda a: a[7][0].update(exit=1),
        ]
        # Use the actual JSON key for the activation pass flag.
        mutations[7] = lambda a: a[6]['activation']['detail'].__setitem__('pass', False)
        for mutate in mutations:
            bad = copy.deepcopy(args)
            mutate(bad)
            with self.subTest(mutation=mutate), self.assertRaises((ValueError, KeyError)):
                m.validate(*bad)

    def test_missing_pin_and_tampering_fail(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp) / 'sample.json'; p.write_text('{}')
            digest = hashlib.sha256(p.read_bytes()).hexdigest()
            self.assertEqual(m.read_pinned(str(p), digest), {})
            for pin in ('', '0' * 64):
                with self.assertRaises(ValueError): m.read_pinned(str(p), pin)
            with self.assertRaises(ValueError): m.read_ref({'path': str(p), 'size': 3, 'sha256': digest})

    def test_audit_and_release_guards(self):
        audit = dict(schema='button-internal-integrity-audit-v1', passed=True,
            checkpoint_pin={'sha256': m.PINS[m.CHECKPOINT]}, root_report_pin={'sha256': m.PINS[m.ROOT]},
            release_pin={'sha256': m.PINS[m.RELEASE]}, artifact_count=40, binary_rows=18,
            critical_source_count=9, native_source_count=12553, process_count=4,
            button_rows=34, activation_rows=40, joins_count=2, build_receipts=2, milestones_completed=0, mismatches=[])
        release = dict(schema='button-internal-source-release-v1', released=True,
            native_source_build_stage_released=True, all_phase_processes_terminal=True,
            checkpoint={'sha256': m.PINS[m.CHECKPOINT]})
        m.validate_audit_release(audit, release)
        for key, value in [('passed', False), ('button_rows', 33), ('mismatches', ['bad'])]:
            bad = copy.deepcopy(audit); bad[key] = value
            with self.assertRaises(ValueError): m.validate_audit_release(bad, release)
        release['all_phase_processes_terminal'] = False
        with self.assertRaises(ValueError): m.validate_audit_release(audit, release)

    def test_unfinished_audit_fails_closed(self):
        with self.assertRaises((ValueError, KeyError)): m.validate_audit_release({}, {})


if __name__ == '__main__': unittest.main()
