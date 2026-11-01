#!/usr/bin/env python3
"""Synthetic negative tests for the bounded context runtime importer."""
import copy
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('recorder', Path(__file__).with_name('record_frontend_context_runtime.py'))
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)

def fixture():
    props = [dict(name=n, actual_flags=3, expected_flags=3, raw_flags=1, computed_flags=2, array_dim=1) for n in ('ReturnValue', 'Class', 'ContextObject')]
    refl = dict(passed=True, properties=props)
    c = dict(schema='fortnite.material-runtime-checkpoint.v1', phase='context-generic-runtime-3', phase_passed=True, intermediate=False, native_commit='95ff94f', engine_commit='d7e04d4b6', source_files=[dict(repository='project', revision='95ff94f') for _ in range(122)], binaries=[], artifacts=[{} for _ in range(1416)], context_build_finish=dict(sha256='build'), context_registry_run=dict(sha256='registry'), context_lifecycle_run=dict(sha256='lifecycle'), actual_loaded_reflection=refl, fresh_metadata_snapshots=[], limitations=['concrete FrontEndGlobalUI unverified'])
    r = dict(schema='context-live-verification-v1', passed=True, build_sha256='build', metadata={}, live_cases={}, limitations='concrete subsystem init/tick/service behavior remain unverified')
    for target in ('FortniteLinux', 'FortniteLinuxServer'):
        artifacts = {}
        for kind in ('cpp', 'object'):
            path = target + kind; artifacts[path] = kind
            c['fresh_metadata_snapshots'].append(dict(target=target, kind=kind, source=path, sha256=kind))
        path = target + 'exe'; artifacts[path] = 'exe'
        c['binaries'].append(dict(target=target, role='source', artifact_kind='executable', path=path, sha256='exe'))
        r['metadata'][target] = dict(passed=True, function_original_hex='1', linked_function_hex='2', original_property_hex=['1']*3, artifacts=artifacts)
    c['binaries'].extend(dict(target='other') for _ in range(16))
    for name in ('registry', 'lifecycle'):
        r['live_cases'][name] = dict(run_sha256=name, log_sha256='log', start_ns=1, end_ns=2, observer=dict(owner=dict(gi='g', manager='m')))
    r['live_cases']['registry']['loaded_reflection'] = copy.deepcopy(refl)
    a = dict(schema='fortnite.context-phase3-integrity-audit.v1', passed=True, phase_passed=True, checkpoint=dict(sha256=m.PINS[m.CHECKPOINT]), independent_root_strict_report=dict(sha256=m.PINS[m.REPORT], passed=True), source_rows_verified=122, unique_source_bindings_verified=122, artifacts_verified=1416, unique_artifact_bindings=1416, binary_rows_verified=18, source_release=dict(all_build_runtime_jobs_ended=True, source_released=True), milestones=dict(authentic_startup_verified=False, interactive_ui_verified=False, google_applied=False, all_four_product_milestones=False))
    return c, r, a

class RuntimeImportTests(unittest.TestCase):
    def test_accepts_complete_synthetic_proof(self):
        m.validate(*fixture())

    def test_rejects_tampering(self):
        def missing_metadata(c,r,a): del r['metadata']['FortniteLinux']
        def missing_case(c,r,a): del r['live_cases']['registry']
        def source(c,r,a): c['source_files'][0]['revision']='wrong'
        def binary(c,r,a): c['binaries'][0]['sha256']='wrong'
        def claim(c,r,a): r['limitations']='concrete behavior verified'
        def phase(c,r,a): c['phase_passed']=False
        def report(c,r,a): r['passed']=False
        def audit(c,r,a): a['passed']=False
        def reflection(c,r,a): r['live_cases']['registry']['loaded_reflection']['properties'].pop()
        for mutate in (missing_metadata, missing_case, source, binary, claim, phase, report, audit, reflection):
            with self.subTest(mutate=mutate.__name__):
                args=fixture(); mutate(*args)
                with self.assertRaises((ValueError, KeyError)):
                    m.validate(*args)

    def test_hash_pin_rejects_modified_receipt(self):
        import tempfile
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp)/'receipt.json'; p.write_text('{}')
            with self.assertRaises(ValueError): m.read_pinned(str(p), '0'*64)

if __name__ == '__main__': unittest.main()
