#!/usr/bin/env python3
"""Import only the independently audited phase-5 base activation diagnostic."""
import hashlib
import json
from pathlib import Path

CHECKPOINT = 'porting/generated/material_pipeline/frontend-activation-runtime-checkpoint-results-5.json'
BASE = 'porting/generated/frontend-native-evidence/activation-runtime-5/'
OWNER = BASE + 'report.json'
CONTEXT = BASE + 'context-report.json'
ROOT = BASE + 'root-report-1.json'
ROOT_CONTEXT = BASE + 'root-context-report-1.json'
AUDIT = BASE + 'integrity-audit-1.json'
SUPPLEMENT = BASE + 'integrity-audit-2.json'
RELEASE = '/mnt/vibrato/fortnite-evidence/frozen-runtime/activation-frozen-runtime-5/source-release.json'
PINS = {
    CHECKPOINT: 'cb324a725852bbe89fced9a13d5422a7a589b04667faa0ec98921a6c5f1124e3',
    OWNER: 'f00bc357859ba6d9a6fed7daa0242b7febdbec218b0e00edfbbb22c67f679bfc',
    CONTEXT: '36d83c0bfe943eeb916c20239d998c142d2118cb8012cb79cd4188d52f0a1e1e',
    RELEASE: '2be936b1f660f3cb701369c923ccdafa52109a66b43398a9f2571175d07def1c',
    ROOT: 'f00bc357859ba6d9a6fed7daa0242b7febdbec218b0e00edfbbb22c67f679bfc',
    ROOT_CONTEXT: '36d83c0bfe943eeb916c20239d998c142d2118cb8012cb79cd4188d52f0a1e1e',
    AUDIT: 'a0fe10546c7e37854d95219af39eaf91437b6029a87c070ad214584ed421b771',
    SUPPLEMENT: '4c11866f0c1b479d57b257e3dd463211280531d1d0fdf793f771db1f7bb60ae4',
}

def require(condition, message):
    if not condition:
        raise ValueError(message)

def read_pinned(path, digest):
    require(isinstance(digest, str) and len(digest) == 64, 'missing pin: ' + path)
    data = Path(path).read_bytes()
    require(hashlib.sha256(data).hexdigest() == digest, 'hash mismatch: ' + path)
    return json.loads(data)

def validate(c, owner, context, root, root_context, release, audit, retry, processes):
    require(c['schema'] == 'activation-runtime-checkpoint-v5' and c['phase_passed'] is True and c['failure'] is None, 'checkpoint phase')
    require(c['native_source_commit'] == 'c06a5cb48327455b91a3a4ee9340a550d741bb29' and c['engine_commit'] == 'd7e04d4b65fb8ee464082849414cf5f7ae4d7bca' and c['tool_head'] == '973d38e33deb07ce9864875f3893e599d3ec868e', 'revisions')
    require(c['new_artifact_count'] == 639 and c['runtime_manifest']['sha256'] == '080c82f43e26ec6ce90c4ad4cba22f008cb475b316d1886abf8e2fa4af4d7319' and c['inherited_binary_rows'] == 18 and c['inherited_intermediate_rows'] == 42, 'artifact inventory')
    require({x['target'] for x in c['inherited_builds']} == {'FortniteLinux', 'FortniteLinuxServer'} and all(x['exit'] == 0 and x['seconds'] > 0 for x in c['inherited_builds']) and 'no new compile' in c['build_origin'], 'inherited builds')
    require(c['strict_report']['sha256'] == PINS[OWNER] and c['context_report']['sha256'] == PINS[CONTEXT], 'owner report bindings')
    require(release['schema'] == 'activation-source-release-v1' and release['phase'] == 'activation-runtime-5' and release['released'] is True and release['native_source_build_stage_released'] is True and release['all_phase_processes_terminal'] is True and release['checkpoint_sha256'] == PINS[CHECKPOINT], 'source release')
    require(retry['schema'] == 'activation-runtime-retry-v1' and set(processes) == set(c['processes']) == {'activation', 'registry', 'lifecycle'}, 'retry cases')
    for name, p in processes.items():
        row = c['processes'][name]
        require(p['schema'] == 'context-run-v2' and p['case'] == name and p['exit'] == row['exit'] == 0 and p['timed_out'] is row['timed_out'] is False and p['pid'] == row['pid'] and p['proc_exe_inode_verified'] is row['inode_verified'] is True and p['binary_after_verified'] is row['binary_after_verified'] is True and retry['start_ns'] < p['start_ns'] < p['end_ns'], 'fresh process: ' + name)
    require(len({p['pid'] for p in processes.values()}) == 3 and len({p['binary_sha256'] for p in processes.values()}) == 1, 'process identity')
    for report, ctx in ((owner, context), (root, root_context)):
        require(report['schema'] == 'activation-live-verification-v2' and report['passed'] is True and report['full_ready'] is True and report['runtime_provenance_verified'] is True and report['gaps'] == [], 'strict activation report')
        require(report['context_report_sha256'] == (PINS[CONTEXT] if report is owner else PINS[ROOT_CONTEXT]) and report['retry_start_sha256'] == c['retry_receipt']['sha256'] and report['run_sha256'] == c['processes']['activation']['receipt']['sha256'], 'report bindings')
        require(report['transcript']['pass'] is True and report['transcript']['rows'] == 40, 'activation cases')
        require(set(report['metadata']) == {'FortniteLinux', 'FortniteLinuxServer'}, 'metadata targets')
        for metadata in report['metadata'].values():
            require(metadata['passed'] is True and metadata['implementation_link']['passed'] is True and set(metadata['functions']) == {'ActivateWidget', 'DeactivateWidget', 'IsActivated'}, 'metadata and linked implementation')
        require(ctx['schema'] == 'context-live-verification-v1' and ctx['passed'] is True and set(ctx['live_cases']) == {'registry', 'lifecycle'}, 'context report')
        for name in ('registry', 'lifecycle'):
            require(ctx['live_cases'][name]['run_sha256'] == c['processes'][name]['receipt']['sha256'], 'context process binding')
    # Audit schema and content are checked after the independent result is available.
    validate_audit(audit)

