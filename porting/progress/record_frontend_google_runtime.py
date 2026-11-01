#!/usr/bin/env python3
"""Import one Google material owner from the frozen phase-2 runtime receipts."""
import hashlib
import json
from pathlib import Path

CHECKPOINT = 'porting/generated/material_pipeline/frontend-google-checkpoint-results-2.json'
AUDIT = 'porting/generated/frontend-native-evidence/google-runtime-2/integrity-audit-1.json'
CONTEXT = 'porting/generated/frontend-native-evidence/google-runtime-2/root-context-validation-1.json'
DEFAULTS = 'porting/generated/frontend-probes/google-owner-probes-2/google-defaults-root-validation-1.json'
COMBINED = 'porting/generated/frontend-probes/google-owner-probes-2/google-combined-root-validation-1.json'
ROOT = 'porting/generated/material_pipeline/google-runtime-root-audit-2.json'
PINS = {
    CHECKPOINT: 'aacafe94827d5d717f5f73650d245e06904b877c7e4f6738c701e281925a4fd8',
    AUDIT: '424df964f0dec72e4966c02b5e8caa4435f4763f929337c95b636095e978c6ec',
    CONTEXT: '163bbddfd1c730b5d88bbc660e915ecde10b8d00408126578e7746ff54320d33',
    DEFAULTS: 'd4be75da4e4f0f802f5e93af0f1aaea1f152aae6429be64500065d55f2fa3603',
    COMBINED: '1658e889602ab677b4715a41ad363893cf0ff09d2b3e2c7f3aef12f429a548a0',
    ROOT: '0768befa2f3e39cf1b60546152edd4343ae6e92ee3df41522fa6a168e0d4b5d7',
}
OWNER = '/Game/UI/Login/Art/M_TempIcon_Login_Google.M_TempIcon_Login_Google'
IDENT = 'material_frontend_owner_96d352985327c1b0'
NATIVE = '5d45ea265da7f3ac617af31cc35d1ebdd8c577e7'

def require(condition, message):
    if not condition:
        raise ValueError(message)

def read_pinned(path, digest):
    data = Path(path).read_bytes()
    require(hashlib.sha256(data).hexdigest() == digest, 'hash mismatch: ' + path)
    return json.loads(data)

def validate(c, a, context, defaults, combined, root):
    require(c['schema'] == 'fortnite.material-runtime-checkpoint.v1' and c['phase'] == 'google-runtime-2' and c['phase_passed'] is True and c['intermediate'] is False, 'checkpoint phase')
    require(c['native_commit'] == NATIVE and c['engine_commit'] == 'd7e04d4b6', 'source revisions')
    require((len(c['source_files']), len(c['binaries']), len(c['artifacts'])) == (202, 18, 1693), 'binding counts')
    require(a['schema'] == 'fortnite.google-phase2-integrity-audit.v1' and a['scope_integrity_passed'] is True and a['phase_passed'] is True and a['checkpoint']['sha256'] == PINS[CHECKPOINT], 'integrity audit')
    require(a['pins']['native_commit'] == NATIVE and a['pins']['engine_commit'] == c['engine_commit'] and a['counts']['source_rows'] == 202 and a['counts']['unique_source_bindings_verified'] == 202 and a['counts']['binary_rows'] == 18 and a['counts']['artifact_rows'] == 1693, 'integrity bindings')
    release = a['source_release']
    require(release['all_phase_jobs_ended'] is True and release['source_release'] is True and release['engine_build_stage_ownership_released'] is True, 'source release')
    require(context['schema'] == 'context-live-verification-v1' and context['passed'] is True and context['build_sha256'] == c['context_build_finish']['sha256'], 'context binding')
    runs = {x['name']: x for x in c['google_profiles']}
    require(set(runs) == {'google-defaults', 'google-combined'}, 'profiles')
    for name, report in [('defaults', defaults), ('combined', combined)]:
        run = runs['google-' + name]
        require(run['exit_status'] == run['strict_exit_status'] == 0 and '-FortniteLegacyMaterialProbeOwner=' + OWNER in run['command'] and '-FortniteLegacyGoogleProfile=' + name in run['command'], 'profile run')
        require(report['passed'] is True and report['verified_shader_payloads'] == 7 and report['owner'] == {'graph_cases': 8, 'owner_cases': 8} and report['source']['mip_count'] == 10, 'strict material proof')
        sampling = report['sampling']
        require(sampling['numeric_profile'] == name and sampling['named_proxy_calls'] == 1 and sampling['checker_changed_pixels'] == 4096 and sampling['source_sha1'] == sampling['copy_sha1'] and sampling['raw_bc1_gpu_readback_measured'] is False, 'sampling proof')
    require(defaults['fixture_sha256'] == combined['fixture_sha256'] and defaults['shader_fixture_sha256'] == combined['shader_fixture_sha256'], 'profile fixture binding')
    require(root['schema'] == 'fortnite.google.root-runtime-audit.v1' and root['passed'] is True and root['checkpoint']['sha256'] == PINS[CHECKPOINT] and root['integrity_audit']['sha256'] == PINS[AUDIT] and root['context_strict_report']['sha256'] == PINS[CONTEXT], 'root audit bindings')
    reports = {row['profile']: row['sha256'] for row in root['google_strict_reports']}
    require(reports == {'defaults': PINS[DEFAULTS], 'combined': PINS[COMBINED]} and root['native_commit'] == NATIVE and root['engine_commit'] == c['engine_commit'] and root['source_release'] == release, 'root profile bindings')

def main():
    c, a, context, defaults, combined, root = (read_pinned(path, digest) for path, digest in PINS.items())
    validate(c, a, context, defaults, combined, root)
    path = Path('porting/progress/evidence.json')
    ledger = json.loads(path.read_text())
    old = ledger['checks'][IDENT]
    require(old['id'] == IDENT and old['owner_path'] == OWNER, 'existing owner identity')
    ledger['checks'][IDENT] = {'id': IDENT, 'owner_path': OWNER, 'result': 'pass', 'exit_code': 0,
        'command': 'python3 -I porting/progress/record_frontend_google_runtime.py',
        'source_native_commit': NATIVE, 'engine_commit': c['engine_commit'],
        'receipts': {path: digest for path, digest in PINS.items()},
        'scope': 'Actual Google material owner diagnostic runtime ingress, named texture binding, all-mip sampling controls and seven shader payloads. Raw GPU-compressed readback, original-client pixel parity, widget integration and interactive UI remain unverified.',
        'provenance_note': 'Pinned external receipts and their independently audited source and artifact bindings must remain available for re-verification.'}
    path.write_text(json.dumps(ledger, indent=1) + '\n')

if __name__ == '__main__':
    main()
