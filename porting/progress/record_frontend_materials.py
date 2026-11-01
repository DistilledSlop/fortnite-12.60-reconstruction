#!/usr/bin/env python3
"""Record verified frontend material owner probes; never run the client.

Run from the repository root. Missing, failed and incomplete probes remain pending.
"""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

EXCLUDED = {'Athena_LockerTabMaster', 'M_UI_GradientPanel', 'M-Border-PowerToastGlow',
            'M_MenuButton_Master', 'M_UI_DonutIcons'}
EVIDENCE = Path('porting/progress/evidence.json')
COMPONENTS = Path('porting/progress/components.json')
VERIFIER = Path('porting/material_pipeline/verify_runtime_artifacts.py')


def digest(path):
    with Path(path).open('rb') as source:
        return hashlib.file_digest(source, 'sha256').hexdigest()


def git_commit(repo, revision):
    return subprocess.check_output(['git', '-C', str(repo), 'rev-parse', '--verify', revision + '^{commit}'], text=True).strip()


def owner_id(owner):
    return 'material_frontend_owner_' + hashlib.sha256(owner.encode()).hexdigest()[:16]


def attachment_proof(line, owner, resource, provenance_owners):
    """Accept pinned old receipts and the newer explicit shared-map receipt."""
    prefix = 'Legacy material ingress: manifest owner ' + owner + ' uses original resource owner ' + resource
    success = 'complete and attached on both threads; seven shaders and all uniform cases passed'
    if prefix + ', ' + success in line:
        return line.endswith(prefix + ', ' + success)
    marker = prefix + ', shared map provenance owner '
    if marker not in line:
        return False
    provenance, separator, result = line.split(marker, 1)[1].partition('; ')
    return bool(separator and provenance in provenance_owners and result == success)


def inventory_owners(path):
    owners = []
    for entry in json.loads(path.read_text())['entries']:
        prefix = 'FortniteGame/Content/'
        if not entry['package'].startswith(prefix):
            raise ValueError('unexpected inventory package root')
        for material in entry['material_reports']:
            owners.append('/Game/' + entry['package'][len(prefix):] + '.' + material['name'])
    if len(owners) != 147 or len(set(owners)) != 147:
        raise ValueError('inventory must contain 147 distinct material exports')
    scoped = [o for o in owners if o.rsplit('.', 1)[1] not in EXCLUDED]
    if len(scoped) != 142 or {o.rsplit('.', 1)[1] for o in owners} & EXCLUDED != EXCLUDED:
        raise ValueError('expected five separately weighted owners and 142 scoped owners')
    return scoped


def report_for(bundle, artifacts):
    # The verifier writes its report: reserve a fresh name and never overwrite old proof.
    with tempfile.NamedTemporaryFile(prefix=artifacts.name + '-dashboard-validation-',
                                     suffix='.json', dir=artifacts.parent, delete=False) as f:
        report = Path(f.name)
    proc = subprocess.run([sys.executable, '-I', str(VERIFIER), '--fixture', str(bundle),
                           '--artifacts', str(artifacts), '--report', str(report)],
                          capture_output=True, text=True)
    if proc.returncode:
        print(f'verification failed for {artifacts}: {proc.stderr.strip()}', file=sys.stderr)
        return None
    return report


def verify_frozen_inputs(inputs_path, manifest_path, manifest_sha256, native_commit, engine_commit):
    inputs = json.loads(inputs_path.read_text())
    for item in inputs:
        path = Path(item['path']).resolve()
        # Native code may have changed since the run; verify committed bytes instead.
        native_root = (Path.cwd() / 'porting/native').resolve()
        engine_root = Path('/opt/fortnite/UnrealEngine').resolve()
        if path.is_relative_to(native_root):
            content = subprocess.check_output(['git', 'show', native_commit + ':' +
                                               str(path.relative_to(Path.cwd().resolve()))])
            actual = hashlib.sha256(content).hexdigest()
        elif path.is_relative_to(engine_root):
            content = subprocess.check_output(['git', '-C', str(engine_root), 'show',
                                               engine_commit + ':' + str(path.relative_to(engine_root))])
            actual = hashlib.sha256(content).hexdigest()
        else:
            actual = digest(path)
        if actual != item['sha256']:
            raise ValueError('frozen build input changed: ' + str(path))
    if not any(Path(i['path']).resolve() == manifest_path.resolve() and i['sha256'] == manifest_sha256 for i in inputs):
        raise ValueError('manifest is not bound to frozen inputs')


