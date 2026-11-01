#!/usr/bin/env python3
"""Import the independently audited, bounded Panel intro diagnostic."""
import hashlib
import json
from pathlib import Path

CHECKPOINT = 'porting/generated/material_pipeline/frontend-panel-intro-runtime-checkpoint-results-2.json'
ROOT = 'porting/generated/frontend-native-evidence/panel-intro-runtime-2/root-report.json'
AUDIT = 'porting/generated/frontend-native-evidence/panel-intro-runtime-2/integrity-audit-1.json'
RELEASE = '/mnt/vibrato/fortnite-evidence/frozen-runtime/panel-intro-runtime-2/source-release.json'
# Fixed independent evidence pins; missing or altered files fail closed.
PINS = {
    CHECKPOINT: '6ee95ee5892bc85f03e60fbaaf867c9c7d62fb57c1ef6e64c05259db49af78ea',
    ROOT: '76745b7bfd04f1920889196bda99d8fb9a2309a47997fdc9b7697b2a0cd5c2b0',
    AUDIT: 'df1101e05c74d0523af8f8d8d67a25b6c13f1385a0f3339369097321ad5031ce',
    RELEASE: 'c27b7b998f37bc1ebc886e3895b47f86ca48bdbcf1d0df0b381b49903de6a931',
}
SOURCE = 'b06234ec750f94286836e5c7830d3eb2921c131b'
ENGINE = 'd7e04d4b65fb8ee464082849414cf5f7ae4d7bca'
CASES = {'panel-intro', 'main-button', 'button', 'activation', 'registry', 'lifecycle'}


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


def validate(c, root, builds, sources, stage, processes, transcripts, joins, observation):
    require(c['schema'] == 'panel-intro-runtime-checkpoint-v2' and c['phase_passed'] is True and c['native_source_commit'] == SOURCE and c['engine_commit'] == ENGINE, 'checkpoint')
    require(c['root_report']['sha256'] == PINS[ROOT] and c['button_rows'] == 34 and c['milestones_completed'] == 0, 'checkpoint bindings')
    require(root['schema'] == 'panel-intro-root-verification-v2' and root['passed'] is True and root['source_commit'] == SOURCE and root['engine_commit'] == ENGINE and root['button_rows'] == 34 and root['binary_rows_verified'] == 18, 'root report')
    require(sources['commit'] == SOURCE and sources['engine_head'] == ENGINE and len(sources['source_tree']) == 12559 and root['native_source_files'] == 12559, 'source snapshot')
    require(len(builds) == 2 and {b['target'] for b in builds} == {'FortniteLinux', 'FortniteLinuxServer'}, 'build targets')
    for b in builds:
        require(b['source_head'] == SOURCE and b['exit'] == 0 and b['source_tree_unchanged'] is True and b['end_ns'] > b['start_ns'] and b['elapsed_seconds'] > 0, 'build result')
    require(len(root['builds']) == 2 and {b['target'] for b in root['builds']} == {'FortniteLinux', 'FortniteLinuxServer'} and all(b['exit'] == 0 and b['source_tree_unchanged'] is True for b in root['builds']), 'root builds')
    require(stage['build_source'] == SOURCE and len(stage['rows']) == root['binary_rows_verified'] == 18 and root['processes'] == c['processes'] and set(processes) == set(transcripts) == set(c['processes']) == CASES, 'process inventory')
    linux = next(b for b in builds if b['target'] == 'FortniteLinux')
    for name in CASES:
        p, t, row = processes[name], transcripts[name], c['processes'][name]
        require(p['schema'] == 'button-diagnostic-run-v1' and p['case'] == name and p['passed'] is True and p['returncode'] == row['returncode'] == 0 and p['pid'] == row['pid'] and p['timed_out'] is False and p['proc_exe_inode_verified'] is True and p['binary_after_verified'] is True and p['start_ns'] > max(b['end_ns'] for b in builds) and p['end_ns'] > p['start_ns'], 'process: ' + name)
        require(p['build_results']['sha256'] == c['build_results']['sha256'] and p['build'] == linux, 'process build: ' + name)
        require(t['case'] == name and t['passed'] is True, 'transcript: ' + name)
    require(len({p['pid'] for p in processes.values()}) == 6, 'distinct processes')
    for name, count in [('panel-intro', 49), ('main-button', 44), ('button', 34)]:
        require(transcripts[name]['detail']['passed'] is True and transcripts[name]['detail']['rows'] == processes[name]['transcript']['rows'] == count and processes[name]['transcript']['passed'] is True, 'rows: ' + name)
    require(c['panel_intro_rows'] == root['panel_intro_rows'] == 49 and c['main_button_rows'] == root['main_button_rows'] == 44 and c['main_button_states'] == root['main_button_states'] == 40, 'row inventory')
    require(transcripts['activation']['detail']['pass'] is True and transcripts['activation']['detail']['rows'] == 40 and transcripts['registry']['detail']['loaded_reflection']['passed'] is True, 'retained checks')
    require(len(joins) == 2 and {j['case'] for j in joins} == {'default', 'athena'} and all(j['exit'] == 0 and j['end_ns'] > j['start_ns'] and j['elapsed_seconds'] > 0 for j in joins), 'joins')
    require(len(root['joins']) == 2 and {j['case'] for j in root['joins']} == {'default', 'athena'} and all(j['exit'] == 0 and j['elapsed_seconds'] == next(x['elapsed_seconds'] for x in joins if x['case'] == j['case']) for j in root['joins']), 'root joins')

    require(root['root_class_load_observation'] == c['root_class_load_observation'] and root['root_class_load_exit'] == observation['exit'] == 139 and observation['timed_out'] is False and observation['binary_after_verified'] is True and observation['previous_on_begin_intro_missing_dependency_resolved'] is True and any('OnBeginOutro' in x for x in observation['native_dependency_details']) and 'not interactive UI' in observation['scope'], 'root load observation')