def validate_audit(audit):
    require(audit['audit'] == 'activation-runtime-5 integrity audit' and audit['passed'] is True, 'integrity audit')
    require(audit['checkpoint']['sha256'] == PINS[CHECKPOINT] and audit['checkpoint']['phase_passed'] is True and audit['checkpoint']['fresh_builds'] is False, 'audit checkpoint')
    require(audit['archive']['entries'] == 639 and audit['archive']['sha256'] == '080c82f43e26ec6ce90c4ad4cba22f008cb475b316d1886abf8e2fa4af4d7319' and audit['archive']['mismatches'] == [], 'audit archive')
    require(audit['inherited_phase2']['binary_row_count_claimed'] == 18 and audit['inherited_phase2']['binary_artifact_count'] == 6 and audit['inherited_phase2']['metadata_object_snapshot_count'] == 42 and audit['inherited_phase2']['metadata_object_snapshot_mismatches'] == [], 'audit inheritance')
    require(audit['strict_report']['sha256'] == PINS[OWNER] and audit['strict_report']['passed'] is True and audit['strict_report']['full_ready'] is True and audit['strict_report']['activation_rows'] == 40 and audit['context_report']['sha256'] == PINS[CONTEXT] and audit['context_report']['passed'] is True, 'audit reports')
    require(audit['release']['sha256'] == PINS[RELEASE] and audit['release']['ownership_released'] is True and audit['release']['all_phase_processes_terminal'] is True and audit['release']['checkpoint_hash_bound'] is True, 'audit release')
    require(audit['retained_results_reference_count'] == 15 and audit['retained_results_reference_mismatches'] == [] and len(audit['tool_input_blob_checks']) == 5 and all(x['tracked_blob_matches'] is True for x in audit['tool_input_blob_checks']), 'audit retained evidence')

def validate_supplement(supplement, c, processes):
    require(supplement['audit'] == 'activation-runtime-5 inherited phase2 binary integrity supplement' and supplement['passed'] is True and supplement['supplement_to_sha256'] == PINS[AUDIT], 'binary audit')
    require(supplement['binary_rows'] == supplement['distinct_physical_inodes_hashed'] == 18 and supplement['role_rows_verified'] == {'snapshot': 6, 'source': 6, 'stage': 6} and supplement['mismatch_paths'] == supplement['missing_or_stat_errors'] == supplement['role_identity_mismatches'] == [], 'binary audit inventory')
    require(len(supplement['identities']) == 6 and all(row['row_count'] == 3 and set(row['roles']) == {'snapshot', 'source', 'stage'} and len(row['paths']) == 3 and row['sha256'] and row['size'] > 0 for row in supplement['identities']), 'binary audit identities')
    require(supplement['process_executable_hash_bound_to_phase2_executable'] is True and set(supplement['phase5_runtime_process_executable_bindings']) == set(processes), 'binary audit process bindings')
    for name, row in supplement['phase5_runtime_process_executable_bindings'].items():
        require(row['exit'] == 0 and row['receipt_sha256'] == c['processes'][name]['receipt']['sha256'] and row['executable_sha256'] == processes[name]['binary_sha256'], 'binary audit process: ' + name)

def main():
    docs = {path: read_pinned(path, digest) for path, digest in PINS.items()}
    c = docs[CHECKPOINT]
    retry = read_pinned(c['retry_receipt']['path'], c['retry_receipt']['sha256'])
    processes = {name: read_pinned(row['receipt']['path'], row['receipt']['sha256']) for name, row in c['processes'].items()}
    validate_supplement(docs[SUPPLEMENT], c, processes)
    validate(c, docs[OWNER], docs[CONTEXT], docs[ROOT], docs[ROOT_CONTEXT], docs[RELEASE], docs[AUDIT], retry, processes)
    path = Path('porting/progress/evidence.json')
    ledger = json.loads(path.read_text())
    check = 'frontend_commonui_base_activation_runtime'
    ledger['checks'][check] = {'id': check, 'result': 'pass', 'exit_code': 0,
        'command': 'python3 -I porting/progress/record_frontend_activation_runtime.py',
        'checkpoint': CHECKPOINT, 'checkpoint_sha256': PINS[CHECKPOINT],
        'strict_report': ROOT, 'strict_report_sha256': PINS[ROOT],
        'integrity_audit': AUDIT, 'integrity_audit_sha256': PINS[AUDIT],
        'binary_audit': SUPPLEMENT, 'binary_audit_sha256': PINS[SUPPLEMENT],
        'scope': 'Bounded CommonUI base activation diagnostic with both-target original UHT metadata and linked implementation; inherited phase-2 builds and three fresh runtime processes. Panel, input, full CommonUI, authentic startup, interactive UI and all four milestones remain unverified.',
        'provenance_note': 'Pinned external receipts and independently audited source and artifact bindings must remain available for re-verification.'}
    path.write_text(json.dumps(ledger, indent=1) + '\n')

if __name__ == '__main__':
    main()
