#!/usr/bin/env python3
"""Import three fixed, hash-pinned frontend source-stage receipts; never infer runtime."""
import hashlib
import json
from pathlib import Path

LEDGER = Path('porting/progress/evidence.json')
RECEIPTS = {
    'frontend_context_source_stage': ('porting/generated/frontend-native-evidence/native-context-runtime-root-audit-1.json', 'd52405265c36dab4812a583edf171792bf5b4b12e5fb5919223a64adf65eb147'),
    'frontend_panel_base_source_stage': ('/tmp/common-panel-native-runtime-candidate-1/frozen-base-stage-1/receipt.json', 'c727a782c26059a767f8703085d63f54fd6d9f4bda8865bf62ae0225c2ff6255'),
    'frontend_input_source_stage': ('/tmp/common-input-native-dependencies-1/frozen-global-weak-stage-7/receipt.json', '138e3dd5ffb4b7bb131cb30799d5b8c89bffa342090810dc7cd5c10a78e9c882'),
}
SCOPES = {
    'frontend_context_source_stage': 'Reviewed generic lookup/registry/base-lifecycle source and generator candidate, portable and policy tests; excludes concrete derived contexts, startup integration, fresh UE build and runtime.',
    'frontend_panel_base_source_stage': 'Isolated base activation candidate with portable/policy tests, fresh UHT and two syntax units; excludes Panel queue integration, shared build, link and runtime.',
    'frontend_input_source_stage': 'Isolated bounded input/device/lifecycle and delegate candidate, generator tests, UHT and syntax; excludes global action dispatch, held/interface completion, cursor warp, shared build and runtime.',
}

def read_receipt(path, expected):
    data = Path(path).read_bytes()
    if hashlib.sha256(data).hexdigest() != expected:
        raise ValueError('receipt hash mismatch: ' + str(path))
    return json.loads(data)

def verify_files(root, rows, field='path'):
    """Check frozen bytes, rather than treating the receipt alone as evidence."""
    checked = {}
    for row in rows:
        path = Path(root) / row[field]
        expected = row['sha256']
        if path not in checked:
            checked[path] = hashlib.sha256(path.read_bytes()).hexdigest()
        if checked[path] != expected:
            raise ValueError('frozen source hash mismatch: ' + str(path))
        if 'size' in row and path.stat().st_size != row['size']:
            raise ValueError('frozen source size mismatch: ' + str(path))
    return len(rows)

def verify_inputs(check_id, path, receipt):
    root = Path(path).parent
    if check_id == 'frontend_context_source_stage':
        candidate = receipt['candidate_receipt']
        frozen = read_receipt(candidate['path'], candidate['sha256'])
        patch = receipt['candidate_patch']
        if hashlib.sha256(Path(patch['path']).read_bytes()).hexdigest() != patch['sha256']:
            raise ValueError('candidate patch hash mismatch')
        root = Path(candidate['path']).parent
        return (verify_files(root, frozen['inputs'], 'snapshot')
                + verify_files(root, frozen['raw_windows'])
                + verify_files(root, frozen['outputs']))
    if check_id == 'frontend_panel_base_source_stage':
        return (verify_files(root, receipt['inputs'], 'snapshot')
                + verify_files(root, receipt['original_windows'], 'snapshot'))
    if check_id == 'frontend_input_source_stage':
        return verify_files(root, receipt['files'])
    raise ValueError('unknown stage')

def validate(check_id, receipt):
    if check_id == 'frontend_context_source_stage':
        assert receipt['passed'] is True and receipt['root_portable_core_test']['passed'] is True
        assert receipt['generator_policy_tests'] == 8 and receipt['source_bindings_verified'] == 30984
        assert receipt['source_review']['derived_context_behavior_claim'] is False
        assert receipt['fresh_uht'] is False and receipt['ue_build'] is False and receipt['ue_runtime'] is False
    elif check_id == 'frontend_panel_base_source_stage':
        assert receipt['stage'] == 'base activation only; Panel/queue not integrated'
        assert receipt['portable_asan_ubsan_pass'] is True and receipt['fresh_uht'] is True
        assert receipt['policy_tests'] == 7 and receipt['ue_syntax_units'] == 2
        assert receipt['ue_build'] is False and receipt['link_runtime_claim'] is False
    elif check_id == 'frontend_input_source_stage':
        assert 'Global action registration/dispatch' in receipt['scope'] and 'No UE link/build/runtime' in receipt['validation']
        assert 'fresh UHT10' in receipt['validation'] and len(receipt['files']) > 0
    else:
        raise ValueError('unknown stage')

def main():
    evidence = json.loads(LEDGER.read_text())
    for check_id, (path, sha) in RECEIPTS.items():
        receipt = read_receipt(path, sha)
        validate(check_id, receipt)
        verified_bindings = verify_inputs(check_id, path, receipt)
        evidence['checks'][check_id] = {
            'id': check_id, 'result': 'pass', 'exit_code': 0,
            'command': 'python3 -I porting/progress/record_frontend_source_stages.py',
            'verification_receipt': path, 'verification_receipt_sha256': sha,
            'frozen_bindings_verified': verified_bindings,
            'scope': SCOPES[check_id],
            'provenance_note': 'Frozen source-stage evidence; external receipt availability is required for independent re-verification.',
        }
    LEDGER.write_text(json.dumps(evidence, indent=1) + '\n')

if __name__ == '__main__':
    main()
