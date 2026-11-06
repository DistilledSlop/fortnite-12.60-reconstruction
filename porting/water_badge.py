#!/usr/bin/env python3
"""Estimate AI inference water from usage and render a documented scenario range.

Refresh: python3 porting/water_badge.py [--publish]
  Default source: ccusage for every agent it detects, from the first commit to today (UTC),
  with local models excluded by name.
  --logs: the older project-scoped log collectors (Fortnite workspace only).
Details: porting/progress/WATER.md. Publishing never reads private logs.
"""
import argparse
from collections import defaultdict
from datetime import datetime, timezone
import hashlib
import json
import gzip
import math
import re
from pathlib import Path
import shutil
import sqlite3
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
    if raw.get('method'):
        if raw['method'] != 'ccusage':
            raise ValueError('unsupported usage method')
        row['method'] = raw['method']
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


CCUSAGE_TIMEZONE = 'UTC'
CCUSAGE_FIELDS = (('input_tokens', 'inputTokens'), ('cache_read_tokens', 'cacheReadTokens'),
                  ('cache_write_tokens', 'cacheCreationTokens'), ('output_tokens', 'outputTokens'))
CCUSAGE_LIMITATIONS = [
    'The default ccusage source counts all cloud agent usage detected on this machine in the window, including projects other than Fortnite; project scoping is unavailable (use --logs for the project-scoped collectors).',
    'ccusage reports no request counts, so requests are reported as unavailable rather than zero.',
    'ccusage has no reasoning field. Where its totalTokens exceeds the reported fields (antigravity and opencode rows, about 0.01%), the excess is recorded as reasoning, charged at the output rate.',
    'Models whose provider is not claude, gpt or gemini and that are not local (for example OpenCode deepseek-v4.1-flash-free) are mapped to unknown and estimated as cloud-hosted.',
]


def ccusage_limitations(coverage):
    local = coverage['ccusage'].get('local_models', [])
    notes = [f"Local models excluded by name: {', '.join(local)}. Local status comes from OpenCode providerID llamacpp/local and porting/water_profiles.json local_models. A model name used by both local and cloud providers is excluded everywhere, so its cloud usage is dropped too."] if local else []
    return [*CCUSAGE_LIMITATIONS, *notes]


LOCAL_PROVIDER_IDS = ('llamacpp', 'local')  # OpenCode providerIDs that run locally (water_collectors._opencode)


def local_models(profiles, opencode_db):
    """Model names served locally: the profile list plus OpenCode rows with a local providerID.

    A missing or unreadable OpenCode DB leaves the profile list in force; the status says which.
    """
    names = {_label(n, r'[^A-Za-z0-9_.:/<>+-]', 150, 'unknown') for n in profiles.get('local_models', [])}
    status = 'missing'
    path = Path(opencode_db)
    if path.exists():
        try:
            with sqlite3.connect('file:' + str(path) + '?mode=ro', uri=True) as conn:
                for (data,) in conn.execute('SELECT data FROM session_message'):
                    try:
                        obj = json.loads(data)
                    except (TypeError, ValueError):
                        continue
                    model = obj.get('model') if isinstance(obj, dict) else None
                    if (isinstance(model, dict) and str(model.get('providerID') or '').lower() in LOCAL_PROVIDER_IDS
                            and model.get('id')):
                        names.add(_label(model['id'], r'[^A-Za-z0-9_.:/<>+-]', 150, 'unknown'))
            status = 'read'
        except sqlite3.Error:
            status = 'unreadable'
    return names, status


def ccusage_provider(model):
    name = model.lower()
    if name.startswith('claude-'):
        return 'anthropic'
    if name.startswith('gpt-'):
        return 'openai'
    if name.startswith('gemini-'):
        return 'google'
    return 'unknown'


def _label(value, pattern, limit, fallback):
    text = re.sub(pattern, '_', str(value or '')).strip('_')[:limit]
    return text or fallback


def first_commit_date(repo):
    """UTC calendar day of the first commit listed by `git log --reverse`."""
    out = subprocess.run(['git', '-C', str(repo), 'log', '--reverse', '--format=%cI'],
                         capture_output=True, text=True, check=True).stdout.split()
    if not out:
        raise ValueError('repository has no commits')
    return datetime.fromisoformat(out[0].replace('Z', '+00:00')).astimezone(timezone.utc).date().isoformat()


def run_ccusage(binary, since, until):
    command = [str(binary), 'daily', '-j', '--since', since, '--until', until,
               '--by-agent', '--offline', '--no-cost', '-z', CCUSAGE_TIMEZONE]
    done = subprocess.run(command, capture_output=True, text=True, timeout=900)
    if done.returncode:
        raise RuntimeError('ccusage failed: ' + done.stderr.strip()[-500:])
    return json.loads(done.stdout)


