#!/usr/bin/env python3
"""Import the pinned, bounded confirmation-description runtime evidence."""
import hashlib
import importlib.util
import json
from pathlib import Path

CHECKPOINT = 'porting/generated/frontend-native-evidence/confirmation-description-runtime-1/checkpoint.json'
RELEASE = '/mnt/vibrato/fortnite-evidence/frozen-runtime/confirmation-description-runtime-1/source-release.json'
CHECKPOINT_SHA = '6be18260996d52ffd1f2fcf43884048a5934dc74d3392d5a195028aa48df5594'
RELEASE_SHA = 'c7a9541c6ff5b9c7bb313f55d2813349ca1bc8cba0d3ff7c8fac72a20f86ed74'
SOURCE = '00e127967abad8711a089052ae6eac71c9ff08a3'
ENGINE = 'd7e04d4b65fb8ee464082849414cf5f7ae4d7bca'
CASES = {'confirmation-description', 'panel-outro', 'panel-intro', 'main-button', 'button', 'activation', 'registry', 'lifecycle'}
BASE = '/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/generated/frontend-native-evidence/confirmation-description-runtime-1/'
BUILD = '/mnt/vibrato/fortnite-work/confirmation-description-native-build-1/'


def require(ok, message):
    if not ok:
        raise ValueError(message)


def read_pinned(path, digest):
    data = Path(path).read_bytes()
    require(hashlib.sha256(data).hexdigest() == digest, 'hash mismatch: ' + str(path))
    return data


