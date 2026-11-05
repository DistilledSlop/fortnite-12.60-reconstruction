"""Read local provider usage logs without retaining conversation content.

Token fields in returned records are mutually exclusive. The caller must treat
coverage as partial: local logs cannot account for unrecorded provider calls.
"""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import subprocess
import sqlite3
from datetime import datetime, timezone

FIELDS = ('input_tokens', 'cache_read_tokens', 'cache_write_tokens', 'output_tokens', 'reasoning_tokens')
REPO = '/opt/fortnite/FortniteClient-12.60-CL-13477524'


def _hash(value):
    return hashlib.sha256(str(value).encode()).hexdigest()[:24]


def _num(value):
    return value if isinstance(value, int) and not isinstance(value, bool) and value >= 0 else 0


def _record(provider, model, usage, key, *, codex=False):
    read = _num(usage.get('cached_input_tokens' if codex else 'cache_read_input_tokens'))
    write = _num(usage.get('cache_write_input_tokens' if codex else 'cache_creation_input_tokens'))
    inp = _num(usage.get('input_tokens'))
    out = _num(usage.get('output_tokens'))
    details = usage.get('output_tokens_details') or {}
    reason = _num(usage.get('reasoning_output_tokens') if codex else details.get('thinking_tokens'))
    # Codex input/output totals include their cached/reasoning subsets; Claude's
    # input excludes both cache categories, while output includes thinking.
    return dict(provider=provider, model=model or 'unknown',
                input_tokens=max(0, inp - read - write) if codex else inp,
                cache_read_tokens=read, cache_write_tokens=write,
                output_tokens=max(0, out - reason), reasoning_tokens=reason,
                requests=1, source=_hash(key))


def _roots(project_root, repo):
    # These are reconstruction dependencies/tools, not arbitrary sibling projects.
    dependencies = ('gemini-package', 'local-model-package', 'UnrealEngine', 'UnrealEngine4260',
                    'FModel', 'ghidra-projects', 'ghidra-install', 'Fortnite-23.20-CL-23659353-SDK-Dump')
    roots = [os.path.abspath(repo), *(os.path.join(os.path.abspath(project_root), name) for name in dependencies)]
    try:
        output = subprocess.run(['git', '-C', repo, 'worktree', 'list', '--porcelain'],
                                check=True, capture_output=True, text=True, timeout=10).stdout
        roots.extend(line[9:] for line in output.splitlines() if line.startswith('worktree '))
    except (OSError, subprocess.SubprocessError):
        pass
    return roots


def _in_scope(path, roots, project_root):
    if not isinstance(path, str) or not path.startswith('/'):
        return False
    path = os.path.normpath(path)
    # The separate research checkout is not the 12.60 reconstruction.
    research = os.path.join(os.path.abspath(project_root), 'Fortnite14Research')
    if path == research or path.startswith(research + os.sep):
        return False
    if path == os.path.abspath(project_root):
        return True
    return any(path == root or path.startswith(root + os.sep) for root in roots)


def _jsonl(path, coverage):
    try:
        with path.open(encoding='utf-8') as stream:
            for line in stream:
                try:
                    obj = json.loads(line)
                    if isinstance(obj, dict):
                        yield obj
                except (ValueError, UnicodeError):
                    coverage['malformed_lines'] += 1
    except (OSError, UnicodeError):
        coverage['unreadable_files'] += 1


