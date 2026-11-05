#!/usr/bin/env python3
"""Backfill Fortnite inference usage and render a documented water scenario range.

Refresh: python3 porting/water_badge.py
Details: porting/progress/WATER.md. Publishing never reads private logs.
"""
import argparse
from collections import defaultdict
from datetime import datetime, timezone
import json
import gzip
import math
import re
from pathlib import Path
import subprocess
import sys

HERE = Path(__file__).resolve().parent
COUNTS = ('input_tokens', 'cache_read_tokens', 'cache_write_tokens', 'output_tokens', 'reasoning_tokens')
GALLON_LITERS = 3.785411784


def model_tier(model):
    """Name-based workload proxies, explicitly not inferred parameter counts."""
    name = model.lower()
    if any(x in name for x in ('astra', 'opus')):
        return 'large'
    if any(x in name for x in ('luna', 'haiku')) or ('gemini' in name and 'flash' in name):
        return 'small'
    if any(x in name for x in ('sol', 'terra', 'sonnet', 'gemini', 'qwen', 'gpt-4', 'gpt-5', 'o3', 'o4')):
        return 'standard'
    return 'unknown'


def estimate(record, profiles):
    override = profiles.get('model_overrides', {}).get(record['model'], {})
    tier = override.get('tier', model_tier(record['model']))
    rates = dict(profiles['tiers'][tier], **{k: v for k, v in override.items() if k != 'tier'})
    energy, site, electricity = [], [], []
    for i in range(3):
        input_equiv = (record['input_tokens']
                       + record['cache_read_tokens'] * profiles['cache_read_input_multiplier'][i]
                       + record['cache_write_tokens'] * profiles['cache_write_input_multiplier'][i])
        output = record['output_tokens'] + record['reasoning_tokens']
        kwh = (input_equiv * rates['input_kwh_per_million'][i]
               + output * rates['output_kwh_per_million'][i]) / 1e6
        energy.append(kwh)
        # Local air-cooled machines still consume water through electricity generation.
        site.append(0. if record['provider'] == 'local' else
                    kwh / profiles['cloud_pue'][i] * profiles['cloud_site_liters_per_it_kwh'][i])
        electricity.append(kwh * profiles['electricity_liters_per_facility_kwh'][i])
    total = [a + b for a, b in zip(site, electricity)]
    return {'tier': tier, 'energy_kwh': energy, 'cooling_liters': site,
            'electricity_liters': electricity, 'total_liters': total,
            'gallons': [v / GALLON_LITERS for v in total]}


def clean_record(raw):
    # Whitelist: private source content and paths must never enter Git.
    row = {k: str(raw[k]) for k in ('provider', 'model', 'source')}
    if not re.fullmatch(r'[a-f0-9]{24,64}', row['source']):
        raise ValueError('source must be a hashed usage identity')
    aliases = raw.get('aliases', [])
    if not isinstance(aliases, list) or any(not isinstance(a, str) or not re.fullmatch(r'[a-f0-9]{24,64}', a) for a in aliases):
        raise ValueError('aliases must be hashed usage identities')
    if aliases:
        row['aliases'] = sorted(set(aliases) - {row['source']})
    if any(len(row[k]) > 150 or not re.fullmatch(r'[A-Za-z0-9_.:/<>+-]+', row[k])
           for k in ('provider', 'model')):
        raise ValueError('invalid provider/model identifier')
    if raw.get('client'):
        if not re.fullmatch(r'[a-z0-9_-]{1,40}', raw['client']):
            raise ValueError('invalid client identifier')
        row['client'] = raw['client']
    if raw.get('provider_detail'):
        if not re.fullmatch(r'[A-Za-z0-9_.:/+-]{1,100}', raw['provider_detail']):
            raise ValueError('invalid provider detail')
        row['provider_detail'] = raw['provider_detail']
    for k in (*COUNTS, 'requests'):
        value = raw.get(k, 0)
        if isinstance(value, bool) or not isinstance(value, int) or value < 0:
            raise ValueError(f'invalid count: {k}')
        row[k] = value
    if raw.get('timestamp'):
        stamp = datetime.fromisoformat(raw['timestamp'].replace('Z', '+00:00'))
        if stamp.tzinfo is None:
            raise ValueError('timestamp requires a timezone')
        row['timestamp'] = stamp.astimezone(timezone.utc).isoformat()
    return row


