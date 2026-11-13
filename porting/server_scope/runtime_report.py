#!/usr/bin/env python3
"""Summarize schema-1 RPC trace evidence against recovered declarations."""
import argparse
from collections import Counter, defaultdict
import hashlib
import json
import math
from pathlib import Path
import re
import sys

PORT = Path(__file__).resolve().parents[1]
OUTCOMES = {'rpc_receive': {'invoked', 'delayed_unmapped', 'rejected_access', 'rejected_unwanted', 'invalid_function', 'decode_error', 'validation_failed'}, 'rpc_send': {'queued', 'submitted', 'channel_closing', 'bunch_error'}}
FIELDS = {'schema_version', 'event', 'outcome', 'utc_unix_ms', 'world_time', 'connection', 'object_id', 'object_class', 'declaring_class', 'function', 'actor_id', 'channel', 'payload_bits', 'reliable'}
CLASS_PATH = re.compile(r'^/[^/.\s]+(?:/[^/.\s]+)*\.[^/.\s]+$')


def integer(value):
    return isinstance(value, int) and not isinstance(value, bool)


def validate(record):
    if not isinstance(record, dict):
        return 'record must be an object'
    if record.keys() != FIELDS:
        return 'fields differ from schema 1: missing=%s extra=%s' % (sorted(FIELDS - record.keys()), sorted(record.keys() - FIELDS))
    if not integer(record['schema_version']) or record['schema_version'] != 1:
        return 'unsupported schema_version'
    if not isinstance(record['event'], str) or record['event'] not in OUTCOMES or not isinstance(record['outcome'], str) or record['outcome'] not in OUTCOMES[record['event']]:
        return 'invalid event/outcome'
    for name in ('utc_unix_ms', 'connection', 'object_id'):
        if not integer(record[name]) or record[name] < 0:
            return name + ' must be a nonnegative integer'
    for name in ('actor_id', 'channel', 'payload_bits'):
        if record[name] is not None and (not integer(record[name]) or record[name] < 0):
            return name + ' must be null or a nonnegative integer'
    v = record['world_time']
    if v is not None and (isinstance(v, bool) or not isinstance(v, (int, float)) or not math.isfinite(v)):
        return 'world_time must be null or finite number'
    for name in ('object_class', 'declaring_class'):
        if name == 'declaring_class' and record[name] is None and record['outcome'] == 'invalid_function':
            continue
        if not isinstance(record[name], str) or not CLASS_PATH.fullmatch(record[name]):
            return name + ' must be a full Unreal class object path'
    if not isinstance(record['function'], str) or not record['function']:
        return 'function must be a nonempty string'
    if not isinstance(record['reliable'], bool) and not (record['reliable'] is None and record['outcome'] == 'invalid_function'):
        return 'reliable must be boolean'
    return None


def source(path):
    raw = path.read_bytes()
    return raw, {'path': str(path.resolve()), 'sha256': hashlib.sha256(raw).hexdigest()}


