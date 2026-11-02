#!/usr/bin/env python3
"""Rebuild the compact audit inventory from HEAD and the working tree; no dashboard evaluation."""
import hashlib
import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
OUT = Path(__file__).resolve().parent
MODEL = 'porting/progress/components.json'
EVIDENCE = 'porting/progress/evidence.json'

def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT)

def sha(data):
    return hashlib.sha256(data).hexdigest()

def snapshot(model_bytes, evidence_bytes):
    model = json.loads(model_bytes)
    evidence = json.loads(evidence_bytes)['checks']
    leaves, categories = [], []
    def visit(node, category):
        if 'children' in node:
            for child in node['children']:
                visit(child, category)
            return
        m = node.get('measure', {})
        checks = m.get('checks', [])
        pointers = {}
        for key, value in m.items():
            if key != 'checks' and (key in ('file', 'count_file', 'roots', 'total_key', 'total_sub', 'count_key') or 'path' in key):
                pointers[key] = value
        if m.get('kind') == 'ledger':
            pointers['ledger_check_ids'] = checks
            pointers['ledger_file'] = EVIDENCE
            pointers['missing_ledger_ids'] = [c for c in checks if c not in evidence]
        elif checks:
            pointers['check_sources'] = [{k: v for k, v in c.items() if k in ('file', 'local', 'glob', 'label')} if isinstance(c, dict) else c for c in checks]
        leaves.append({'id': node['id'], 'category_id': category['id'], 'category': category['name'], 'name': node['name'], 'weight': node['weight'], 'measure_kind': m.get('kind'), 'check_count': len(checks), 'rationale': node.get('rationale', ''), 'evidence_pointers': pointers})
    for category in model['tree']:
        start = len(leaves)
        visit(category, category)
        categories.append({'id': category['id'], 'name': category['name'], 'leaf_count': len(leaves)-start, 'weight': sum(x['weight'] for x in leaves[start:])})
    assert len(leaves) == len({x['id'] for x in leaves}), 'duplicate leaf ids'
    assert all(isinstance(x['weight'], (int, float)) and x['weight'] >= 0 and x['measure_kind'] for x in leaves)
    return {'model_sha256': sha(model_bytes), 'evidence_sha256': sha(evidence_bytes), 'leaf_count': len(leaves), 'total_weight': sum(x['weight'] for x in leaves), 'categories': categories, 'leaves': leaves}

def main():
    head = git('rev-parse', 'HEAD').decode().strip()
    committed = snapshot(git('show', f'HEAD:{MODEL}'), git('show', f'HEAD:{EVIDENCE}'))
    working = snapshot((ROOT / MODEL).read_bytes(), (ROOT / EVIDENCE).read_bytes())
    before = {x['id']: x for x in committed['leaves']}
    after = {x['id']: x for x in working['leaves']}
    changes = [{'id': key, 'before_weight': before[key]['weight'] if key in before else None, 'after_weight': after[key]['weight'] if key in after else None} for key in sorted(before.keys() | after.keys()) if key not in before or key not in after or before[key]['weight'] != after[key]['weight']]
    result = {'source_head': head, 'source_paths': {'model': MODEL, 'evidence': EVIDENCE}, 'working_tree_status': git('status', '--short', '--', MODEL, EVIDENCE).decode().splitlines(), 'meaning': 'Weights estimate total completed plus remaining effort, not a progress result. Working-tree changes are proposals, not accepted calibration.', 'committed_head': committed, 'working_tree': working, 'weight_changes': changes}
    (OUT / 'inventory.json').write_text(json.dumps(result, indent=2, ensure_ascii=False) + '\n')
    print(f"HEAD {head}: {committed['leaf_count']} leaves, {committed['total_weight']} points; working tree: {working['leaf_count']} leaves, {working['total_weight']} points; {len(changes)} weight changes")

if __name__ == '__main__':
    main()
