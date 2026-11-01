#!/usr/bin/env python3
"""Import the fixed generic context runtime audit; no product milestone credit."""
import hashlib
import json
from pathlib import Path

CHECKPOINT = 'porting/generated/material_pipeline/frontend-context-checkpoint-results-3.json'
REPORT = 'porting/generated/frontend-native-evidence/context-runtime-3/root-report-1.json'
AUDIT = 'porting/generated/frontend-native-evidence/context-runtime-3/integrity-audit-1.json'
PINS = {
    CHECKPOINT: '22b6b87c942044d7f46067a660e1f1e25e289262810fc52cf878d868f88ca8e6',
    REPORT: 'ab5e924073e9559675093f9f8aabeb6f1cb5346caaed267f51713deda24713cb',
    AUDIT: 'd09c52d9d05333a680c40c6e000c9fa1b641f49333d7a4c66a1fe535a8a43828',
}
SCOPES = {
    'frontend_context_runtime_metadata': 'Exact original Native metadata in both targets: UHT, object and linked ELF.',
    'frontend_context_runtime_reflection': 'Three measured loaded properties and exact original flags plus source-proven computed bits.',
    'frontend_context_runtime_lookup': 'Real local player, game instance, player and widget lookup selection in diagnostic runtime.',
    'frontend_context_runtime_registry': 'Legacy registry replacement, GC retention, removal and deinitialization in diagnostic runtime.',
    'frontend_context_runtime_lifecycle': 'Actual world travel, deferred timer and copied-map dispatch in diagnostic runtime.',
}
LIMIT = 'Generic diagnostic runtime only; concrete context methods, authentic startup, playable UI and all four product milestones remain unverified.'

def read_pinned(path, digest):
    data = Path(path).read_bytes()
    if hashlib.sha256(data).hexdigest() != digest:
        raise ValueError('hash mismatch: ' + path)
    return json.loads(data)

def require(condition, message):
    if not condition:
        raise ValueError(message)

def validate(c, r, a):
    require(c['schema'] == 'fortnite.material-runtime-checkpoint.v1' and c['phase'] == 'context-generic-runtime-3' and c['phase_passed'] is True and c['intermediate'] is False, 'checkpoint phase')
    require(c['native_commit'] == '95ff94f' and c['engine_commit'] == 'd7e04d4b6', 'source revisions')
    require(len(c['source_files']) == 122 and len(c['binaries']) == 18 and len(c['artifacts']) == 1416, 'binding counts')
    require(all(row['revision'] == ('d7e04d4b6' if row['repository'] == '/opt/fortnite/UnrealEngine' else '95ff94f') for row in c['source_files']), 'source binding')
    require(r['schema'] == 'context-live-verification-v1' and r['passed'] is True, 'strict report')
    require(r['build_sha256'] == c['context_build_finish']['sha256'], 'build binding')
    require(set(r['metadata']) == {'FortniteLinux', 'FortniteLinuxServer'}, 'metadata targets')
    for target, metadata in r['metadata'].items():
        require(metadata['passed'] is True and metadata['function_original_hex'] and metadata['linked_function_hex'] and len(metadata['original_property_hex']) == 3, 'metadata proof')
        artifacts = metadata['artifacts']
        for kind in ('cpp', 'object'):
            rows = [x for x in c['fresh_metadata_snapshots'] if x['target'] == target and x['kind'] == kind]
            require(len(rows) == 1 and artifacts.get(rows[0]['source']) == rows[0]['sha256'], 'metadata artifact binding')
        bins = [x for x in c['binaries'] if x['target'] == target and x['role'] == 'source' and x['artifact_kind'] == 'executable']
        require(len(bins) == 1 and artifacts.get(bins[0]['path']) == bins[0]['sha256'], 'binary binding')
    require(set(r['live_cases']) == {'registry', 'lifecycle'}, 'runtime cases')
    for name in ('registry', 'lifecycle'):
        case = r['live_cases'][name]
        require(case['run_sha256'] == c['context_' + name + '_run']['sha256'] and case['log_sha256'] and case['start_ns'] < case['end_ns'] and case['observer']['owner']['gi'] and case['observer']['owner']['manager'], 'runtime binding')
    refl = r['live_cases']['registry']['loaded_reflection']
    require(refl == c['actual_loaded_reflection'] and refl['passed'] is True, 'reflection binding')
    props = refl['properties']
    require({p['name'] for p in props} == {'ReturnValue', 'Class', 'ContextObject'} and len(props) == 3, 'loaded properties')
    require(all(p['actual_flags'] == p['expected_flags'] == (p['raw_flags'] | p['computed_flags']) and p['array_dim'] == 1 for p in props), 'loaded flags')
    require(a['schema'] == 'fortnite.context-phase3-integrity-audit.v1' and a['passed'] is True and a['phase_passed'] is True, 'integrity audit')
    require(a['checkpoint']['sha256'] == PINS[CHECKPOINT] and a['independent_root_strict_report']['sha256'] == PINS[REPORT] and a['independent_root_strict_report']['passed'] is True, 'audit receipt bindings')
    require((a['source_rows_verified'], a['unique_source_bindings_verified'], a['artifacts_verified'], a['unique_artifact_bindings'], a['binary_rows_verified']) == (122, 122, 1416, 1416, 18), 'integrity counts')
    require(a['source_release']['all_build_runtime_jobs_ended'] is True and a['source_release']['source_released'] is True, 'source release')
    require(all(v is False for v in a['milestones'].values()) and set(a['milestones']) == {'authentic_startup_verified', 'interactive_ui_verified', 'google_applied', 'all_four_product_milestones'}, 'milestone claim')
    require('concrete subsystem init/tick/service behavior' in r['limitations'] and 'remain unverified' in r['limitations'] and any('concrete FrontEndGlobalUI' in s and 'unverified' in s for s in c['limitations']), 'concrete behavior claim')

def main():
    c, r, a = (read_pinned(path, digest) for path, digest in PINS.items())
    validate(c, r, a)
    path = Path('porting/progress/evidence.json')
    ledger = json.loads(path.read_text())
    for check_id, scope in SCOPES.items():
        ledger['checks'][check_id] = {'id': check_id, 'result': 'pass', 'exit_code': 0,
            'command': 'python3 -I porting/progress/record_frontend_context_runtime.py',
            'checkpoint': CHECKPOINT, 'checkpoint_sha256': PINS[CHECKPOINT],
            'strict_report': REPORT, 'strict_report_sha256': PINS[REPORT],
            'integrity_audit': AUDIT, 'integrity_audit_sha256': PINS[AUDIT],
            'scope': scope + ' ' + LIMIT,
            'provenance_note': 'Pinned external receipts and their independently audited source and artifact bindings must remain available for re-verification.'}
    path.write_text(json.dumps(ledger, indent=1) + '\n')

if __name__ == '__main__':
    main()
