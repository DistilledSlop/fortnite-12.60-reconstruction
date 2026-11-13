#!/usr/bin/env python3
"""Inventory recovered native network declarations, not observed interactions."""
import argparse
from collections import Counter, defaultdict
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]
PORT = ROOT / 'porting'
SOURCES = {
    'reflection': 'generated/reflection/fortnite_game_reflection.json',
    'class_index': 'generated/reflection/native_class_index.json',
    'parents': 'generated/reflection/class_parent_map.json',
    'execs': 'generated/reflection/native_funcs.json',
    'conditions': 'replication_conditions.json',
    'manifest': 'behavior/MANIFEST.json',
    'splice': 'generated/reflection/behavior_splice.json',
}


def load_sources():
    data, provenance = {}, {}
    for key, relative in SOURCES.items():
        path = PORT / relative
        if not path.exists() and key == 'splice':
            data[key] = None
            provenance[key] = {'path': str(path.relative_to(ROOT)), 'sha256': None}
            continue
        raw = path.read_bytes()
        provenance[key] = {'path': str(path.relative_to(ROOT)), 'sha256': hashlib.sha256(raw).hexdigest()}
        data[key] = json.loads(raw)
    return data, provenance


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=PORT / 'generated/agents/network-contract-inventory', help='output directory')
    args = parser.parse_args()
    d, sources = load_sources()
    blocks = defaultdict(list)
    for name, entry in d['class_index'].items():
        if entry.get('block_va'):
            blocks[entry['block_va']].append(name)
    names_to_blocks = defaultdict(set)
    for va, names in blocks.items():
        for name in names:
            names_to_blocks[name].add(va)
    manifest = defaultdict(list)
    for entry in d['manifest']:
        manifest[(entry.get('class'), entry.get('function'))].append({k: entry.get(k) for k in ('status', 'kind', 'inc_path', 'address')})
    splice = d['splice']
    accepted = set(splice.get('accepted', [])) if splice else set()
    rejected = set(splice.get('rejected_names', [])) if splice and isinstance(splice.get('rejected_names'), list) else set()
    entries = defaultdict(list)
    unresolved, ambiguous, name_disagreements, list_disagreements = [], [], [], []
    matched_conditions = set()
    for cls in d['reflection']['classes_in_groups_with_fort_names']:
        va = cls['block_va']
        candidates = sorted(set(blocks.get(va, [])))
        owner = candidates[0] if len(candidates) == 1 else None
        parent = d['parents'].get(va)
        meta = {'block_va': va, 'declared_owner': owner, 'owner_candidates': candidates,
                'reflection_name': cls.get('name'), 'parent_metadata': parent}
        props = [p for p in cls.get('properties', []) if p.get('replicated')]
        funcs = [f for f in cls.get('functions', []) if f.get('replicated_function')]
        for label, actual, field in [('replicated_properties', props, 'name'), ('replicated_functions', funcs, 'name')]:
            listed = Counter(cls.get(label, []))
            observed = Counter(v.get(field) for v in actual)
            if listed != observed:
                list_disagreements.append({'block_va': va, 'list': label, 'listed': dict(listed), 'records': dict(observed)})
        if not owner:
            (ambiguous if candidates else unresolved).append({'block_va': va, 'candidates': candidates, 'parent_name': parent.get('name') if parent else None, 'replicated_fields': len(props), 'rpc_functions': len(funcs)})
        if owner and parent and parent.get('name') and parent['name'] != owner:
            name_disagreements.append({'block_va': va, 'index_name': owner, 'parent_name': parent['name']})
        # Use a block identity for unresolved/ambiguous owners, never conflate them.
        owner_key = owner if owner and len(names_to_blocks[owner]) == 1 else '@' + va
        for p in props:
            name = p.get('name')
            condition = None
            if owner and len(names_to_blocks[owner]) == 1:
                ce = d['conditions']['classes'].get(owner, {})
                # A same-named class from another block is not a valid join.
                if ce.get('block_va') == va and name in ce.get('properties', {}):
                    condition = ce['properties'][name]
                    matched_conditions.add((owner, name))
            record = dict(meta, kind='replicated_field', name=name, type=p.get('kind'),
                          type_refs=p.get('type_refs'), array_dim=p.get('array_dim'), rep_notify=p.get('rep_notify'),
                          original_offset=p.get('offset'), original_recovered_condition=condition,
                          condition_status='recovered' if condition is not None else 'unknown', reflection=p)
            entries[('replicated_field', owner_key, name)].append(record)
        for f in funcs:
            name = f.get('name')
            native = d['execs'].get(owner, {}).get(name) if owner and len(names_to_blocks[owner]) == 1 else None
            # The name join is surfaced as evidence, not proof of runtime behavior.
            evidence = {suffix or 'exact': manifest.get((owner, name + suffix), []) for suffix in ('', '_Implementation', '_Validate')} if owner and name else {}
            qualified = f'{owner}::{name}'
            record = dict(meta, kind='rpc', name=name, raw_net_roles=f.get('net_roles'), parameters=f.get('params'),
                          original_exec=native.get('exec_addr') if native else None,
                          exec_join_status='name_join' if native else 'unknown', reflection=f,
                          current_body_evidence={'manifest_entries': evidence, 'exact_name_in_splice_accepted': True if qualified in accepted else None,
                                                 'implementation_name_in_splice_accepted': True if qualified + '_Implementation' in accepted else None,
                                                 'exact_name_in_splice_rejected': True if qualified in rejected else None,
                                                 'runtime_verified': None})
            entries[('rpc', owner_key, name)].append(record)
    rows, collisions = [], []
    for key, variants in sorted(entries.items(), key=lambda item: tuple(str(x) for x in item[0])):
        unique = {json.dumps(v, sort_keys=True, separators=(',', ':')): v for v in variants}
        values = [unique[k] for k in sorted(unique)]
        row = {'kind': key[0], 'owner_key': key[1], 'name': key[2], 'declaration_count': len(variants), 'variants': values}
        rows.append(row)
        if len(variants) > 1:
            collisions.append({'kind': key[0], 'owner_key': key[1], 'name': key[2], 'declaration_count': len(variants), 'distinct_variants': len(values)})
    all_conditions = {(c, p) for c, v in d['conditions']['classes'].items() for p in v.get('properties', {})}
    unmatched_conditions = [{'class': c, 'property': p, 'metadata': d['conditions']['classes'][c]['properties'][p]} for c, p in sorted(all_conditions - matched_conditions)]
    counts = Counter(r['kind'] for r in rows)
    missing = [{'owner_key': r['owner_key'], 'name': r['name'], 'block_va': v['block_va']} for r in rows if r['kind'] == 'replicated_field' for v in r['variants'] if v['condition_status'] == 'unknown']
    role_counts = Counter(json.dumps(v['raw_net_roles'], sort_keys=True) for r in rows if r['kind'] == 'rpc' for v in r['variants'])
    head = subprocess.run(['git', 'rev-parse', 'HEAD'], cwd=ROOT, check=True, text=True, capture_output=True).stdout.strip()
    summary = {'schema_version': 1, 'git_head': head, 'sources': sources,
               'domain': 'Recovered native reflection class blocks in classes_in_groups_with_fort_names, including engine and other modules; declared replicated properties and functions flagged replicated_function. Not a full expected interaction list.',
               'exclusions_and_unknowns': ['Blueprint declarations and behavior, unrecovered native metadata, gameplay, engine behavior and backend are not complete.', 'Runtime exercise, sequence, ownership, relevancy, serialization and semantic parity are unknown.', 'Raw roles are preserved without inferring direction for entries with no role.', 'Manifest and splice information are source/generation evidence only, never runtime verification.', 'Null offset, exec or condition means unknown, not a default.', 'No inherited members are expanded into descendants.'],
               'counts': {'class_blocks': len(d['reflection']['classes_in_groups_with_fort_names']), 'declarations_by_kind': dict(sorted(counts.items())), 'input_declaration_occurrences': sum(r['declaration_count'] for r in rows), 'colliding_keys': len(collisions), 'unresolved_class_blocks': len(unresolved), 'ambiguous_class_blocks': len(ambiguous), 'class_name_disagreements': len(name_disagreements), 'reflection_list_disagreements': len(list_disagreements), 'missing_condition_variants': len(missing), 'unmatched_condition_entries': len(unmatched_conditions), 'rpc_variants_by_raw_roles': dict(sorted(role_counts.items()))}}
    args.output.mkdir(parents=True, exist_ok=True)
    for name, obj in [('summary.json', summary), ('issues.json', {'unresolved_class_blocks': unresolved, 'ambiguous_class_blocks': ambiguous, 'class_name_disagreements': name_disagreements, 'reflection_list_disagreements': list_disagreements, 'collisions': collisions, 'missing_conditions': missing, 'unmatched_conditions': unmatched_conditions})]:
        (args.output / name).write_text(json.dumps(obj, indent=2, sort_keys=True) + '\n')
    with (args.output / 'declarations.jsonl').open('w') as out:
        for row in rows:
            out.write(json.dumps(row, sort_keys=True, separators=(',', ':')) + '\n')
    print(json.dumps(summary['counts'], sort_keys=True))


if __name__ == '__main__':
    main()