def validate(c, release):
    require(c['schema'] == 'fortnite.confirmation-description-runtime-checkpoint.v1' and c['native_commit'] == SOURCE and c['engine_commit'] == ENGINE and c['native_source_files'] == 12701 and c['native_source_tree_unchanged'] is True, 'checkpoint')
    require(release['schema'] == 'fortnite.source-release.v1' and release['native_commit'] == SOURCE and release['checkpoint_sha256'] == CHECKPOINT_SHA and release['all_build_stage_probe_join_handles_terminal'] is True and release['active_native_build_or_stage_owner'] is None, 'release')
    require(len(c['source_files']) == 15 and len({x['path'] for x in c['source_files']}) == 15, 'source inventory')
    for row in c['source_files']:
        read_pinned(row['path'], row['sha256'])
        read_pinned(row['archive'], row['sha256'])
    artifacts = {x['path']: x for x in c['artifacts']}
    require(len(artifacts) == len(c['artifacts']) == 185, 'artifact inventory')
    for row in artifacts.values():
        read_pinned(row['archive'], row['sha256'])
    def artifact(path):
        return read_pinned(artifacts[path]['archive'], artifacts[path]['sha256'])
    def doc(path):
        return json.loads(artifact(path))
    builds = doc(BUILD + 'results.json')
    require(builds == c['builds'] and len(builds) == 2 and {b['target'] for b in builds} == {'FortniteLinux', 'FortniteLinuxServer'}, 'build inventory')
    for b in builds:
        require(b['source_head'] == SOURCE and b['exit'] == 0 and b['source_tree_unchanged'] is True and b['end_ns'] > b['start_ns'] and b['elapsed_seconds'] > 0 and artifacts[b['log']]['sha256'] == b['log_sha256'], 'build receipt')
    stage = doc(BUILD + 'stage.json')
    rows = stage['rows']
    require(stage['build_source'] == SOURCE and rows == c['binary_rows'] and len(rows) == 18, 'stage inventory')
    for target in ('FortniteLinux', 'FortniteLinuxServer'):
        b = next(x for x in builds if x['target'] == target)
        for suffix in ('', '.sym', '.debug'):
            group = [r for r in rows if r['target'] == target and Path(r['path']).name == target + suffix]
            require(len(group) == 3 and {r['role'] for r in group} == {'source', 'archive', 'stage'} and len({(r['size'], r['sha256']) for r in group}) == 1 and group[0]['size'] > 0 and len(group[0]['sha256']) == 64, 'stage binary fields')
            if not suffix:
                require((group[0]['size'], group[0]['sha256']) == (b['binary']['size'], b['binary']['sha256']), 'build binary fields')
    linux = next(b for b in builds if b['target'] == 'FortniteLinux')
    stage_binary = next(r for r in rows if r['target'] == 'FortniteLinux' and r['role'] == 'stage' and Path(r['path']).name == 'FortniteLinux')
    launches = c['retained_diagnostics']
    require(launches == doc(BASE + 'retained-native-1/launch-results.json') and len(launches) == 8 and {x['case'] for x in launches} == CASES and all(x['exit'] == 0 for x in launches), 'launches')
    pids = set()
    for case in CASES:
        prefix = BASE + 'retained-native-1/' + case + '/'
        p, t = doc(prefix + 'receipt.json'), doc(prefix + 'retained-transcript-check.json')
        require(p['schema'] == 'button-diagnostic-run-v1' and p['case'] == case and p['passed'] is True and p['returncode'] == 0 and p['timed_out'] is False and p['proc_exe_inode_verified'] is True and p['binary_after_verified'] is True and p['error'] is None and p['start_ns'] > max(b['end_ns'] for b in builds) and p['end_ns'] > p['start_ns'], 'process: ' + case)
        require(p['build'] == linux and p['build_results']['sha256'] == artifacts[BUILD + 'results.json']['sha256'] and p['binary'] == stage_binary['path'] and p['argv'] == p['proc_argv'] and p['argv'][0] == p['binary'] and p['cwd'] == p['proc_cwd'], 'process identity: ' + case)
        require(t['case'] == case and t['passed'] is True and artifacts[p['log']['path']]['sha256'] == p['log']['sha256'], 'transcript receipt: ' + case)
        pids.add(p['pid'])
        if case == 'confirmation-description':
            require('-ConfirmationDescriptionProbe' in p['argv'], 'diagnostic argument')
            # Load the pinned verifier only after both source copies were hashed.
            verifier_path = next(x['archive'] for x in c['source_files'] if x['path'].endswith('/confirmation_description_verifier.py'))
            spec = importlib.util.spec_from_file_location('confirmation_description_verifier', verifier_path)
            verifier = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(verifier)
            result = verifier.verify(artifact(p['log']['path']).decode('utf-8', errors='replace'))
            require(result == p['transcript'] == t['detail'] == doc(prefix + 'verification.json') == c['confirmation_description'] and result['rows'] == 13, 'diagnostic transcript')
    require(len(pids) == 8, 'distinct diagnostic processes')
    root = c['root_probe']
    require(root == doc('/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/generated/frontend-probes/confirmation-description-root-1/result.json') and root['exit'] == 0 and root['timed_out'] is False and root['proc_exe_inode_verified'] is True and root['binary_after_verified'] is True and root['binary_before'] == stage_binary and root['stage_receipt_sha256'] == artifacts[BUILD + 'stage.json']['sha256'] and root['argv'] == root['proc_argv'] and root['end_ns'] > root['start_ns'] > max(b['end_ns'] for b in builds), 'root process')
    stdout = artifact('/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/generated/frontend-probes/confirmation-description-root-1/stdout.log')
    require(hashlib.sha256(stdout).hexdigest() == root['stdout_sha256'] and b'RootLayout_Athena.RootLayout_Athena_C -> WidgetBlueprintGeneratedClass' in stdout and not any(b'bytecode' in line.lower() and b'warning' in line.lower() for line in stdout.splitlines()), 'root log')
    joins = c['joins']
    require(joins == doc(BASE + 'joins.json') and len(joins) == 2 and {x['case'] for x in joins} == {'default', 'athena'} and all(j['exit'] == 0 and j['end_ns'] > j['start_ns'] > max(b['end_ns'] for b in builds) and j['elapsed_seconds'] > 0 for j in joins), 'joins')
    for join in joins:
        require(set(join['logs']) == {'result.txt', 'server.log', 'client.log'}, 'join log inventory')
        for log in join['logs'].values():
            read_pinned(log['path'], log['sha256'])


def check():
    c = json.loads(read_pinned(CHECKPOINT, CHECKPOINT_SHA))
    release = json.loads(read_pinned(RELEASE, RELEASE_SHA))
    validate(c, release)


def main():
    check()
    path = Path('porting/progress/evidence.json')
    ledger = json.loads(path.read_text())
    key = 'frontend_confirmation_description_runtime'
    ledger['checks'][key] = {'id': key, 'result': 'pass', 'exit_code': 0, 'command': 'python3 -I porting/progress/record_frontend_confirmation_description_runtime.py', 'checkpoint': CHECKPOINT, 'checkpoint_sha256': CHECKPOINT_SHA, 'release': RELEASE, 'release_sha256': RELEASE_SHA, 'scope': 'Actual UE description constructor, layout and typed delegate copy/destruction diagnostic, Root class loading, eight retained processes and two joins. No confirmation action, interactive UI, Blueprint execution or parity claim. Stage binary size and hash fields are cross-checked; binary contents are not rehashed.', 'provenance_note': 'Pinned source pairs and frozen artifacts must remain available for re-verification.'}
    path.write_text(json.dumps(ledger, indent=1) + '\n')


if __name__ == '__main__':
    main()
