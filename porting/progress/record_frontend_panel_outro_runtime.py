#!/usr/bin/env python3
"""Import the independently audited, bounded Panel outro diagnostic."""
import hashlib
import json
from pathlib import Path

CHECKPOINT = 'porting/generated/material_pipeline/frontend-panel-outro-runtime-checkpoint-results-1.json'
ROOT = 'porting/generated/frontend-native-evidence/panel-outro-runtime-1/root-report.json'
AUDIT = 'porting/generated/frontend-native-evidence/panel-outro-runtime-1/integrity-audit-1.json'
RELEASE = '/mnt/vibrato/fortnite-evidence/frozen-runtime/panel-outro-runtime-1/source-release.json'
# Fixed independent evidence pins; missing or altered files fail closed.
PINS = {CHECKPOINT: 'fe8c7bffce934727e3d1ec3af154839f9cd3361e056613538528257852e52219', ROOT: '2ec8ffb191e6bf18eca7efc868133756213c239e779379de27272844984d96f4', AUDIT: '0d4e3ed4418b60a535f6823935a237b44e239e4650b5e7a7c770397f66daaa0c', RELEASE: 'f383625757cc7f4c740a2223b4074c4df07e8f0f709c0f443142fbbaeec99607'}
SOURCE = 'a99655f487efb11a49fa94822206ac1401fa6785'
ENGINE = 'd7e04d4b65fb8ee464082849414cf5f7ae4d7bca'
CASES = {'panel-outro', 'panel-intro', 'main-button', 'button', 'activation', 'registry', 'lifecycle'}


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
    require(c['schema'] == 'panel-outro-runtime-checkpoint-v1' and c['phase_passed'] is True and c['native_source_commit'] == SOURCE and c['engine_commit'] == ENGINE, 'checkpoint')
    require(c['root_report']['sha256'] == PINS[ROOT] and c['button_rows'] == 34 and c['milestones_completed'] == 0, 'checkpoint bindings')
    require(root['schema'] == 'panel-outro-root-verification-v1' and root['passed'] is True and root['source_commit'] == SOURCE and root['engine_commit'] == ENGINE and root['button_rows'] == 34 and root['binary_rows_verified'] == 18, 'root report')
    require(sources['commit'] == SOURCE and sources['engine_head'] == ENGINE and len(sources['source_tree']) == 12568 and root['native_source_files'] == 12568, 'source snapshot')
    require(len(builds) == 2 and {b['target'] for b in builds} == {'FortniteLinux', 'FortniteLinuxServer'}, 'build targets')
    for b in builds:
        require(b['source_head'] == SOURCE and b['exit'] == 0 and b['source_tree_unchanged'] is True and b['end_ns'] > b['start_ns'] and b['elapsed_seconds'] > 0, 'build result')
    require(len(root['builds']) == 2 and {b['target'] for b in root['builds']} == {'FortniteLinux', 'FortniteLinuxServer'} and all(b['exit'] == 0 and b['source_tree_unchanged'] is True and b['elapsed_seconds'] == next(x['elapsed_seconds'] for x in builds if x['target'] == b['target']) for b in root['builds']), 'root builds')
    require(stage['build_source'] == SOURCE and len(stage['rows']) == root['binary_rows_verified'] == 18 and root['processes'] == c['processes'] and set(processes) == set(transcripts) == set(c['processes']) == CASES, 'process inventory')
    linux = next(b for b in builds if b['target'] == 'FortniteLinux')
    for name in CASES:
        p, t, row = processes[name], transcripts[name], c['processes'][name]
        require(p['schema'] == 'button-diagnostic-run-v1' and p['case'] == name and p['passed'] is True and p['returncode'] == row['returncode'] == 0 and p['pid'] == row['pid'] and p['timed_out'] is False and p['proc_exe_inode_verified'] is True and p['binary_after_verified'] is True and p['start_ns'] > max(b['end_ns'] for b in builds) and p['end_ns'] > p['start_ns'], 'process: ' + name)
        require(p['build_results']['sha256'] == c['build_results']['sha256'] and p['build'] == linux, 'process build: ' + name)
        require(t['case'] == name and t['passed'] is True, 'transcript: ' + name)
    require(len({p['pid'] for p in processes.values()}) == 7, 'distinct processes')
    for name, count in [('panel-outro', 60), ('panel-intro', 49), ('main-button', 44), ('button', 34)]:
        require(transcripts[name]['detail']['passed'] is True and transcripts[name]['detail']['rows'] == processes[name]['transcript']['rows'] == count and processes[name]['transcript']['passed'] is True, 'rows: ' + name)
    require(c['panel_outro_rows'] == root['panel_outro_rows'] == 60 and c['panel_intro_rows'] == root['panel_intro_rows'] == 49 and c['main_button_rows'] == root['main_button_rows'] == 44 and c['main_button_states'] == root['main_button_states'] == 40, 'row inventory')
    require(transcripts['activation']['detail']['pass'] is True and transcripts['activation']['detail']['rows'] == 40 and transcripts['registry']['detail']['loaded_reflection']['passed'] is True, 'retained checks')
    require(len(joins) == 2 and {j['case'] for j in joins} == {'default', 'athena'} and all(j['exit'] == 0 and j['end_ns'] > j['start_ns'] and j['elapsed_seconds'] > 0 for j in joins), 'joins')
    require(len(root['joins']) == 2 and {j['case'] for j in root['joins']} == {'default', 'athena'} and all(j['exit'] == 0 and j['elapsed_seconds'] == next(x['elapsed_seconds'] for x in joins if x['case'] == j['case']) for j in root['joins']), 'root joins')

    require(root['root_class_load_observation'] == c['root_class_load_observation'] and root['root_class_load_exit'] == observation['exit'] == 139 and observation['timed_out'] is False and observation['binary_after_verified'] is True and observation['previous_on_begin_intro_missing_dependency_resolved'] is True and observation['previous_on_begin_outro_missing_dependency_resolved'] is True and 'M_UI_ComicBubble_Inst' in observation['next_observed_material_boundary'] and 'MemoryImage.cpp:1064' in observation['next_observed_assertion'] and 'not interactive UI' in observation['scope'], 'root load observation')


