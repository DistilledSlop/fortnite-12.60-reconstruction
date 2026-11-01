#!/usr/bin/env python3
"""Guard the bounded Panel outro evidence importer."""
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('panel_recorder', Path(__file__).with_name('record_frontend_panel_outro_runtime.py'))
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


def fixture():
    c = json.loads(Path(m.CHECKPOINT).read_text())
    root = m.read_ref(c['root_report'])
    builds, sources, stage, joins, observation = (m.read_ref(c[k]) for k in ('build_results', 'source_snapshot', 'stage', 'joins', 'root_class_load_observation'))
    processes = {n: m.read_ref(row['receipt']) for n, row in c['processes'].items()}
    transcripts = {n: m.read_ref(row['transcript']) for n, row in c['processes'].items()}
    return [c, root, builds, sources, stage, processes, transcripts, joins, observation]


class PanelOutroImportTests(unittest.TestCase):
    def test_evidence_and_negative_guards(self):
        args = fixture()
        m.validate(*args)
        mutations = [
            lambda a: a[0].update(phase_passed=False),
            lambda a: a[1].update(passed=False),
            lambda a: a[2][0].update(source_tree_unchanged=False),
            lambda a: a[1]['builds'][0].update(elapsed_seconds=0),
            lambda a: a[3]['source_tree'].pop(next(iter(a[3]['source_tree']))),
            lambda a: a[5].pop('lifecycle'),
            lambda a: a[5]['panel-intro'].update(proc_exe_inode_verified=False),
            lambda a: a[5]['panel-intro'].update(pid=a[5]['activation']['pid']),
            lambda a: a[5]['panel-intro']['build_results'].update(sha256='wrong'),
            lambda a: a[6]['panel-outro']['detail'].update(rows=59),
            lambda a: a[6]['panel-intro']['detail'].update(rows=48),
            lambda a: a[6]['main-button']['detail'].update(rows=43),
            lambda a: a[6]['activation']['detail'].__setitem__('pass', False),
            lambda a: a[6]['registry']['detail']['loaded_reflection'].update(passed=False),
            lambda a: a[7][0].update(exit=1),
            lambda a: a[8].update(exit=0),
            lambda a: a[8].update(previous_on_begin_intro_missing_dependency_resolved=False),
            lambda a: a[8].update(previous_on_begin_outro_missing_dependency_resolved=False),
            lambda a: a[8].update(next_observed_material_boundary='other'),
            lambda a: a[8].update(next_observed_assertion='other'),
            lambda a: a[8].update(timed_out=True),
            lambda a: a[8].update(binary_after_verified=False),
        ]
        for mutate in mutations:
            bad = copy.deepcopy(args)
            mutate(bad)
            with self.subTest(mutation=mutate), self.assertRaises((ValueError, KeyError)):
                m.validate(*bad)

    def test_missing_pin_and_tampering_fail(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp) / 'sample.json'
            p.write_text('{}')
            digest = hashlib.sha256(p.read_bytes()).hexdigest()
            self.assertEqual(m.read_pinned(str(p), digest), {})
            for pin in ('', '0' * 64):
                with self.assertRaises(ValueError): m.read_pinned(str(p), pin)
            with self.assertRaises(ValueError): m.read_ref({'path': str(p), 'size': 3, 'sha256': digest})
            p.write_text('[]')
            with self.assertRaises(ValueError): m.read_pinned(str(p), digest)

    def test_audit_and_release_guards(self):
        audit = dict(schema='panel-outro-integrity-audit-v1', passed=True,
            checkpoint_pin=m.PINS[m.CHECKPOINT], root_report_pin=m.PINS[m.ROOT],
            release_pin=m.PINS[m.RELEASE], artifact_count=88, binary_rows=18,
            critical_source_count=53, native_source_count=12568, process_count=7,
            panel_outro_rows=60, panel_intro_rows=49, main_button_rows=44,
            button_rows=34, activation_rows=40, joins_count=2, build_receipts=2,
            milestones_completed=0, root_material_boundary=True, root_stdout_verified=True, mismatches=[])
        release = dict(schema='panel-outro-source-release-v1', released=True,
            native_source_build_stage_released=True, all_phase_processes_terminal=True,
            checkpoint={'sha256': m.PINS[m.CHECKPOINT]})
        c = fixture()[0]
        manifest = m.read_ref(c['artifact_manifest'])
        m.validate_audit_release(audit, release, manifest)
        for key, value in [('passed', False), ('panel_outro_rows', 59), ('process_count', 6),
                           ('artifact_count', 87), ('critical_source_count', 52),
                           ('root_material_boundary', False), ('root_stdout_verified', False), ('mismatches', ['bad'])]:
            bad = copy.deepcopy(audit)
            bad[key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                m.validate_audit_release(bad, release, manifest)
        release['all_phase_processes_terminal'] = False
        with self.assertRaises(ValueError):
            m.validate_audit_release(audit, release, manifest)

    def test_unfinished_audit_fails_closed(self):
        with self.assertRaises((ValueError, KeyError)): m.validate_audit_release({}, {}, {})


if __name__ == '__main__': unittest.main()