def ccusage_records(payload, since, until, excluded=frozenset(), stats=None):
    """Normalize ccusage daily rows into one record per (day, agent, model).

    Input excludes cache reads and writes, as ccusage's totalTokens sums them
    separately. Output is recorded as reported; any totalTokens excess over the
    reported fields is attributed to reasoning (see below), never added twice.
    Models named in `excluded` (local models) are dropped by name; their token
    totals go to stats['excluded'], and stats['unattributed'] takes any residual
    that has no kept model to carry it.
    """
    stats = {} if stats is None else stats
    stats.setdefault('excluded', {})
    stats.setdefault('unattributed', 0)
    if not isinstance(payload, dict) or not isinstance(payload.get('daily'), list):
        raise ValueError('ccusage output lacks a daily list')
    grouped = {}
    for row in payload['daily']:
        day = row.get('period') or row.get('date')
        if not isinstance(day, str) or not re.fullmatch(r'\d{4}-\d{2}-\d{2}', day):
            raise ValueError('ccusage row lacks a YYYY-MM-DD period')
        if not since <= day <= until:
            continue
        if isinstance(row.get('agents'), list):
            agent_rows = row['agents']
        elif row.get('agent') not in (None, 'all'):
            agent_rows = [row]
        else:
            raise ValueError('ccusage row lacks a per-agent breakdown; ccusage must run with --by-agent')
        for agent_row in agent_rows:
            agent = _label(str(agent_row.get('agent') or '').lower(), r'[^a-z0-9_-]', 40, 'unknown')
            breakdowns = agent_row.get('modelBreakdowns')
            if not isinstance(breakdowns, list):
                raise ValueError(f'ccusage {agent} {day} lacks modelBreakdowns')
            sums = {src: 0 for _, src in CCUSAGE_FIELDS}
            placed = []
            for item in breakdowns:
                model = _label(item.get('modelName'), r'[^A-Za-z0-9_.:/<>+-]', 150, 'unknown')
                values = {}
                for field, src in CCUSAGE_FIELDS:
                    value = item.get(src, 0)
                    if isinstance(value, bool) or not isinstance(value, int) or value < 0:
                        raise ValueError(f'invalid ccusage count: {src}')
                    values[field] = value
                    sums[src] += value
                if model in excluded:
                    stats['excluded'][model] = stats['excluded'].get(model, 0) + sum(values.values())
                    continue
                counts = grouped.setdefault((day, agent, model), dict.fromkeys(COUNTS, 0))
                for field, value in values.items():
                    counts[field] += value
                placed.append((counts, values['output_tokens']))
            # Agent-level totals must equal their model breakdowns, or tokens would vanish silently.
            for src, total in sums.items():
                if src in agent_row and agent_row[src] != total:
                    raise ValueError(f'ccusage {agent} {day} {src} does not match its model breakdowns')
            # ccusage totalTokens can exceed the four fields it reports (observed on antigravity and
            # opencode rows, with no field to name it). The excess is reasoning, charged at the output
            # rate. It is computed over every breakdown, so excluded tokens are subtracted out, and it is
            # allocated over kept models only, so excluded tokens never reach a kept row.
            residual = agent_row['totalTokens'] - sum(sums.values()) if 'totalTokens' in agent_row else 0
            if residual < 0:
                raise ValueError(f'ccusage {agent} {day} totalTokens is below its reported fields')
            if residual and not placed:
                stats['unattributed'] += residual
            elif residual:
                for (counts, _), part in zip(placed, _split_residual(residual, [out for _, out in placed])):
                    counts['reasoning_tokens'] += part
    records = []
    for (day, agent, model), counts in sorted(grouped.items()):
        identity = hashlib.sha256(f'ccusage:{day}:{agent}:{model}'.encode()).hexdigest()[:24]
        records.append(dict(provider=ccusage_provider(model), model=model, client=agent, method='ccusage',
                            timestamp=f'{day}T12:00:00+00:00', source=identity, requests=0, **counts))
    return records