def index_inventory(rows):
    index = defaultdict(set)
    for i, row in enumerate(rows):
        if row['kind'] != 'rpc':
            continue
        for variant in row['variants']:
            meta = variant.get('parent_metadata') or {}
            owner = variant.get('declared_owner')
            package = meta.get('package')
            # The package and matching parent identity establish the qualified class;
            # do not use a bare C++ name as a global class identity.
            if owner and meta.get('name') == owner and isinstance(package, str) and package.startswith('/Script/') and len(owner) > 1 and owner[0] in 'AU':
                index[(package + '.' + owner[1:], row['name'])].add(i)
            # Keep unresolved candidate blocks visible as possible competing joins.
            elif package and isinstance(package, str):
                for candidate in variant.get('owner_candidates') or []:
                    if len(candidate) > 1 and candidate[0] in 'AU':
                        index[(package + '.' + candidate[1:], row['name'])].add(i)
    return index


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('traces', nargs='+', type=Path)
    p.add_argument('--inventory', type=Path, default=PORT / 'generated/agents/network-contract-inventory/declarations.jsonl')
    p.add_argument('--output', type=Path, default=PORT / 'generated/agents/runtime-network-report')
    args = p.parse_args(argv)
    try:
        raw, inventory_source = source(args.inventory)
        rows = [json.loads(line) for line in raw.splitlines() if line.strip()]
        for row in rows:
            if not isinstance(row, dict) or row.get('kind') not in ('rpc', 'replicated_field') or not isinstance(row.get('owner_key'), str) or not isinstance(row.get('name'), str) or not isinstance(row.get('variants'), list):
                raise ValueError('invalid inventory declaration')
        index = index_inventory(rows)
        aggregates, errors, trace_sources, observed = {}, [], [], set()
        for path in args.traces:
            raw, provenance = source(path)
            trace_sources.append(provenance)
            for number, line in enumerate(raw.splitlines(), 1):
                try:
                    record = json.loads(line)
                    error = validate(record)
                except (ValueError, TypeError, UnicodeDecodeError) as exc:
                    error = 'invalid JSON: ' + str(exc)
                if error:
                    errors.append({'path': provenance['path'], 'sha256': provenance['sha256'], 'line': number, 'error': error})
                    continue
                candidates = sorted(index.get((record['declaring_class'], record['function']), set()))
                # A block with unresolved or duplicated declaration identity is not a safe join.
                match = candidates[0] if len(candidates) == 1 and not rows[candidates[0]]['owner_key'].startswith('@') and rows[candidates[0]].get('declaration_count') == 1 else None
                status = 'matched' if match is not None else ('ambiguous' if candidates else 'unmatched')
                if match is not None:
                    observed.add(match)
                key = (record['declaring_class'], record['function'], record['connection'], record['event'], record['outcome'])
                if key not in aggregates:
                    aggregates[key] = {'declaring_class': key[0], 'function': key[1], 'connection': key[2], 'event': key[3], 'outcome': key[4], 'join_status': status, 'owner_key': rows[match]['owner_key'] if match is not None else None, 'candidate_owner_keys': sorted({rows[i]['owner_key'] for i in candidates}), 'count': 0, 'first_utc_unix_ms': record['utc_unix_ms'], 'last_utc_unix_ms': record['utc_unix_ms'], 'payload_bits_total': 0, 'payload_bits_known_count': 0, 'evidence': [], 'behavior_parity': 'unknown', 'receiver_observed': 'unknown'}
                a = aggregates[key]
                a['count'] += 1
                a['first_utc_unix_ms'] = min(a['first_utc_unix_ms'], record['utc_unix_ms'])
                a['last_utc_unix_ms'] = max(a['last_utc_unix_ms'], record['utc_unix_ms'])
                if record['payload_bits'] is not None:
                    a['payload_bits_total'] += record['payload_bits']
                    a['payload_bits_known_count'] += 1
                a['evidence'].append({'path': provenance['path'], 'sha256': provenance['sha256'], 'line': number})
        declarations = [{'kind': r['kind'], 'owner_key': r['owner_key'], 'name': r['name'], 'status': ('observed_in_provided_traces' if i in observed else 'not_observed_in_provided_traces') if r['kind'] == 'rpc' else 'not_covered_by_rpc_trace', 'behavior_parity': 'unknown', 'receiver_observed': 'unknown'} for i, r in enumerate(rows)]
        args.output.mkdir(parents=True, exist_ok=True)
        report = {'schema_version': 1, 'inventory_source': inventory_source, 'trace_sources': trace_sources, 'valid_observation_count': sum(a['count'] for a in aggregates.values()), 'invalid_record_count': len(errors), 'trace_status': 'invalid_records' if errors else ('observations_present' if aggregates else 'no_observations'), 'count_semantics': 'Counts are trace records, not distinct calls; invoked and validation_failed may describe the same invocation.', 'stage_semantics': {'queued': 'queued locally; delivery unknown', 'submitted': 'handed to SendBunch; acknowledgement and delivery unknown', 'invoked': 'attempt immediately before ProcessEvent; behavior correctness unknown', 'validation_failed': 'optional later separate record', 'payload_bits': 'incoming entry Reader.GetBitsLeft; outgoing serialized parameter bits, or null before serialization'}, 'observations': [aggregates[k] for k in sorted(aggregates, key=lambda k: (k[0] or '', *k[1:]))], 'declarations': declarations, 'errors': errors}
        (args.output / 'report.json').write_text(json.dumps(report, indent=2, sort_keys=True, allow_nan=False) + '\n')
        print(json.dumps({'trace_status': report['trace_status'], 'valid_observation_count': report['valid_observation_count'], 'invalid_record_count': len(errors)}, sort_keys=True))
        return 1 if errors else 0
    except (OSError, ValueError, KeyError, TypeError) as exc:
        print('runtime report error: ' + str(exc), file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
