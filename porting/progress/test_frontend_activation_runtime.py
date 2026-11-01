#!/usr/bin/env python3
"""Negative guards for the bounded activation runtime importer."""
import copy
import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('activation_recorder', Path(__file__).with_name('record_frontend_activation_runtime.py'))
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)

def fixture():
    metadata = {t: {'passed': True, 'implementation_link': {'passed': True}, 'functions': {n: {} for n in ('ActivateWidget', 'DeactivateWidget', 'IsActivated')}} for t in ('FortniteLinux', 'FortniteLinuxServer')}
    processes = {n: dict(schema='context-run-v2', case=n, exit=0, timed_out=False, pid=i, proc_exe_inode_verified=True, binary_after_verified=True, start_ns=2, end_ns=3, binary_sha256='binary') for i,n in enumerate(('activation', 'registry', 'lifecycle'), 1)}
    rows = {n: dict(receipt={'sha256': n}, exit=0, timed_out=False, pid=p['pid'], inode_verified=True, binary_after_verified=True) for n,p in processes.items()}
    c = dict(schema='activation-runtime-checkpoint-v5', phase_passed=True, failure=None, native_source_commit='c06a5cb48327455b91a3a4ee9340a550d741bb29', engine_commit='d7e04d4b65fb8ee464082849414cf5f7ae4d7bca', tool_head='973d38e33deb07ce9864875f3893e599d3ec868e', new_artifact_count=639, runtime_manifest={'sha256':'080c82f43e26ec6ce90c4ad4cba22f008cb475b316d1886abf8e2fa4af4d7319'}, inherited_binary_rows=18, inherited_intermediate_rows=42, inherited_builds=[dict(target=t,exit=0,seconds=1) for t in metadata], build_origin='no new compile', strict_report={'sha256':m.PINS[m.OWNER]}, context_report={'sha256':m.PINS[m.CONTEXT]}, retry_receipt={'sha256':'retry'}, processes=rows)
    report = dict(schema='activation-live-verification-v2', passed=True, full_ready=True, runtime_provenance_verified=True, gaps=[], context_report_sha256=m.PINS[m.CONTEXT], retry_start_sha256='retry', run_sha256='activation', transcript={'pass':True,'rows':40}, metadata=metadata)
    ctx = dict(schema='context-live-verification-v1', passed=True, live_cases={n:{'run_sha256':n} for n in ('registry','lifecycle')})
    root = copy.deepcopy(report); root['context_report_sha256'] = m.PINS[m.ROOT_CONTEXT]
    release = dict(schema='activation-source-release-v1',phase='activation-runtime-5',released=True,native_source_build_stage_released=True,all_phase_processes_terminal=True,checkpoint_sha256=m.PINS[m.CHECKPOINT])
    return [c, report, ctx, root, copy.deepcopy(ctx), release, {}, {'schema':'activation-runtime-retry-v1','start_ns':1}, processes]

class ActivationImportTests(unittest.TestCase):
    def test_structural_guards(self):
        with patch.object(m, 'validate_audit'):
            m.validate(*fixture())
            mutations = [lambda x:x[0].update(phase_passed=False), lambda x:x[0]['inherited_builds'][0].update(exit=1), lambda x:x[1]['transcript'].update(rows=39), lambda x:x[3].update(passed=False), lambda x:x[4]['live_cases']['registry'].update(run_sha256='wrong'), lambda x:x[5].update(all_phase_processes_terminal=False), lambda x:x[8]['activation'].update(start_ns=1), lambda x:x[8]['registry'].update(pid=1), lambda x:x[1]['metadata']['FortniteLinux']['implementation_link'].update(passed=False)]
            for mutate in mutations:
                args=fixture(); mutate(args)
                with self.subTest(mutation=mutate):
                    with self.assertRaises((ValueError, KeyError)): m.validate(*args)

    def test_independent_audit_guards(self):
        audit = dict(audit='activation-runtime-5 integrity audit', passed=True,
            checkpoint=dict(sha256=m.PINS[m.CHECKPOINT], phase_passed=True, fresh_builds=False),
            archive=dict(entries=639, sha256='080c82f43e26ec6ce90c4ad4cba22f008cb475b316d1886abf8e2fa4af4d7319', mismatches=[]),
            inherited_phase2=dict(binary_row_count_claimed=18, binary_artifact_count=6, metadata_object_snapshot_count=42, metadata_object_snapshot_mismatches=[]),
            strict_report=dict(sha256=m.PINS[m.OWNER], passed=True, full_ready=True, activation_rows=40), context_report=dict(sha256=m.PINS[m.CONTEXT], passed=True),
            release=dict(sha256=m.PINS[m.RELEASE], ownership_released=True, all_phase_processes_terminal=True, checkpoint_hash_bound=True),
            retained_results_reference_count=15, retained_results_reference_mismatches=[], tool_input_blob_checks=[dict(tracked_blob_matches=True) for _ in range(5)])
        m.validate_audit(audit)
        for section, key, value in [('archive','mismatches',['bad']), ('inherited_phase2','metadata_object_snapshot_count',41), ('strict_report','passed',False), ('release','ownership_released',False)]:
            bad = copy.deepcopy(audit); bad[section][key] = value
            with self.subTest(section=section):
                with self.assertRaises(ValueError): m.validate_audit(bad)

    def test_binary_supplement_guards(self):
        args = fixture(); c, processes = args[0], args[8]
        supplement = dict(audit='activation-runtime-5 inherited phase2 binary integrity supplement', passed=True, supplement_to_sha256=m.PINS[m.AUDIT], binary_rows=18, distinct_physical_inodes_hashed=18, role_rows_verified={'snapshot':6,'source':6,'stage':6}, mismatch_paths=[], missing_or_stat_errors=[], role_identity_mismatches=[], identities=[dict(row_count=3, roles=['snapshot','source','stage'], paths=['a','b','c'], sha256='hash', size=1) for _ in range(6)], process_executable_hash_bound_to_phase2_executable=True, phase5_runtime_process_executable_bindings={n:dict(exit=0, receipt_sha256=n, executable_sha256='binary') for n in processes})
        m.validate_supplement(supplement, c, processes)
        for key, value in [('binary_rows',17), ('mismatch_paths',['bad']), ('process_executable_hash_bound_to_phase2_executable',False)]:
            bad=copy.deepcopy(supplement); bad[key]=value
            with self.subTest(key=key):
                with self.assertRaises(ValueError): m.validate_supplement(bad, c, processes)
        bad=copy.deepcopy(supplement); bad['phase5_runtime_process_executable_bindings']['activation']['executable_sha256']='wrong'
        with self.assertRaises(ValueError): m.validate_supplement(bad, c, processes)

    def test_unpinned_and_tampered_evidence_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp)/'receipt.json'; p.write_text('{}')
            for pin in (None, '0'*64):
                with self.assertRaises(ValueError): m.read_pinned(str(p), pin)
        with self.assertRaises((ValueError, KeyError)): m.validate_supplement({}, {}, {})

if __name__ == '__main__': unittest.main()