def _codex(root, roots, project_root):
    cov = dict(files=0, scoped_files=0, usage_records=0, missing_usage_files=0,
               malformed_lines=0, unreadable_files=0, fallback_files=0, unaccounted_events=0,
               limitations=['Local session logs only; cumulative fallback cannot identify every request.'])
    grouped, fallback, direct_totals = {}, [], {}
    for path in sorted(root.rglob('*.jsonl')) if root.exists() else ():
        cov['files'] += 1
        in_scope = ever_scoped = False
        model = 'unknown'
        direct, events = [], []
        ambiguous_usage = set()
        for position, obj in enumerate(_jsonl(path, cov)):
            typ, payload = obj.get('type'), obj.get('payload') or {}
            if not isinstance(payload, dict):
                continue
            if typ in ('session_meta', 'turn_context'):
                if isinstance(payload.get('cwd'), str):
                    in_scope = _in_scope(payload['cwd'], roots, project_root)
                    ever_scoped |= in_scope
                if typ == 'turn_context':
                    model = payload.get('model') or 'unknown'
            elif typ == 'token_usage_record' and isinstance(payload.get('usage'), dict):
                if in_scope:
                    direct.append((payload, model, obj.get('timestamp'), position))
            elif typ == 'event_msg' and payload.get('type') == 'token_count':
                info = payload.get('info') or {}
                if isinstance(info, dict) and isinstance(info.get('total_token_usage'), dict):
                    events.append((obj.get('timestamp'), info['total_token_usage'], model, in_scope, position))
        if not ever_scoped:
            continue
        cov['scoped_files'] += 1
        if direct:
            for payload, model, timestamp, position in direct:
                key = payload.get('response_id')
                if not key:
                    cov.setdefault('missing_response_ids', 0); cov['missing_response_ids'] += 1
                    continue
                total = payload.get('thread_token_usage')
                key = 'codex:' + str(key)
                if isinstance(total, dict):
                    signature = tuple(_num(total.get(k)) for k in ('input_tokens', 'cached_input_tokens', 'cache_write_input_tokens', 'output_tokens', 'reasoning_output_tokens'))
                    direct_totals.setdefault(signature, set()).add(key)
                else:
                    ambiguous_usage.add(tuple(_num(payload['usage'].get(k)) for k in ('input_tokens', 'cached_input_tokens', 'cache_write_input_tokens', 'output_tokens', 'reasoning_output_tokens')))
                rec = _record('openai', model, payload['usage'], key, codex=True)
                rec.update(timestamp=timestamp, client='codex')
                if key not in grouped:
                    grouped[key] = rec
                else:
                    prior = grouped[key]
                    generated = max(prior['output_tokens'] + prior['reasoning_tokens'], rec['output_tokens'] + rec['reasoning_tokens'])
                    for field in FIELDS:
                        prior[field] = max(prior[field], rec[field])
                    prior['output_tokens'] = max(0, generated - prior['reasoning_tokens'])
                    if grouped[key]['model'] == 'unknown' and model != 'unknown':
                        grouped[key]['model'] = model
        if events:
            cov['fallback_files'] += 1
            prev, prev_pos = {}, -1
            fields = ('input_tokens', 'cached_input_tokens', 'cache_write_input_tokens', 'output_tokens', 'reasoning_output_tokens')
            for timestamp, total, model, scoped, position in events:
                if _num(total.get('input_tokens')) < _num(prev.get('input_tokens')):
                    prev = {}
                delta = {k: max(0, _num(total.get(k)) - _num(prev.get(k))) for k in fields}
                prev = total
                interval = {}
                ambiguous = False
                for payload, _, _, dpos in direct:
                    if not prev_pos < dpos < position:
                        continue
                    did = payload.get('response_id')
                    if not did:
                        continue
                    if not isinstance(payload.get('thread_token_usage'), dict):
                        ambiguous = True
                        continue
                    prior = interval.setdefault(str(did), {})
                    for k in fields:
                        prior[k] = max(prior.get(k, 0), _num(payload['usage'].get(k)))
                prev_pos = position
                if not scoped or not any(delta.values()):
                    continue
                total_key = json.dumps(total, sort_keys=True)
                key = 'codex-event:' + str(timestamp) + ':' + total_key
                used = {k: sum(u.get(k, 0) for u in interval.values()) for k in fields}
                if ambiguous or any(used[k] > delta[k] for k in fields):
                    cov['unaccounted_events'] += 1
                    continue
                residual = {k: delta[k] - used[k] for k in fields}
                rec = _record('openai', model, residual, key, codex=True)
                rec.update(requests=0, timestamp=timestamp, client='codex')
                signature = tuple(_num(total.get(k)) for k in fields)
                fallback.append((key, signature, rec, list(interval)))
        if not direct and not events:
            cov['missing_usage_files'] += 1
    seen = set()
    records = list(grouped.values())
    for key, signature, rec, interval in fallback:
        if key in seen:
            continue
        seen.add(key)
        if any(_num(rec.get(field)) for field in FIELDS):
            # With no direct observations in this file a matching cumulative
            # total may belong to a fork, but cannot prove the full interval.
            # Report the ambiguity instead of claiming overlapping usage.
            matches = direct_totals.get(signature, set())
            if not interval and matches:
                cov['unaccounted_events'] += 1
            else:
                records.append(rec)
        elif len(interval) == 1:
            direct_rec = grouped.get('codex:' + interval[0])
            if direct_rec is not None:
                direct_rec.setdefault('aliases', []).append(rec['source'])
    cov['usage_records'] = len(records)
    return records, cov