def merge_records(previous, current):
    """Replace refreshed counters; retain records whose logs were rotated away."""
    def key(r):
        return r['source']
    result = {key(r): clean_record(r) for r in previous}
    refreshed = {}
    for raw in current:
        row = clean_record(raw)
        identity = key(row)
        # A direct request can supersede a previously saved cumulative fallback.
        for alias in row.get('aliases', []):
            result.pop(alias, None)
        if identity in refreshed:
            old = refreshed[identity]
            if row['model'] == 'unknown' and old['model'] != 'unknown':
                row['model'] = old['model']
            total_output = max(old['output_tokens'] + old['reasoning_tokens'], row['output_tokens'] + row['reasoning_tokens'])
            for field in (*COUNTS, 'requests'):
                row[field] = max(old[field], row[field])
            row['output_tokens'] = max(0, total_output - row['reasoning_tokens'])
            aliases = sorted(set(row.get('aliases', [])) | set(old.get('aliases', [])))
            if aliases:
                row['aliases'] = aliases
        refreshed[identity] = row
    # Remove fallback identities even if they were also seen in this scan.
    superseded = {alias for r in refreshed.values() for alias in r.get('aliases', [])}
    for alias in superseded:
        refreshed.pop(alias, None)
    result.update(refreshed)
    return sorted(result.values(), key=lambda r: (r['provider'], r['model'], r['source']))


def summarize(records, coverage, profiles, timestamp, retained=0):
    grouped = defaultdict(lambda: {k: 0 for k in (*COUNTS, 'requests')})
    for record in records:
        group = grouped[record['provider'], record['model']]
        for k in group:
            group[k] += record[k]
    models = []
    for (provider, model), counts in sorted(grouped.items()):
        row = dict(provider=provider, model=model, **counts)
        matching = [r for r in records if r['provider'] == provider and r['model'] == model]
        row['clients'] = sorted({r['client'] for r in matching if r.get('client')})
        row['provider_details'] = sorted({r['provider_detail'] for r in matching if r.get('provider_detail')})
        row.update(estimate(row, profiles))
        models.append(row)
    totals = {k: [sum(row[k][i] for row in models) for i in range(3)]
              for k in ('energy_kwh', 'cooling_liters', 'electricity_liters', 'total_liters', 'gallons')}
    dated = [r['timestamp'] for r in records if r.get('timestamp')]
    return {'schema_version': 2, 'updated_at': timestamp, 'scope': 'Fortnite inference with recorded usage',
            'usage_first_at': min(dated) if dated else None, 'usage_last_at': max(dated) if dated else None,
            'records_without_timestamps': len(records) - len(dated),
            'recorded_requests': sum(r['requests'] for r in records),
            'range_kind': 'illustrative scenarios; not confidence bounds or bounds on missing usage',
            'scenarios': profiles['scenarios'], 'estimate': totals,
            'tokens': sum(sum(r[k] for k in COUNTS) for r in records),
            'token_types': {k: sum(r[k] for r in records) for k in COUNTS},
            'models': models, 'coverage': coverage, 'retained_historical_records': retained,
            'assumptions_file': 'porting/water_profiles.json',
            'limitations': ['Missing logs and unreported token categories are excluded, never treated as known zero.',
                            'Specific model names use heuristic family profiles unless explicitly overridden.',
                            'Uncertain geography, hardware, batching and long-context effects prevent calibrated error bounds.',
                            'Inference only: excludes training, hardware manufacture, network, builds and game runtime.']}


def number(v):
    """Two significant digits: scenario assumptions do not justify decimals."""
    if v == 0:
        return '0'
    decimals = 1 - math.floor(math.log10(abs(v)))
    rounded = round(v, decimals)
    return f'{rounded:,.{max(0, decimals)}f}'