def validate_audit_release(audit, release, manifest):
    require(release['schema'] == 'panel-outro-source-release-v1' and release['released'] is True and release['native_source_build_stage_released'] is True and release['all_phase_processes_terminal'] is True and release['checkpoint']['sha256'] == PINS[CHECKPOINT], 'release')
    require(audit['schema'] == 'panel-outro-integrity-audit-v1' and audit['passed'] is True, 'independent audit')
    require(audit['checkpoint_pin'] == PINS[CHECKPOINT] and audit['root_report_pin'] == PINS[ROOT] and audit['release_pin'] == PINS[RELEASE], 'audit bindings')
    require(manifest['schema'] == 'panel-outro-artifact-manifest-v1' and audit['artifact_count'] == len(manifest['artifacts']) == 88 and audit['binary_rows'] == len(manifest['binaries']) == 18 and audit['critical_source_count'] == len(manifest['critical_committed_sources']) == 53 and audit['native_source_count'] == 12568 and audit['process_count'] == 7 and audit['panel_outro_rows'] == 60 and audit['panel_intro_rows'] == 49 and audit['main_button_rows'] == 44 and audit['button_rows'] == 34 and audit['activation_rows'] == 40 and audit['joins_count'] == 2 and audit['build_receipts'] == 2 and audit['milestones_completed'] == 0 and audit['root_material_boundary'] is True and audit['root_stdout_verified'] is True and audit['mismatches'] == [], 'audit inventory')


def main():
    docs = {path: read_pinned(path, digest) for path, digest in PINS.items()}
    c = docs[CHECKPOINT]
    builds, sources, stage, joins = (read_ref(c[key]) for key in ('build_results', 'source_snapshot', 'stage', 'joins'))
    processes = {name: read_ref(row['receipt']) for name, row in c['processes'].items()}
    transcripts = {name: read_ref(row['transcript']) for name, row in c['processes'].items()}
    manifest = read_ref(c['artifact_manifest'])
    observation = read_ref(c['root_class_load_observation'])
    validate(c, docs[ROOT], builds, sources, stage, processes, transcripts, joins, observation)
    validate_audit_release(docs[AUDIT], docs[RELEASE], manifest)
    path = Path('porting/progress/evidence.json')
    ledger = json.loads(path.read_text())
    check = 'frontend_commonui_panel_outro_runtime'
    ledger['checks'][check] = {'id': check, 'result': 'pass', 'exit_code': 0,
        'command': 'python3 -I porting/progress/record_frontend_panel_outro_runtime.py',
        'checkpoint': CHECKPOINT, 'checkpoint_sha256': PINS[CHECKPOINT],
        'runtime_report': ROOT, 'runtime_report_sha256': PINS[ROOT],
        'integrity_audit': AUDIT, 'integrity_audit_sha256': PINS[AUDIT],
        'scope': 'Controlled full Panel intro, outro, reacquisition and listener lifecycle with held-action map, NaN duration, callback manager relookup and structural interface reset fixtures and C++ ProcessEvent interception; two builds, seven processes and two joins. Blueprint VM, full interface routing, native input producer, queue, main button, authentic UI and parity remain unverified. No new exhaustive original metadata, link, material or pixel checks.',
        'provenance_note': 'Pinned external receipts and independently audited artifacts must remain available for re-verification.'}
    path.write_text(json.dumps(ledger, indent=1) + '\n')


if __name__ == '__main__':
    main()