def _claude(root, roots, project_root):
    cov = dict(files=0, scoped_files=0, usage_records=0, missing_usage_files=0,
               malformed_lines=0, unreadable_files=0, missing_message_ids=0,
               limitations=['Local session logs only; calls without usage or stable message IDs are excluded.'])
    grouped = {}
    for path in sorted(root.rglob('*.jsonl')) if root.exists() else ():
        cov['files'] += 1
        scoped = False
        in_scope = False
        entries = []
        for obj in _jsonl(path, cov):
            if isinstance(obj.get('cwd'), str):
                in_scope = _in_scope(obj['cwd'], roots, project_root)
                scoped |= in_scope
            msg = obj.get('message')
            if in_scope and obj.get('type') == 'assistant' and isinstance(msg, dict) and isinstance(msg.get('usage'), dict):
                entries.append((obj, msg))
        if not scoped:
            continue
        cov['scoped_files'] += 1
        if not entries:
            cov['missing_usage_files'] += 1
        for obj, msg in entries:
            key = msg.get('id') or obj.get('requestId')
            if not key:
                cov['missing_message_ids'] += 1
                continue
            key = 'claude:' + str(key)
            rec = _record('anthropic', msg.get('model') or obj.get('requestedModel'), msg['usage'], key)
            rec['timestamp'] = obj.get('timestamp')
            rec['client'] = 'claude'
            if key not in grouped:
                grouped[key] = rec
            else:
                # Streaming snapshots repeat the same message with progressively
                # updated counts. Preserve the maximum for each token category.
                old = grouped[key]
                generated = max(old['output_tokens'] + old['reasoning_tokens'], rec['output_tokens'] + rec['reasoning_tokens'])
                for field in FIELDS:
                    old[field] = max(old[field], rec[field])
                old['output_tokens'] = max(0, generated - old['reasoning_tokens'])
                if old['model'] == 'unknown' and rec['model'] != 'unknown':
                    old['model'] = rec['model']
    records = list(grouped.values())
    cov['usage_records'] = len(records)
    return records, cov


def _opencode(db, roots, project_root):
    cov = dict(files=int(db.exists()), scoped_files=0, usage_records=0,
               missing_usage_files=0, unreadable_files=0,
               limitations=['OpenCode normalizes stored output to exclude reasoning and input to exclude cache; older versions may differ.',
                            'Only locally retained OpenCode messages are included; direct fnrunner/llama-server calls do not retain usage here.'])
    records = []
    if not db.exists():
        return records, cov
    try:
        with sqlite3.connect('file:' + str(db) + '?mode=ro', uri=True) as conn:
            rows = conn.execute('SELECT s.id, s.directory, m.id, m.data FROM session_v2 s JOIN session_message m ON m.session_id=s.id')
            sessions, with_usage = set(), set()
            for sid, directory, mid, data in rows:
                if not _in_scope(directory, roots, project_root):
                    continue
                sessions.add(sid)
                try:
                    obj = json.loads(data)
                except (TypeError, ValueError):
                    continue
                tokens = obj.get('tokens')
                if not isinstance(tokens, dict):
                    continue
                with_usage.add(sid)
                model = obj.get('model') or {}
                if not isinstance(model, dict):
                    model = {}
                cache = tokens.get('cache') or {}
                if not isinstance(cache, dict):
                    cache = {}
                provider_id = str(model.get('providerID') or 'unknown').lower()
                provider = {'llamacpp': 'local', 'local': 'local', 'openai': 'openai', 'anthropic': 'anthropic', 'google': 'google'}.get(provider_id, 'unknown')
                created = (obj.get('time') or {}).get('created') if isinstance(obj.get('time'), dict) else None
                try:
                    timestamp = datetime.fromtimestamp(created / 1000, timezone.utc).isoformat() if isinstance(created, (int, float)) else None
                except (ValueError, OverflowError, OSError):
                    timestamp = None
                records.append(dict(provider=provider, client='opencode', provider_detail=provider_id, timestamp=timestamp,
                                    model=model.get('id') or 'unknown', input_tokens=_num(tokens.get('input')),
                                    cache_read_tokens=_num(cache.get('read')), cache_write_tokens=_num(cache.get('write')),
                                    output_tokens=_num(tokens.get('output')), reasoning_tokens=_num(tokens.get('reasoning')),
                                    requests=1, source=_hash('opencode:' + mid)))
            cov['scoped_files'] = len(sessions)
            cov['missing_usage_files'] = len(sessions - with_usage)
    except sqlite3.Error:
        cov['unreadable_files'] += 1
    cov['usage_records'] = len(records)
    return records, cov


