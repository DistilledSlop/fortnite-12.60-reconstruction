#!/usr/bin/env python3
"""Import the independently audited, bounded internal button diagnostic."""
import hashlib
import json
from pathlib import Path

CHECKPOINT = 'porting/generated/material_pipeline/frontend-button-internal-runtime-checkpoint-results-1.json'
ROOT = 'porting/generated/frontend-native-evidence/button-internal-runtime-1/root-report.json'
AUDIT = 'porting/generated/frontend-native-evidence/button-internal-runtime-1/integrity-audit-1.json'
RELEASE = '/mnt/vibrato/fortnite-evidence/frozen-runtime/button-internal-runtime-1/source-release.json'
# Fixed independent evidence pins; missing or altered files fail closed.
PINS = {
    CHECKPOINT: 'c9f4c509cf39a9e67afd8ca2fcdb1e79a74c4a99eb37660d4c205c79dbe6a223',
    ROOT: '6b5ecdd20e41d5d2afbacaae322843b2e745a58486c77b618939fc1b724bc527',
    AUDIT: 'b1220c97b767069d7573b1e7063b3af27c986e0ce7d852e8070d6bca5e52a503',
    RELEASE: '20ac4b3367c755dad48caa8bdb23745eba3bebfcbdb99eba7efc409c0627b7a8',
}
SOURCE = '82b35764ba1c2dbd1a62a59b8b7b50aadf44bf82'
ENGINE = 'd7e04d4b65fb8ee464082849414cf5f7ae4d7bca'
CASES = {'button', 'activation', 'registry', 'lifecycle'}


def require(condition, message):
    if not condition:
        raise ValueError(message)


def read_pinned(path, digest):
    require(isinstance(digest, str) and len(digest) == 64, 'missing pin: ' + str(path))
    data = Path(path).read_bytes()
    require(hashlib.sha256(data).hexdigest() == digest, 'hash mismatch: ' + str(path))
    return json.loads(data)


def read_ref(ref):
    path = ref['path']
    require(Path(path).stat().st_size == ref['size'], 'size mismatch: ' + path)
    return read_pinned(path, ref['sha256'])


def validate(c, root, builds, sources, stage, processes, transcripts, joins):
    require(c['schema'] == 'button-internal-runtime-checkpoint-v1' and c['phase_passed'] is True and c['native_source_commit'] == SOURCE and c['engine_commit'] == ENGINE, 'checkpoint')
    require(c['root_report']['sha256'] == PINS[ROOT] and c['button_rows'] == 34 and c['milestones_completed'] == 0, 'checkpoint bindings')
    require(root['schema'] == 'button-internal-root-verification-v1' and root['passed'] is True and root['source_commit'] == SOURCE and root['engine_commit'] == ENGINE and root['button_rows'] == 34 and root['binary_rows_verified'] == 18, 'root report')
    require(sources['commit'] == SOURCE and sources['engine_head'] == ENGINE, 'source snapshot')
    require(len(builds) == 2 and {b['target'] for b in builds} == {'FortniteLinux', 'FortniteLinuxServer'}, 'build targets')
    for b in builds:
        require(b['source_head'] == SOURCE and b['exit'] == 0 and b['source_tree_unchanged'] is True and b['end_ns'] > b['start_ns'] and b['elapsed_seconds'] > 0, 'build result')
    require(len(root['builds']) == 2 and {b['target'] for b in root['builds']} == {'FortniteLinux', 'FortniteLinuxServer'} and all(b['exit'] == 0 and b['source_tree_unchanged'] is True for b in root['builds']), 'root builds')
    require(stage['rows'] and root['processes'] == c['processes'] and set(processes) == set(transcripts) == set(c['processes']) == CASES, 'process inventory')
    linux = next(b for b in builds if b['target'] == 'FortniteLinux')
    for name in CASES:
        p, t, row = processes[name], transcripts[name], c['processes'][name]
        require(p['schema'] == 'button-diagnostic-run-v1' and p['case'] == name and p['passed'] is True and p['returncode'] == row['returncode'] == 0 and p['pid'] == row['pid'] and p['timed_out'] is False and p['proc_exe_inode_verified'] is True and p['binary_after_verified'] is True and p['start_ns'] > max(b['end_ns'] for b in builds) and p['end_ns'] > p['start_ns'], 'process: ' + name)
        require(p['build_results']['sha256'] == c['build_results']['sha256'] and p['build'] == linux, 'process build: ' + name)
        require(t['case'] == name and t['passed'] is True, 'transcript: ' + name)
    require(len({p['pid'] for p in processes.values()}) == 4, 'distinct processes')
    require(transcripts['button']['detail']['passed'] is True and transcripts['button']['detail']['rows'] == processes['button']['transcript']['rows'] == 34 and processes['button']['transcript']['passed'] is True, 'button rows')
    require(transcripts['activation']['detail']['pass'] is True and transcripts['activation']['detail']['rows'] == 40 and transcripts['registry']['detail']['loaded_reflection']['passed'] is True, 'retained checks')
    require(len(joins) == 2 and {j['case'] for j in joins} == {'default', 'athena'} and all(j['exit'] == 0 and j['end_ns'] > j['start_ns'] and j['elapsed_seconds'] > 0 for j in joins), 'joins')
    require(len(root['joins']) == 2 and {j['case'] for j in root['joins']} == {'default', 'athena'} and all(j['exit'] == 0 for j in root['joins']), 'root joins')