def report(data):
    e = data['estimate']
    lines = ['# Estimated AI water for Fortnite', '',
             f"Refreshed **{data['updated_at']}**. Recorded inference usage only; missing usage remains unquantified.", '',
             f"Recorded **{data['recorded_requests']:,} requests** across **{len(data['models'])} model identifiers**. "
             "Token volume counts repeated cache reads across calls, not unique text.", '',
             f"**Central scenario: {number(e['gallons'][1])} US gallons. Scenario range: "
             f"{number(e['gallons'][0])}–{number(e['gallons'][2])} US gallons.**", '',
             'This is an assumption-driven scenario range, not a measurement, confidence interval, or bound on the entire project.', '',
             '| Component | Low | Central | High |', '|---|---:|---:|---:|',
             *[f'| {label} | ' + ' | '.join(number(v) for v in e[key]) + ' |' for label, key in
               (('Serving energy (kWh)', 'energy_kwh'), ('Cooling water (L)', 'cooling_liters'),
                ('Electricity generation water (L)', 'electricity_liters'), ('Total consumption (L)', 'total_liters'))], '',
             'Known token overlaps are removed: output excludes separately reported reasoning. Antigravity input/cache separation remains an explicit assumption; see coverage and methodology.', '',
             '| Provider / model | Proxy | Input | Cache read | Cache write | Output | Reasoning | Central gal | Low–high gal |',
             '|---|---|---:|---:|---:|---:|---:|---:|---:|']
    for r in data['models']:
        lines.append(f"| {r['provider']} / {r['model']} | {r['tier']} | "
                     + ' | '.join(f'{r[k]:,}' for k in COUNTS)
                     + f" | {number(r['gallons'][1])} | {number(r['gallons'][0])}–{number(r['gallons'][2])} |")
    lines += ['', '## Collection coverage', '', '```json', json.dumps(data['coverage'], indent=2, sort_keys=True), '```', '',
              f"Retained records from earlier refreshes whose source is no longer present: {data['retained_historical_records']}.", '',
              'See [methodology and assumptions](WATER.md) and [machine-readable snapshot](water.json).', '']
    return '\n'.join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-dir', type=Path, default=HERE / 'progress')
    parser.add_argument('--profiles', type=Path, default=HERE / 'water_profiles.json')
    parser.add_argument('--project-root', type=Path, default=Path('/opt/fortnite'),
                        help='Parent Fortnite workspace, also containing worker packages')
    for name in ('codex-root', 'claude-root', 'gemini-root', 'antigravity-root', 'opencode-db'):
        parser.add_argument('--' + name, type=Path, default=None)
    parser.add_argument('--replace-ledger', action='store_true', help='Rebuild from available logs only, discarding retained history')
    parser.add_argument('--publish', action='store_true', help='Refresh only the README water badge after saving')
    parser.add_argument('--import-usage', action='append', type=Path, default=[],
                        help='Additional normalized JSONL usage; each record must declare project=fortnite-12.60-reconstruction')
    args = parser.parse_args()
    if args.publish and args.output_dir.resolve() != (HERE / 'progress').resolve():
        parser.error('--publish requires the default output directory')
    from water_collectors import collect_usage
    current, coverage = collect_usage(project_root=str(args.project_root), repo=str(HERE.parent),
                                     **{name: getattr(args, name) for name in
                                        ('codex_root', 'claude_root', 'gemini_root', 'antigravity_root', 'opencode_db')})
    for path in args.import_usage:
        imported = []
        for line in path.read_text().splitlines():
            if not line.strip():
                continue
            raw = json.loads(line)
            if raw.pop('project', None) != 'fortnite-12.60-reconstruction':
                raise ValueError('Imported usage must explicitly declare the Fortnite project')
            imported.append(clean_record(raw))
        current.extend(imported)
        coverage.setdefault('imports', {'usage_records': 0})['usage_records'] += len(imported)
    ledger = args.output_dir / 'water_usage.json.gz'
    previous = []
    if ledger.exists() and not args.replace_ledger:
        saved = json.loads(gzip.decompress(ledger.read_bytes()))
        if saved['schema_version'] != 1:
            raise ValueError('Unsupported usage ledger schema')
        previous = saved['records']
    records = merge_records(previous, current)
    if not records:
        raise ValueError('No attributable usage available; refusing to publish zero consumption')
    current_keys = {identity for r in current for identity in (r['source'], *r.get('aliases', []))}
    retained = sum(r['source'] not in current_keys for r in records)
    profiles = json.loads(args.profiles.read_text())
    timestamp = datetime.now(timezone.utc).isoformat(timespec='seconds')
    data = summarize(records, coverage, profiles, timestamp, retained)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    ledger.write_bytes(gzip.compress(json.dumps({'schema_version': 1, 'records': records}, separators=(',', ':')).encode(), mtime=0))
    (args.output_dir / 'water.json').write_text(json.dumps(data, indent=2) + '\n')
    (args.output_dir / 'WATER_REPORT.md').write_text(report(data))
    lo, mid, hi = data['estimate']['gallons']
    print(f"{len(records):,} usage records; {data['tokens']:,} tokens; {number(mid)} gal ({number(lo)}–{number(hi)}); {args.output_dir}")
    if args.publish:
        subprocess.run([sys.executable, str(HERE / 'progress/publish.py'), '--water-only'], check=True)


if __name__ == '__main__':
    main()
