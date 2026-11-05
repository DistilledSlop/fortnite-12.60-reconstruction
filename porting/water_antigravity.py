"""Read Antigravity's locally retained usage metadata without reading prompts.

Schema verified against installed agy CLI SHA-256
b9043860a06a3bfa70b68f06c3d4527abc16207767f14ae9d1e06e3026081862.
CortexStepMetadata: created_at=1, model_usage=9, model_info=24;
CortexStepGeneratorMetadata.chat_model=1; ChatModelMetadata.usage=4 and
response_model_full=22; ModelUsageStats input=2, output=3, cache_write=4,
cache_read=5, provider=6, message_id=7, thinking=9, response_output=10,
response_id=11, provider_assigned_message_id=12. Unknown fields are skipped;
no compiled proprietary descriptors or prompts are included. Counts are local
coverage estimates, not an API bill.
"""
from __future__ import annotations

from contextlib import closing
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import sqlite3
from urllib.parse import quote, unquote, urlsplit

from water_collectors import _in_scope

_FIELDS = ('input_tokens', 'cache_read_tokens', 'cache_write_tokens', 'output_tokens', 'reasoning_tokens')
# APIProvider enum values from the locally installed Antigravity CLI schema.
_PROVIDERS = {3: 'google', 24: 'google', 26: 'anthropic', 30: 'google', 31: 'openai'}


def _wire(blob):
    """Decode protobuf scalars and length-delimited fields; ignore unknowns."""
    if not isinstance(blob, bytes):
        raise ValueError('missing protobuf bytes')
    pos, size, fields = 0, len(blob), {}

    def varint():
        nonlocal pos
        value = 0
        for shift in range(0, 70, 7):
            if pos >= size:
                raise ValueError('truncated varint')
            byte = blob[pos]
            pos += 1
            value |= (byte & 127) << shift
            if byte < 128:
                return value
        raise ValueError('oversized varint')

    while pos < size:
        key = varint()
        field, typ = key >> 3, key & 7
        if not field:
            raise ValueError('invalid field')
        if typ == 0:
            value = varint()
        elif typ == 2:
            count = varint()
            if count > size - pos:
                raise ValueError('truncated field')
            value = blob[pos:pos + count]
            pos += count
        elif typ in (1, 5):
            count = 8 if typ == 1 else 4
            if count > size - pos:
                raise ValueError('truncated fixed field')
            pos += count
            continue
        else:
            raise ValueError('unsupported wire type')
        fields[field] = value
    return fields


def _text(value):
    if not isinstance(value, bytes):
        return ''
    try:
        return value.decode('utf-8')
    except UnicodeDecodeError:
        return ''


def _value(fields, key):
    value = fields.get(key, 0)
    return value if isinstance(value, int) and value >= 0 else 0


def _timestamp(value):
    try:
        stamp = _wire(value)
        seconds, nanos = _value(stamp, 1), _value(stamp, 2)
        if not seconds or nanos >= 1_000_000_000:
            return None
        return datetime.fromtimestamp(seconds, timezone.utc).isoformat().replace('+00:00', 'Z')
    except (ValueError, TypeError, OverflowError, OSError):
        return None


def _provider(usage, model):
    name = model.lower()
    if 'claude' in name:
        return 'anthropic'
    if 'gemini' in name:
        return 'google'
    if 'gpt' in name or name.startswith('o1') or name.startswith('o3'):
        return 'openai'
    return _PROVIDERS.get(_value(usage, 6), 'unknown')


def _entry(usage, model, fallback):
    provider = _provider(usage, model)
    assigned, response, message = (_text(usage.get(k)) for k in (12, 11, 7))
    # Same source keys as the corresponding native collector, where possible.
    if provider == 'anthropic' and assigned:
        key = 'claude:' + assigned
    elif provider == 'openai' and response:
        key = 'codex:' + response
    else:
        key = 'antigravity:' + (response or assigned or message or fallback)
    inp, out, write, read, reason = (_value(usage, k) for k in (2, 3, 4, 5, 9))
    # Antigravity's internal usage is not the raw Google API response. In the
    # inspected Google records, cache reads frequently exceed input, so retain
    # the reported input and cache fields separately rather than subtracting.
    # Their exact normalization for other providers has not been verified.
    reason = min(reason, out)
    return dict(provider=provider, client='antigravity', model=model or 'unknown',
                input_tokens=inp, cache_read_tokens=read, cache_write_tokens=write,
                output_tokens=out - reason, reasoning_tokens=reason, requests=1,
                source=hashlib.sha256(key.encode()).hexdigest()[:24])


def _open(path):
    return sqlite3.connect('file:' + quote(str(path), safe='/') + '?mode=ro', uri=True)


def _scoped(uris, roots, project_root):
    try:
        values = json.loads(uris)
    except (TypeError, ValueError):
        return False
    if not isinstance(values, list):
        return False
    for uri in values:
        if not isinstance(uri, str):
            continue
        parsed = urlsplit(uri)
        if parsed.scheme == 'file' and parsed.netloc in ('', 'localhost'):
            if _in_scope(unquote(parsed.path), roots, project_root):
                return True
    return False


