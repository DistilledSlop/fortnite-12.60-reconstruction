#!/usr/bin/env python3
"""Record verified batch-7 frontend owner probes; never run the client.

Run from the repository root. Missing, failed and incomplete probes remain pending.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

MANIFEST_SHA256 = '5a77ff5ee75f769785fcb434f6713671fa37fb1f4ca7da40f93c6d4bd73eae56'
EXCLUDED = {'Athena_LockerTabMaster', 'M_UI_GradientPanel', 'M-Border-PowerToastGlow',
            'M_MenuButton_Master', 'M_UI_DonutIcons'}
EVIDENCE = Path('porting/progress/evidence.json')
COMPONENTS = Path('porting/progress/components.json')
INPUTS = Path('porting/generated/material_pipeline/batch7-build-inputs-1.json')
VERIFIER = Path('porting/material_pipeline/verify_runtime_artifacts.py')


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def owner_id(owner):
    return 'material_frontend_owner_' + hashlib.sha256(owner.encode()).hexdigest()[:16]


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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--receipt', type=Path, required=True)
    parser.add_argument('--manifest', type=Path, required=True)
    parser.add_argument('--inventory', type=Path, required=True)
    parser.add_argument('--mainl', action='store_true', help='independently verify and credit the existing MainL checkpoint')
    args = parser.parse_args()
    if digest(args.manifest) != MANIFEST_SHA256:
        raise ValueError('manifest does not match frozen batch-7 SHA-256')
    for item in json.loads(INPUTS.read_text()):
        if digest(item['path']) != item['sha256']:
            raise ValueError('frozen build input changed: ' + item['path'])
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
        if (f'-FortniteLegacyMaterialManifestSHA256={MANIFEST_SHA256}' not in command or
            f'-FortniteLegacyMaterialManifest={args.manifest.resolve()}' not in command or
            f'-FortniteLegacyMaterialProbeOwner={owner}' not in command or
            f'-FortniteLegacyMaterialArtifactDir={artifacts}' not in command):
            continue
        lines = log.read_text(errors='replace').splitlines()
        resource = entry['resource_owner_path']
        complete = [l for l in lines if 'Legacy actual-owner completeness owner=' + resource + ' ' in l
                    and 'required=7 missing=0 complete=1' in l]
        attach = [l for l in lines if 'manifest owner ' + owner + ' uses original resource owner ' + resource +
                  ', complete and attached on both threads; seven shaders and all uniform cases passed' in l]
        uniform = [l for l in lines if re.search(r'\b[1-9][0-9]* manifest owner live uniform cases passed', l)]
        fixture = json.loads(bundle.read_text())['runtime_fixture']
        view = [l for l in lines if re.search(r'\b[1-9][0-9]* manifest View draw live uniform cases passed', l)]
        if not (complete and attach and uniform) or (fixture.get('view_cases') and not view):
            continue
        ident = owner_id(owner)
        previous = checks.get(ident, {})
        old_report = Path(previous.get('artifact_validation_receipt', '/nonexistent'))
        if (previous.get('result') == 'pass' and previous.get('log_sha256') == digest(log)
            and previous.get('bundle_sha256') == digest(bundle) and old_report.is_file()
            and previous.get('artifact_validation_sha256') == digest(old_report)):
            continue
        report = report_for(bundle, artifacts)
        if report is None:
            continue
        checks[ident] = {'id': ident, 'result': 'pass', 'exit_code': 0, 'owner_path': owner,
                         'resource_owner_path': resource, 'command': command, 'log': str(log),
                         'log_sha256': digest(log), 'key_lines': [complete[-1], uniform[-1], attach[-1]] + view[-1:],
                         'manifest': str(args.manifest), 'manifest_sha256': MANIFEST_SHA256,
                         'bundle': str(bundle), 'bundle_sha256': digest(bundle), 'run_receipt': str(args.receipt),
                         'artifact_validation_receipt': str(report), 'artifact_validation_sha256': digest(report),
                         'source_frozen_inputs': str(INPUTS), 'source_frozen_inputs_sha256': digest(INPUTS),
                         'scope': 'This owner original material map compatibility and attachment in the bounded frontend closure; widget integration and original visual parity remain unverified.'}
        passed.append(owner)
    EVIDENCE.write_text(json.dumps(evidence, indent=1, sort_keys=True) + '\n')
    print(f'Added {len(passed)} passing batch owners; {sum(checks[owner_id(o)].get("result") == "pass" for o in owners)}/{len(owners)} closure owners pass.')


if __name__ == '__main__':
    main()