def validate_audit_release(audit, release):
    require(release['schema'] == 'button-internal-source-release-v1' and release['released'] is True and release['native_source_build_stage_released'] is True and release['all_phase_processes_terminal'] is True and release['checkpoint']['sha256'] == PINS[CHECKPOINT], 'release')
    require(audit['schema'] == 'button-internal-integrity-audit-v1' and audit['passed'] is True, 'independent audit')
    require(audit['checkpoint_pin']['sha256'] == PINS[CHECKPOINT] and audit['root_report_pin']['sha256'] == PINS[ROOT] and audit['release_pin']['sha256'] == PINS[RELEASE], 'audit bindings')
    require(audit['artifact_count'] == 40 and audit['binary_rows'] == 18 and audit['critical_source_count'] == 9 and audit['native_source_count'] == 12553 and audit['process_count'] == 4 and audit['button_rows'] == 34 and audit['joins_count'] == 2 and audit['activation_rows'] == 40 and audit['build_receipts'] == 2 and audit['milestones_completed'] == 0 and audit['mismatches'] == [], 'audit inventory')


def main():
    docs = {path: read_pinned(path, digest) for path, digest in PINS.items()}
    c = docs[CHECKPOINT]
    builds, sources, stage, joins = (read_ref(c[key]) for key in ('build_results', 'source_snapshot', 'stage', 'joins'))
    processes = {name: read_ref(row['receipt']) for name, row in c['processes'].items()}
    transcripts = {name: read_ref(row['transcript']) for name, row in c['processes'].items()}
    read_ref(c['artifact_manifest'])
    validate(c, docs[ROOT], builds, sources, stage, processes, transcripts, joins)
    validate_audit_release(docs[AUDIT], docs[RELEASE])
    path = Path('porting/progress/evidence.json')
    ledger = json.loads(path.read_text())
    check = 'frontend_commonui_internal_button_runtime'
    ledger['checks'][check] = {'id': check, 'result': 'pass', 'exit_code': 0,
        'command': 'python3 -I porting/progress/record_frontend_button_runtime.py',
        'checkpoint': CHECKPOINT, 'checkpoint_sha256': PINS[CHECKPOINT],
        'runtime_report': ROOT, 'runtime_report_sha256': PINS[ROOT],
        'integrity_audit': AUDIT, 'integrity_audit_sha256': PINS[AUDIT],
        'scope': 'Controlled real UObject and Slate internal button factory, lifecycle, core input and callback diagnostic; two builds, four live processes and two joins. No new exhaustive original UHT/ELF audit or material rerun; main CommonButton, Panel, queue, full UI, gesture and parity remain unverified.',
        'provenance_note': 'Pinned external receipts and independently audited artifacts must remain available for re-verification.'}
    path.write_text(json.dumps(ledger, indent=1) + '\n')


if __name__ == '__main__':
    main()