def collect_antigravity(root, roots, project_root):
    """Return (records, coverage) for project workspaces in local databases."""
    root = Path(root)
    cov = dict(files=0, scoped_files=0, usage_records=0, missing_usage_files=0,
               unreadable_files=0, malformed_metadata=0, missing_ids=0,
               missing_databases=0, google_cache_exceeds_input=0,
               google_output_components_match=0, limitations=[
                   'Only retained local conversations with a project workspace are included; missing databases are unknown, not zero usage.',
                   'Generation-only records lack timestamps; step creation time is not necessarily request time.',
                   'Workspace attribution is conversation-wide; mixed-workspace activity cannot be separated.',
                   'Input and cache fields are treated as separate; their provider-specific semantics are not fully verified.',
                   'Generator-only fallback without an identifier is excluded when step usage exists.',
               ])
    summary = root / 'conversation_summaries.db'
    if not summary.exists():
        return [], cov
    try:
        with closing(_open(summary)) as conn:
            scoped = [cid for cid, uris in conn.execute(
                'SELECT conversation_id, workspace_uris FROM conversation_summaries')
                if isinstance(cid, str) and _scoped(uris, roots, project_root)]
    except sqlite3.Error:
        cov['unreadable_files'] += 1
        return [], cov
    candidates_by_id = {}
    for cid in scoped:
        # Treat IDs as opaque filenames, never allow a summary to escape root.
        if Path(cid).name != cid or cid in ('.', '..'):
            cov['malformed_metadata'] += 1
            continue
        path = root / 'conversations' / (cid + '.db')
        if not path.is_file():
            cov['missing_databases'] += 1
            continue
        cov['files'] += 1
        cov['scoped_files'] += 1
        candidates, step_usage = [], False
        try:
            with closing(_open(path)) as conn:
                for idx, blob in conn.execute('SELECT idx, metadata FROM steps WHERE metadata IS NOT NULL'):
                    try:
                        fields = _wire(blob)
                        if 9 not in fields:
                            continue
                        usage = _wire(fields[9])
                        info = _wire(fields[24]) if 24 in fields else {}
                        model = next((_text(info.get(i)) for i in (12, 8, 20) if _text(info.get(i))), '')
                        candidates.append((usage, model, f'{cid}:step:{idx}', False, _timestamp(fields.get(1))))
                        step_usage = True
                    except (ValueError, TypeError):
                        cov['malformed_metadata'] += 1
                for idx, blob in conn.execute('SELECT idx, data FROM gen_metadata WHERE data IS NOT NULL'):
                    try:
                        gen = _wire(blob)
                        if 1 not in gen:
                            continue
                        chat = _wire(gen[1])
                        if 4 not in chat:
                            continue
                        usage = _wire(chat[4])
                        model = next((_text(chat.get(i)) for i in (22, 19, 21) if _text(chat.get(i))), '')
                        candidates.append((usage, model, f'{cid}:gen:{idx}', True, None))
                    except (ValueError, TypeError):
                        cov['malformed_metadata'] += 1
        except sqlite3.Error:
            cov['unreadable_files'] += 1
            continue
        if not candidates:
            cov['missing_usage_files'] += 1
            continue
        # Collect both copies under response/message ID; enrich blank step names
        # with the model name in generation metadata.
        for usage, model, fallback, is_gen, timestamp in candidates:
            if not any(_text(usage.get(k)) for k in (7, 11, 12)):
                cov['missing_ids'] += 1
                if is_gen and step_usage:
                    continue
            identity = (_text(usage.get(11)) or _text(usage.get(12)) or
                        _text(usage.get(7)) or fallback)
            old = candidates_by_id.get(identity)
            if old is None:
                candidates_by_id[identity] = (usage, model, fallback, timestamp)
            else:
                previous, previous_model, old_fallback, old_timestamp = old
                merged = dict(previous)
                for field in (2, 3, 4, 5, 9, 10):
                    merged[field] = max(_value(previous, field), _value(usage, field))
                for field in (1, 6, 7, 11, 12):
                    if not merged.get(field) and usage.get(field):
                        merged[field] = usage[field]
                candidates_by_id[identity] = (merged, previous_model or model, old_fallback, old_timestamp or timestamp)
    records = []
    for usage, model, fallback, timestamp in candidates_by_id.values():
        rec = _entry(usage, model, fallback)
        if rec['provider'] == 'google':
            cov['google_cache_exceeds_input'] += int(_value(usage, 5) > _value(usage, 2))
            cov['google_output_components_match'] += int(
                _value(usage, 3) == _value(usage, 9) + _value(usage, 10))
        rec['timestamp'] = timestamp
        records.append(rec)
    records = [rec for rec in records if any(rec[f] for f in _FIELDS)]
    cov['usage_records'] = len(records)
    return records, cov
