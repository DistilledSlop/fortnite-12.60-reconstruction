#!/usr/bin/env python3
"""Regression checks for the bounded Google owner importer."""
import copy
import importlib.util
import tempfile
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('google', Path(__file__).with_name('record_frontend_google_runtime.py'))
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)

def fixture():
    release = dict(all_phase_jobs_ended=True, source_release=True, engine_build_stage_ownership_released=True)
    runs = [dict(name='google-'+n, exit_status=0, strict_exit_status=0, command=['-FortniteLegacyMaterialProbeOwner='+m.OWNER, '-FortniteLegacyGoogleProfile='+n]) for n in ('defaults','combined')]
    c = dict(schema='fortnite.material-runtime-checkpoint.v1', phase='google-runtime-2', phase_passed=True, intermediate=False, native_commit=m.NATIVE, engine_commit='d7e04d4b6', source_files=[{}]*202, binaries=[{}]*18, artifacts=[{}]*1693, context_build_finish={'sha256':'build'}, google_profiles=runs)
    a = dict(schema='fortnite.google-phase2-integrity-audit.v1', scope_integrity_passed=True, phase_passed=True, checkpoint={'sha256':m.PINS[m.CHECKPOINT]}, pins=dict(native_commit=m.NATIVE, engine_commit=c['engine_commit']), counts=dict(source_rows=202, unique_source_bindings_verified=202, binary_rows=18, artifact_rows=1693), source_release=release)
    context = dict(schema='context-live-verification-v1', passed=True, build_sha256='build')
    def report(n):
        return dict(passed=True, verified_shader_payloads=7, owner=dict(graph_cases=8,owner_cases=8), source=dict(mip_count=10), sampling=dict(numeric_profile=n,named_proxy_calls=1,checker_changed_pixels=4096,source_sha1='x',copy_sha1='x',raw_bc1_gpu_readback_measured=False), fixture_sha256='fixture', shader_fixture_sha256='shader')
    root = dict(schema='fortnite.google.root-runtime-audit.v1', passed=True, checkpoint={'sha256':m.PINS[m.CHECKPOINT]}, integrity_audit={'sha256':m.PINS[m.AUDIT]}, context_strict_report={'sha256':m.PINS[m.CONTEXT]}, google_strict_reports=[dict(profile=n, sha256=m.PINS[p]) for n,p in [('defaults',m.DEFAULTS),('combined',m.COMBINED)]], native_commit=m.NATIVE, engine_commit=c['engine_commit'], source_release=copy.deepcopy(release))
    return c,a,context,report('defaults'),report('combined'),root

class GoogleRuntimeTests(unittest.TestCase):
    def test_accepts_complete_proof(self):
        m.validate(*fixture())

    def test_rejects_tampering(self):
        def phase(c,a,x,d,b,r): c['phase_passed']=False
        def owner(c,a,x,d,b,r): c['google_profiles'][0]['command'][0]='wrong'
        def audit(c,a,x,d,b,r): a['checkpoint']['sha256']='wrong'
        def context(c,a,x,d,b,r): x['build_sha256']='wrong'
        def texture(c,a,x,d,b,r): d['sampling']['copy_sha1']='wrong'
        def profile(c,a,x,d,b,r): b['sampling']['numeric_profile']='defaults'
        def reports(c,a,x,d,b,r): r['google_strict_reports'][1]['sha256']='wrong'
        def release(c,a,x,d,b,r): a['source_release']['source_release']=False
        for mutate in (phase,owner,audit,context,texture,profile,reports,release):
            with self.subTest(mutate=mutate.__name__):
                args=fixture(); mutate(*args)
                with self.assertRaises((ValueError,KeyError)): m.validate(*args)

    def test_hash_rejects_changed_receipt(self):
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp)/'receipt'; p.write_text('{}')
            with self.assertRaises(ValueError): m.read_pinned(str(p),'0'*64)

if __name__ == '__main__': unittest.main()