def _gemini(root, roots, project_root):
    """Read native Gemini CLI session JSON with explicitly scoped directories."""
    cov = dict(files=0, scoped_files=0, usage_records=0, missing_usage_files=0,
               unscoped_files=0, unreadable_files=0, unaccounted_tool_tokens=0,
               limitations=['No native Gemini session files found means usage is unavailable, not measured zero; sessions without attributable project roots are excluded.',
                            'JSONL session format and unrecorded calls are not included; tool token overlap is unverified.'])
    records = {}
    for path in sorted(root.glob('tmp/*/chats/*.json')) if root.exists() else ():
        cov['files'] += 1
        try:
            with path.open(encoding='utf-8') as stream:
                obj = json.load(stream)
        except (OSError, UnicodeError, ValueError):
            cov['unreadable_files'] += 1
            continue
        if not isinstance(obj, dict):
            cov['unreadable_files'] += 1
            continue
        directories = obj.get('directories') or []
        try:
            project_path = (path.parent.parent / '.project_root').read_text(encoding='utf-8').strip()
            if isinstance(directories, list):
                directories = directories + [project_path]
        except (OSError, UnicodeError):
            pass
        project_hashes = {hashlib.sha256(os.path.abspath(r).encode()).hexdigest() for r in roots}
        if isinstance(obj.get('cwd'), str):
            directories = list(directories) + [obj['cwd']] if isinstance(directories, list) else [obj['cwd']]
        scoped_dirs = isinstance(directories, list) and any(_in_scope(d, roots, project_root) for d in directories)
        if not scoped_dirs and obj.get('projectHash') not in project_hashes:
            cov['unscoped_files'] += 1
            continue
        cov['scoped_files'] += 1
        found = False
        for msg in obj.get('messages', []):
            if not isinstance(msg, dict) or msg.get('type') != 'gemini' or not isinstance(msg.get('tokens'), dict):
                continue
            token, mid = msg['tokens'], msg.get('id')
            if not mid:
                continue
            found = True
            key = 'gemini:' + str(mid)
            inp, cached = _num(token.get('input')), _num(token.get('cached'))
            cov['unaccounted_tool_tokens'] += _num(token.get('tool'))
            rec = dict(provider='google', client='gemini', model=msg.get('model') or 'unknown',
                       input_tokens=max(0, inp - cached), cache_read_tokens=cached, cache_write_tokens=0,
                       output_tokens=_num(token.get('output')), reasoning_tokens=_num(token.get('thoughts')),
                       requests=1, source=_hash(key), timestamp=msg.get('timestamp'))
            if key not in records:
                records[key] = rec
            else:
                for field in FIELDS:
                    records[key][field] = max(records[key][field], rec[field])
        if not found:
            cov['missing_usage_files'] += 1
    cov['usage_records'] = len(records)
    return list(records.values()), cov


def collect_usage(*, codex_root=None, claude_root=None, opencode_db=None, gemini_root=None, antigravity_root=None, project_root='/opt/fortnite', repo=REPO):
    """Return normalized usage records and coverage; never return prompt text."""
    roots = _roots(project_root, repo)
    codex, codex_cov = _codex(Path(codex_root or Path.home() / '.codex/sessions'), roots, project_root)
    claude, claude_cov = _claude(Path(claude_root or Path.home() / '.claude/projects'), roots, project_root)
    opencode, opencode_cov = _opencode(Path(opencode_db or Path.home() / '.local/share/opencode/opencode.db'), roots, project_root)
    gemini, gemini_cov = _gemini(Path(gemini_root or Path.home() / '.gemini'), roots, project_root)
    from water_antigravity import collect_antigravity
    antigravity, antigravity_cov = collect_antigravity(Path(antigravity_root or Path.home() / '.gemini/antigravity-cli'), roots, project_root)
    coverage = {'codex': codex_cov, 'claude': claude_cov, 'opencode': opencode_cov,
                'antigravity': antigravity_cov,
                'gemini': gemini_cov}
    records = []
    for rec in codex + claude + opencode + gemini + antigravity:
        if not any(_num(rec.get(field)) for field in FIELDS):
            cov = coverage[rec['client']]
            cov['excluded_zero_records'] = cov.get('excluded_zero_records', 0) + 1
            cov['usage_records'] -= 1
        else:
            records.append(rec)
    return records, coverage