def validate_audit_release(audit, release):
    require(release['schema'] == 'panel-intro-source-release-v2' and release['released'] is True and release['native_source_build_stage_released'] is True and release['all_phase_processes_terminal'] is True and release['checkpoint']['sha256'] == PINS[CHECKPOINT], 'release')
    require(audit['schema'] == 'panel-intro-integrity-audit-v1' and audit['passed'] is True, 'independent audit')
    require(audit['checkpoint_pin'] == PINS[CHECKPOINT] and audit['root_report_pin'] == PINS[ROOT] and audit['release_pin'] == PINS[RELEASE], 'audit bindings')
    require(audit['artifact_count'] == 74 and audit['binary_rows'] == 18 and audit['critical_source_count'] == 22 and audit['native_source_count'] == 12559 and audit['process_count'] == 6 and audit['panel_intro_rows'] == 49 and audit['main_button_rows'] == 44 and audit['button_rows'] == 34 and audit['activation_rows'] == 40 and audit['joins_count'] == 2 and audit['build_receipts'] == 2 and audit['milestones_completed'] == 0 and audit['root_missing_dependency'] is True and audit['mismatches'] == [], 'audit inventory')


def main():
    docs = {path: read_pinned(path, digest) for path, digest in PINS.items()}
    c = docs[CHECKPOINT]
    builds, sources, stage, joins = (read_ref(c[key]) for key in ('build_results', 'source_snapshot', 'stage', 'joins'))
    processes = {name: read_ref(row['receipt']) for name, row in c['processes'].items()}
    transcripts = {name: read_ref(row['transcript']) for name, row in c['processes'].items()}
    read_ref(c['artifact_manifest'])
    observation = read_ref(c['root_class_load_observation'])
    validate(c, docs[ROOT], builds, sources, stage, processes, transcripts, joins, observation)
    validate_audit_release(docs[AUDIT], docs[RELEASE])
    path = Path('porting/progress/evidence.json')
    ledger = json.loads(path.read_text())
    check = 'frontend_commonui_panel_intro_runtime'
    ledger['checks'][check] = {'id': check, 'result': 'pass', 'exit_code': 0,
        'command': 'python3 -I porting/progress/record_frontend_panel_intro_runtime.py',
        'checkpoint': CHECKPOINT, 'checkpoint_sha256': PINS[CHECKPOINT],
        'runtime_report': ROOT, 'runtime_report_sha256': PINS[ROOT],
        'integrity_audit': AUDIT, 'integrity_audit_sha256': PINS[AUDIT],
        'scope': 'Controlled real Panel intro, EndIntro and full activation path with input and priority fixture and C++ ProcessEvent interception; two builds, six processes and two joins. Blueprint VM, whole Panel, outro, deactivation, input producer, manager, full UI and parity remain unverified. No new exhaustive original metadata, link, material or pixel checks.',
        'provenance_note': 'Pinned external receipts and independently audited artifacts must remain available for re-verification.'}
    path.write_text(json.dumps(ledger, indent=1) + '\n')


if __name__ == '__main__':
    main()