def verify_checkpoint(path, expected_sha, receipt, manifest, manifest_sha, native_commit, engine_commit):
    if digest(path) != expected_sha:
        raise ValueError('checkpoint SHA-256 mismatch')
    checkpoint = json.loads(path.read_text())
    if checkpoint.get('schema') != 'fortnite.material-runtime-checkpoint.v1':
        raise ValueError('unsupported runtime checkpoint schema')
    if (git_commit(Path.cwd(), checkpoint['native_commit']) != native_commit or
        git_commit(Path('/opt/fortnite/UnrealEngine'), checkpoint['engine_commit']) != engine_commit):
        raise ValueError('checkpoint source revisions do not match')
    if (Path(checkpoint['manifest']['path']).resolve() != manifest.resolve() or
        checkpoint['manifest']['sha256'] != manifest_sha or checkpoint['manifest_sha256'] != manifest_sha):
        raise ValueError('checkpoint manifest does not match')
    for key in ('build_receipt', 'owner_run_receipt'):
        ref = checkpoint[key]
        if digest(ref['path']) != ref['sha256']:
            raise ValueError(key + ' hash mismatch')
    if Path(checkpoint['owner_run_receipt']['path']).resolve() != receipt.resolve():
        raise ValueError('checkpoint owner receipt does not match')
    runs = json.loads(receipt.read_text())
    if checkpoint['probes'] != runs:
        raise ValueError('embedded probes do not match owner receipt')
    build = json.loads(Path(checkpoint['build_receipt']['path']).read_text())
    if build.get('exit_status') != 0 or build not in checkpoint['builds']:
        raise ValueError('client build did not pass or is not bound to checkpoint')
    for item in checkpoint['source_files']:
        repo = Path(item['repository']).resolve()
        if repo == Path.cwd().resolve():
            expected_commit = native_commit
        elif repo == Path('/opt/fortnite/UnrealEngine').resolve():
            expected_commit = engine_commit
        else:
            raise ValueError('unexpected source repository')
        if git_commit(repo, item['revision']) != expected_commit:
            raise ValueError('source revision mismatch')
        source = subprocess.check_output(['git', '-C', str(repo), 'show', expected_commit + ':' + item['path']])
        if hashlib.sha256(source).hexdigest() != item['sha256'] or digest(item['snapshot']) != item['sha256']:
            raise ValueError('source blob or snapshot mismatch')
    executable_sha = None
    for kind in ('executable', 'symbols', 'debug'):
        binaries = [b for b in checkpoint['binaries'] if b.get('target') == 'FortniteLinux'
                    and b.get('artifact_kind') == kind]
        if len(binaries) != 3 or {b['role'] for b in binaries} != {'source', 'stage', 'snapshot'}:
            raise ValueError('missing ' + kind + ' source, stage or snapshot')
        if len({(b['sha256'], b['size']) for b in binaries}) != 1:
            raise ValueError('source and stage ' + kind + ' hashes differ')
        snapshot = next(b for b in binaries if b['role'] == 'snapshot')
        if Path(snapshot['path']).stat().st_size != snapshot['size'] or digest(snapshot['path']) != snapshot['sha256']:
            raise ValueError('frozen ' + kind + ' snapshot mismatch')
        if kind == 'executable':
            executable_sha = snapshot['sha256']
    return checkpoint, executable_sha


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--receipt', type=Path, required=True)
    parser.add_argument('--manifest', type=Path, required=True)
    parser.add_argument('--inventory', type=Path, required=True)
    parser.add_argument('--manifest-sha256', required=True)
    parser.add_argument('--inputs', type=Path, required=True)
    parser.add_argument('--source-native-commit', required=True)
    parser.add_argument('--engine-commit', required=True)
    parser.add_argument('--checkpoint', type=Path, required=True)
    parser.add_argument('--checkpoint-sha256', required=True)
    parser.add_argument('--mainl', action='store_true', help='independently verify and credit the existing MainL checkpoint')
    args = parser.parse_args()
    if not re.fullmatch(r'[0-9a-f]{64}', args.manifest_sha256) or digest(args.manifest) != args.manifest_sha256:
        raise ValueError('manifest does not match requested SHA-256')
    native_commit = git_commit(Path.cwd(), args.source_native_commit)
    engine_commit = git_commit(Path('/opt/fortnite/UnrealEngine'), args.engine_commit)
    verify_frozen_inputs(args.inputs, args.manifest, args.manifest_sha256, native_commit, engine_commit)
    checkpoint, binary_sha = verify_checkpoint(args.checkpoint, args.checkpoint_sha256, args.receipt, args.manifest, args.manifest_sha256, native_commit, engine_commit)
    artifact_hashes = {str(Path(a['path']).resolve()): a['sha256'] for a in checkpoint['artifacts']}
    source_hashes = {str((Path(x['repository']) / x['path']).resolve()): x['sha256'] for x in checkpoint['source_files']}
    for item in json.loads(args.inputs.read_text()):
        path = Path(item['path']).resolve()
        if path.is_relative_to((Path.cwd() / 'porting/native').resolve()) and source_hashes.get(str(path)) != item['sha256']:
            raise ValueError('native build input is not bound to checkpoint source blobs')
    owners = inventory_owners(args.inventory)
    model = json.loads(COMPONENTS.read_text())
    def leaves(nodes):
        for node in nodes:
            if 'children' in node:
                yield from leaves(node['children'])
            else:
                yield node
    closure = next(n for n in leaves(model['tree']) if n['id'] == 'ui.material_frontend_closure')
    if closure['measure']['checks'] != [owner_id(o) for o in owners]:
        raise ValueError('component closure does not match inventory')
    manifest = {entry['owner_path']: entry for entry in json.loads(args.manifest.read_text())['entries']}
    evidence = json.loads(EVIDENCE.read_text())
    checks = evidence['checks']
    for owner in owners:
        ident = owner_id(owner)
        checks.setdefault(ident, {'id': ident, 'owner_path': owner, 'result': 'pending',
                                  'scope': 'Material owner in the bounded frontend closure; actual-owner runtime ingress has not been credited.'})
    if args.mainl:
        owner = next(o for o in owners if o.endswith('.M-Border-MainL'))
        bundle = Path('porting/generated/material_pipeline/mainl-ingress-bundle-checked.json')
        artifacts = Path('porting/generated/frontend-probes/legacy-mainl-artifacts-1')
        report = report_for(bundle, artifacts)
        if report is None:
            raise ValueError('MainL independent verification failed')
        mainl = checks['material_mainl_asset_ingress'].copy()
        mainl.update(id=owner_id(owner), owner_path=owner, artifact_validation_receipt=str(report),
                     artifact_validation_sha256=digest(report))
        checks[owner_id(owner)] = mainl
    passed = []
    for run in json.loads(args.receipt.read_text()):
        owner = run['owner']
        entry = manifest.get(owner)
        if run['exit_status'] != 0 or entry is None or owner not in owners:
            continue
        bundle, log, artifacts = Path(entry['bundle_path']), Path(run['log']), Path(run['artifact_dir'])
        if digest(bundle) != entry['bundle_sha256']:
            continue
        command = run['command']
        if (f'-FortniteLegacyMaterialManifestSHA256={args.manifest_sha256}' not in command or
            f'-FortniteLegacyMaterialManifest={args.manifest.resolve()}' not in command or
            f'-FortniteLegacyMaterialProbeOwner={owner}' not in command or
            f'-FortniteLegacyMaterialArtifactDir={artifacts}' not in command):
            continue
        if artifact_hashes.get(str(log.resolve())) != digest(log):
            continue
        payloads = list(artifacts.glob('*.spv')) + list(artifacts.glob('*.ue-vulkan'))
        if len(payloads) != 14 or any(artifact_hashes.get(str(p.resolve())) != digest(p) for p in payloads):
            continue
        lines = log.read_text(errors='replace').splitlines()
        resource = entry['resource_owner_path']
        complete = [l for l in lines if 'Legacy actual-owner completeness owner=' + resource + ' ' in l
                    and 'required=7 missing=0 complete=1' in l]
        # Shared maps may legitimately retain another resource's provenance.
        # Require that resource to have the same pinned raw bytes and shader
        # identity; the frozen native reader additionally verifies ordered names,
        # per-owner receipts, canonical map identity and both-thread attachment.
        provenance_owners = {resource} | {
            e['resource_owner_path'] for e in manifest.values()
            if e.get('resource_sha256') and e['resource_sha256'] == entry.get('resource_sha256')
            and e['shader_identity'] == entry['shader_identity']}
        attach = [l for l in lines if attachment_proof(l, owner, resource, provenance_owners)]
        uniform = [l for l in lines if re.search(r'\b[1-9][0-9]* manifest owner live uniform cases passed', l)]
        fixture = json.loads(bundle.read_text())['runtime_fixture']
        view = [l for l in lines if re.search(r'\b[1-9][0-9]* manifest View draw live uniform cases passed', l)]
        expected_uniform = len(fixture.get('owner_uniform_cases', []))
        expected_view = len(fixture.get('view_cases', []))
        if (not complete or not attach or not uniform or not expected_uniform or
            not re.search(r'\b' + str(expected_uniform) + r' manifest owner live uniform cases passed', uniform[-1]) or
            (expected_view and (not view or not re.search(r'\b' + str(expected_view) + r' manifest View draw live uniform cases passed', view[-1])))):
            continue
        ident = owner_id(owner)
        previous = checks.get(ident, {})
        old_report = Path(previous.get('artifact_validation_receipt', '/nonexistent'))
        if (previous.get('result') == 'pass' and previous.get('log_sha256') == digest(log)
            and previous.get('bundle_sha256') == digest(bundle) and old_report.is_file()
            and previous.get('artifact_validation_sha256') == digest(old_report)
            and previous.get('runtime_checkpoint_sha256') == args.checkpoint_sha256
            and previous.get('source_native_commit') == native_commit
            and previous.get('engine_commit') == engine_commit
            and previous.get('run_receipt_sha256') == digest(args.receipt)
            and previous.get('source_and_staged_client_sha256') == binary_sha):
            continue
        report = report_for(bundle, artifacts)
        if report is None:
            continue
        checks[ident] = {'id': ident, 'result': 'pass', 'exit_code': 0, 'owner_path': owner,
                         'resource_owner_path': resource, 'command': command, 'log': str(log),
                         'log_sha256': digest(log), 'key_lines': [complete[-1], uniform[-1], attach[-1]] + view[-1:],
                         'manifest': str(args.manifest), 'manifest_sha256': args.manifest_sha256,
                         'bundle': str(bundle), 'bundle_sha256': digest(bundle), 'run_receipt': str(args.receipt),
                         'artifact_validation_receipt': str(report), 'artifact_validation_sha256': digest(report),
                         'source_frozen_inputs': str(args.inputs), 'source_frozen_inputs_sha256': digest(args.inputs),
                         'source_native_commit': native_commit, 'engine_commit': engine_commit,
                         'commit': git_commit(Path.cwd(), 'HEAD'),
                         'utc': datetime.now(timezone.utc).isoformat(timespec='seconds').replace('+00:00', 'Z'),
                         'run_receipt_sha256': digest(args.receipt),
                         'runtime_checkpoint': str(args.checkpoint), 'runtime_checkpoint_sha256': args.checkpoint_sha256,
                         'build_receipt': checkpoint['build_receipt']['path'],
                         'build_receipt_sha256': checkpoint['build_receipt']['sha256'],
                         'source_and_staged_client_sha256': binary_sha,
                         'scope': 'This owner original material map compatibility and attachment in the bounded frontend closure; widget integration and original visual parity remain unverified.'}
        passed.append(owner)
    EVIDENCE.write_text(json.dumps(evidence, indent=1, sort_keys=True) + '\n')
    print(f'Added {len(passed)} passing batch owners; {sum(checks[owner_id(o)].get("result") == "pass" for o in owners)}/{len(owners)} closure owners pass.')


if __name__ == '__main__':
    main()