def _split_residual(residual, weights):
    """Split an integer residual by weight; the largest share absorbs rounding."""
    total = sum(weights)
    if total == 0:
        weights, total = [1] + [0] * (len(weights) - 1), 1
    parts = [residual * w // total for w in weights]
    parts[weights.index(max(weights))] += residual - sum(parts)
    return parts


def ccusage_coverage(records, payload, since, until, local=None, stats=None):
    local, stats = local or {}, stats or {}
    excluded = dict(sorted(stats.get('excluded', {}).items()))
    return {'ccusage': {
        'command': f'ccusage daily -j --since {since} --until {until} --by-agent --offline --no-cost -z {CCUSAGE_TIMEZONE}',
        'window': [since, until], 'daily_rows': len(payload['daily']), 'usage_records': len(records),
        'agents': sorted({r['client'] for r in records}),
        'project_scoping': 'none: all agent usage detected on this machine in the window',
        'request_counts': 'not reported by ccusage',
        'local_models': local.get('names', []), 'opencode_db': local.get('opencode_db'),
        'local_excluded_tokens': excluded, 'local_excluded_total': sum(excluded.values()),
        'unattributed_residual_tokens': stats.get('unattributed', 0)}}


def scoped_history(previous, window):
    """ccusage replaces ledger rows inside its window; default refreshes rebuild from logs only."""
    if window:
        return [r for r in previous if not (r.get('timestamp') and window[0] <= r['timestamp'][:10] <= window[1])]
    return [r for r in previous if r.get('method') != 'ccusage']


def summarize(records, coverage, profiles, timestamp, retained=0):
    ccusage = 'ccusage' in coverage
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
    return {'schema_version': 2, 'updated_at': timestamp,
            'scope': ('Cloud agent usage detected by ccusage on this machine in the window, local models excluded; not project-scoped'
                      if ccusage else 'Fortnite inference with recorded usage'),
            'usage_first_at': min(dated) if dated else None, 'usage_last_at': max(dated) if dated else None,
            'records_without_timestamps': len(records) - len(dated),
            'requests_reported': not ccusage,
            'recorded_requests': None if ccusage else sum(r['requests'] for r in records),
            'range_kind': 'illustrative scenarios; not confidence bounds or bounds on missing usage',
            'scenarios': profiles['scenarios'], 'estimate': totals,
            'tokens': sum(sum(r[k] for k in COUNTS) for r in records),
            'token_types': {k: sum(r[k] for r in records) for k in COUNTS},
            'models': models, 'coverage': coverage, 'retained_historical_records': retained,
            'assumptions_file': 'porting/water_profiles.json',
            'limitations': ['Missing logs and unreported token categories are excluded, never treated as known zero.',
                            'Specific model names use heuristic family profiles unless explicitly overridden.',
                            'Uncertain geography, hardware, batching and long-context effects prevent calibrated error bounds.',
                            'Inference only: excludes training, hardware manufacture, network, builds and game runtime.']
            + (ccusage_limitations(coverage) if ccusage else [])}


def number(v):
    """Two significant digits: scenario assumptions do not justify decimals."""
    if v == 0:
        return '0'
    decimals = 1 - math.floor(math.log10(abs(v)))
    rounded = round(v, decimals)
    return f'{rounded:,.{max(0, decimals)}f}'


def report(data):
    e = data['estimate']
    coverage_text = (f"Recorded **{data['recorded_requests']:,} requests** across **{len(data['models'])} model identifiers**. "
                     if data['requests_reported'] else
                     f"Request counts are not reported by ccusage; usage spans **{len(data['models'])} model identifiers**. ")
    excluded = data['coverage'].get('ccusage', {}).get('local_excluded_tokens', {})
    local_text = (' Excluded local models (by name, not counted): '
                  + '; '.join(f'{m} {t / 1e6:,.1f}M tokens' for m, t in excluded.items()) + '.') if excluded else ''
    lines = ['# Estimated AI water for Fortnite', '',
             f"Refreshed **{data['updated_at']}**. Recorded inference usage only; missing usage remains unquantified.", '',
             coverage_text +
             "Token volume counts repeated cache reads across calls, not unique text." + local_text, '',
             f"**Central scenario: {number(e['gallons'][1])} US gallons. Scenario range: "
             f"{number(e['gallons'][0])}–{number(e['gallons'][2])} US gallons.**", '',
             'This is an assumption-driven scenario range, not a measurement, confidence interval, or bound on the entire project.', '',
             '| Component | Low | Central | High |', '|---|---:|---:|---:|',
             *[f'| {label} | ' + ' | '.join(number(v) for v in e[key]) + ' |' for label, key in
               (('Serving energy (kWh)', 'energy_kwh'), ('Cooling water (L)', 'cooling_liters'),
                ('Electricity generation water (L)', 'electricity_liters'), ('Total consumption (L)', 'total_liters'))], '',
             'Known token overlaps are removed: output excludes separately reported reasoning. Antigravity input/cache separation remains an explicit assumption; see coverage and methodology.', '',
             '| Provider / model | Client | Proxy | Input | Cache read | Cache write | Output | Reasoning | Central gal | Low–high gal |',
             '|---|---|---|---:|---:|---:|---:|---:|---:|---:|']
    for r in data['models']:
        lines.append(f"| {r['provider']} / {r['model']} | {', '.join(r['clients']) or 'not recorded'} | {r['tier']} | "
                     + ' | '.join(f'{r[k]:,}' for k in COUNTS)
                     + f" | {number(r['gallons'][1])} | {number(r['gallons'][0])}–{number(r['gallons'][2])} |")
    lines += ['', '## Collection coverage', '', '```json', json.dumps(data['coverage'], indent=2, sort_keys=True), '```', '',
              f"Retained records from earlier refreshes whose source is no longer present: {data['retained_historical_records']}.", '',
              'See [methodology and assumptions](WATER.md) and [machine-readable snapshot](water.json).', '']
    return '\n'.join(lines)


def build_parser():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--output-dir', type=Path, default=HERE / 'progress')
    parser.add_argument('--profiles', type=Path, default=HERE / 'water_profiles.json')
    parser.add_argument('--project-root', type=Path, default=Path('/opt/fortnite'),
                        help='--logs only: parent Fortnite workspace, also containing worker packages')
    for name in ('codex-root', 'claude-root', 'gemini-root', 'antigravity-root', 'opencode-db'):
        parser.add_argument('--' + name, type=Path, default=None,
                            help='--logs: log location override; opencode-db is also read for local-model names' if name == 'opencode-db' else None)
    parser.add_argument('--replace-ledger', action='store_true',
                        help='Discard retained ledger history before refreshing')
    parser.add_argument('--publish', action='store_true', help='Refresh only the README water badge after saving')
    parser.add_argument('--import-usage', action='append', type=Path, default=[],
                        help='Additional normalized JSONL usage; each record must declare project=fortnite-12.60-reconstruction')
    parser.add_argument('--logs', action='store_true',
                        help='Use the project-scoped log collectors instead of the default ccusage source')
    parser.add_argument('--ccusage', action='store_true', help='Accepted for compatibility; ccusage is already the default')
    parser.add_argument('--since', help='ccusage source: first day YYYY-MM-DD (default: first commit date)')
    parser.add_argument('--until', help='ccusage source: last day YYYY-MM-DD (default: today, UTC)')
    parser.add_argument('--ccusage-bin', type=Path, default=None, help='ccusage source: ccusage executable')
    return parser


def source_plan(args):
    """Return the ccusage window (since, until), or None when the --logs collectors are used."""
    if args.logs and args.ccusage:
        raise ValueError('--logs and --ccusage are mutually exclusive (ccusage is the default)')
    if args.logs:
        if args.since or args.until or args.ccusage_bin:
            raise ValueError('--since, --until and --ccusage-bin apply to the ccusage source and cannot be used with --logs')
        return None
    window = (args.since or first_commit_date(HERE.parent),
              args.until or datetime.now(timezone.utc).date().isoformat())
    if not all(re.fullmatch(r'\d{4}-\d{2}-\d{2}', day) for day in window) or window[0] > window[1]:
        raise ValueError('--since/--until must be YYYY-MM-DD with since <= until')
    return window


def gather_usage(args, window):
    """Usage records and coverage: ccusage over `window`, or the log collectors when window is None."""
    if window is None:
        from water_collectors import collect_usage
        return collect_usage(project_root=str(args.project_root), repo=str(HERE.parent),
                             **{name: getattr(args, name) for name in
                                ('codex_root', 'claude_root', 'gemini_root', 'antigravity_root', 'opencode_db')})
    profiles = json.loads(args.profiles.read_text())
    local, status = local_models(profiles, args.opencode_db or Path.home() / '.local/share/opencode/opencode.db')
    binary = args.ccusage_bin or shutil.which('ccusage') or Path.home() / '.local/bin/ccusage'
    payload = run_ccusage(binary, *window)
    stats = {}
    current = ccusage_records(payload, *window, excluded=frozenset(local), stats=stats)
    coverage = ccusage_coverage(current, payload, *window,
                                local={'names': sorted(local), 'opencode_db': status}, stats=stats)
    return current, coverage


def main():
    parser = build_parser()
    args = parser.parse_args()
    if args.publish and args.output_dir.resolve() != (HERE / 'progress').resolve():
        parser.error('--publish requires the default output directory')
    try:
        window = source_plan(args)
    except ValueError as error:
        parser.error(str(error))
    current, coverage = gather_usage(args, window)
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
    stale = sum(r.get('method') == 'ccusage' for r in previous) if window is None else 0
    previous = scoped_history(previous, window)
    if stale:
        print(f'Dropped {stale:,} ccusage ledger rows: --logs rebuilds from logs only', file=sys.stderr)
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
