#!/usr/bin/env python3
"""Build ground-truth (class, function, exec thunk, true body) pairs for ported natives.

Read-only with respect to the repo: reads behavior/MANIFEST.json, the .inc snippets,
reflection/native_funcs.json, behavior/**/native-evidence*.json and the generated
thunk decompiles. Writes only generated/ground-truth/{pairs,summary}.json.

Body candidates come only from explicit citations:
  - manifest field native_body_address (explicit)
  - manifest notes/note text and native-evidence JSON text (cue-word + address)
  - .inc comment lines (cue-word + address)
Thunk decompiles are NOT a source of truth (they are what thunk analysis is
calibrated against); they only corroborate a cited body when the thunk's
decompile references that body address.
"""
import collections
import glob
import json
import os
import re

PORT = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(PORT)
MANIFEST = os.path.join(PORT, 'behavior', 'MANIFEST.json')
NATIVE_FUNCS = os.path.join(PORT, 'generated', 'reflection', 'native_funcs.json')
DECOMP = os.path.join(PORT, 'generated', 'decomp')
EVIDENCE_GLOB = os.path.join(PORT, 'behavior', '**', 'native-evidence*.json')
OUT_DIR = os.path.join(PORT, 'generated', 'ground-truth')

ADDR_RE = re.compile(r'0x14[0-9a-fA-F]{7}\b')
SNIPPET_MAX = 200
CONF_RANK = {'high': 0, 'medium': 1, 'low': 2}

# Cue words that may precede a citation. 'strong' = names the body/impl;
# 'weak' = bare "native" (needs thunk corroboration); 'reject' = callee of the
# body or another function's original exec, never the true body.
CUES = [
    ('native_body', re.compile(r'native[ _-]*(?:body|impl(?:ementation)?|target)\b', re.I), 'strong'),
    ('original_address', re.compile(r'original\s+address\b', re.I), 'strong'),
    ('wrapper', re.compile(r'\bwrapper\b', re.I), 'strong'),
    ('body', re.compile(r'\bbody\b', re.I), 'strong'),
    ('target', re.compile(r'\btarget\b', re.I), 'strong'),
    ('tail_call', re.compile(r'tail[- ]?(?:call|jump)s?\b', re.I), 'strong'),
    ('native_helper', re.compile(r'native[ _-]*(?:helper|function)\b', re.I), 'strong'),
    ('native', re.compile(r'\bnative\b', re.I), 'weak'),
    ('callee', re.compile(r'\bcallee\b|direct call|\bcalls?\b|\bcall to\b', re.I), 'reject'),
    ('original_exec', re.compile(r'original exec\b|\bexec\b', re.I), 'reject'),
]


def norm(addr):
    return '0x' + addr.lower()[2:]


def cue_for_window(window):
    """Return (tag, strength) of the cue closest to the end of window, or None.

    Any reject cue, or a qualified Class::Function subject, anywhere in the lead-in
    window means the address belongs to another function (callee, exec thunk, or a
    different function's body), so the citation is rejected rather than attributed.
    """
    if '::' in window:
        return ('other_subject', 'reject')
    for tag, rx, strength in CUES:
        if strength == 'reject' and rx.search(window):
            return (tag, 'reject')
    best = None
    for tag, rx, strength in CUES:
        for m in rx.finditer(window):
            if best is None or m.end() >= best[0]:
                best = (m.end(), tag, strength)
    return None if best is None else (best[1], best[2])


def snippet_around(text, start, end):
    s = max(0, start - 90)
    e = min(len(text), end + 60)
    return ' '.join(text[s:e].split())[:SNIPPET_MAX]


def scan_text(text, source, multiline):
    """Yield (addr, tag, strength, snippet) for every cue-qualified 0x14 citation."""
    out = []
    units = text.splitlines() if multiline else [text]
    for unit in units:
        prev_end = 0
        for m in ADDR_RE.finditer(unit):
            window = unit[max(prev_end, m.start() - 60):m.start()]
            prev_end = m.end()
            cue = cue_for_window(window)
            if cue is None:
                continue
            tag, strength = cue
            out.append((norm(m.group(0)), tag, strength,
                        snippet_around(unit, m.start(), m.end()), source))
    return out


def load_json(path, default):
    try:
        with open(path, encoding='utf-8') as fh:
            return json.load(fh)
    except (OSError, ValueError):
        return default


def main():
    manifest = load_json(MANIFEST, [])
    native_funcs = load_json(NATIVE_FUNCS, {})

    exec_map = {}  # (class, function) -> exec_addr from native_funcs
    thunk_owner = {}  # exec_addr -> (class, function)
    for cls, funcs in native_funcs.items():
        if not isinstance(funcs, dict):
            continue
        for fn, info in funcs.items():
            ea = info.get('exec_addr') if isinstance(info, dict) else None
            if ea:
                exec_map[(cls, fn)] = norm(ea)
                thunk_owner.setdefault(norm(ea), (cls, fn))

    ported = [e for e in manifest
              if e.get('status') == 'ported' and e.get('kind') != 'member_field']
    inc_users = collections.Counter(e.get('inc_path') for e in ported)

    cands = collections.defaultdict(list)
    inc_text_cache = {}
    for e in ported:
        key = (e['class'], e['function'])
        nb = e.get('native_body_address')
        if nb and ADDR_RE.fullmatch(str(nb)):
            cands[key].append((norm(nb), 'native_body_address', 'explicit', str(nb),
                               'manifest_field'))
        for field in ('notes', 'note'):
            if e.get(field):
                for c in scan_text(str(e[field]), 'manifest_notes', multiline=False):
                    cands[key].append(c)
        inc = e.get('inc_path')
        if inc:
            path = os.path.join(ROOT, inc)
            if path not in inc_text_cache:
                try:
                    with open(path, encoding='utf-8', errors='replace') as fh:
                        inc_text_cache[path] = fh.read()
                except OSError:
                    inc_text_cache[path] = ''
            text = inc_text_cache[path]
            if text:
                name_ok = re.search(r'\b%s\b' % re.escape(e['function']), text) is not None
                shared_unnamed = inc_users[inc] > 1 and not name_ok
                for c in scan_text(text, 'inc_comment', multiline=True):
                    if shared_unnamed:
                        c = c[:4] + ('shared_inc_no_name',)
                    cands[key].append(c)

    for path in glob.glob(EVIDENCE_GLOB, recursive=True):
        data = load_json(path, {})
        rows = data.get('functions') if isinstance(data, dict) else None
        for row in rows or []:
            if not isinstance(row, dict) or not row.get('class') or not row.get('function'):
                continue
            key = (row['class'], row['function'])
            if row.get('native_body') and ADDR_RE.fullmatch(str(row['native_body'])):
                cands[key].append((norm(row['native_body']), 'native_body', 'explicit',
                                   str(row['native_body']), 'evidence_json'))
            ev = row.get('evidence')
            ev_text = '\n'.join(ev) if isinstance(ev, list) else str(ev or '')
            for c in scan_text(ev_text, 'evidence_json', multiline=True):
                cands[key].append(c)

    stats = collections.Counter()
    pairs = []
    thunk_mismatch = 0
    no_thunk = []
    for e in ported:
        key = (e['class'], e['function'])
        thunk = norm(e['address']) if ADDR_RE.fullmatch(str(e.get('address') or '')) else None
        fallback = exec_map.get(key)
        if thunk and fallback and thunk != fallback:
            thunk_mismatch += 1
        if not thunk:
            thunk = fallback
        if not thunk:
            no_thunk.append(key)
            continue

        # Thunk decompile, used only to corroborate citations.
        thunk_file = os.path.join(DECOMP, e['class'], '%s__%s.c' % (e['function'], thunk[2:]))
        thunk_text = None
        if os.path.isfile(thunk_file):
            with open(thunk_file, encoding='utf-8', errors='replace') as fh:
                thunk_text = fh.read()

        accepted = []
        raw = cands.get(key, [])
        for addr, tag, strength, snippet, source in raw:
            stats['candidates_raw'] += 1
            if addr == thunk:
                stats['rejected_is_thunk'] += 1
                continue
            if source == 'shared_inc_no_name' or tag == 'shared_inc_no_name':
                stats['rejected_shared_inc_no_name'] += 1
                continue
            if strength == 'reject':
                stats['rejected_callee_or_exec_cue'] += 1
                continue
            owner = thunk_owner.get(addr)
            if owner and owner != key:
                stats['rejected_other_function_thunk'] += 1
                continue
            corroborated = None
            if thunk_text is not None:
                corroborated = re.search(r'(?<![0-9a-fA-F])%s(?![0-9a-fA-F])' % addr[4:],
                                         thunk_text, re.I) is not None
            if strength == 'explicit':
                conf = 'high'
            elif strength == 'strong':
                conf = 'high' if corroborated else 'medium'
            else:  # weak
                if not corroborated:
                    stats['rejected_weak_uncorroborated'] += 1
                    continue
                conf = 'medium'
            accepted.append({'addr': addr, 'tag': tag, 'source': source, 'confidence': conf,
                             'corroborated': corroborated, 'snippet': snippet})

        if not accepted:
            continue
        accepted.sort(key=lambda c: (CONF_RANK[c['confidence']],
                                     0 if c['source'] == 'manifest_field' else 1))
        best = accepted[0]
        alts = sorted({c['addr'] for c in accepted} - {best['addr']})
        pairs.append({
            'class': e['class'],
            'function': e['function'],
            'kind': e.get('kind'),
            'exec_thunk_addr': thunk,
            'true_body_addr': best['addr'],
            'confidence': best['confidence'],
            'primary_source': best['source'],
            'sources': sorted({c['source'] for c in accepted}),
            'corroborated_by_thunk_decomp': best['corroborated'],
            'evidence': [{'source': c['source'], 'tag': c['tag'], 'addr': c['addr'],
                          'snippet': c['snippet']} for c in accepted[:3]],
            'alternate_body_addrs': alts,
            'conflict': bool(alts),
        })

    pairs.sort(key=lambda p: (p['class'], p['function']))
    by_conf = collections.Counter(p['confidence'] for p in pairs)
    by_primary = collections.Counter(p['primary_source'] for p in pairs)
    by_any = collections.Counter(s for p in pairs for s in p['sources'])
    corr = collections.Counter(str(p['corroborated_by_thunk_decomp']) for p in pairs)
    samples = pairs[::max(1, len(pairs) // 10)][:10] if pairs else []
    summary = {
        'ported_entries_scanned': len(ported),
        'unique_class_function': len({(e['class'], e['function']) for e in ported}),
        'missing_thunk_skipped': len(no_thunk),
        'thunk_manifest_vs_native_funcs_mismatch': thunk_mismatch,
        'thunk_decomp_files_found': sum(
            1 for e in ported if e.get('address') and os.path.isfile(os.path.join(
                DECOMP, e['class'], '%s__%s.c' % (e['function'], e['address'][2:].lower())))),
        'pairs_found': len(pairs),
        'pairs_by_confidence': dict(by_conf),
        'pairs_by_primary_source': dict(by_primary),
        'pairs_by_any_source': dict(by_any),
        'pairs_with_conflicting_citations': sum(1 for p in pairs if p['conflict']),
        'corroborated_by_thunk_decomp': dict(corr),
        'candidate_rejections': {k: v for k, v in stats.items() if k.startswith('rejected')},
        'candidates_raw': stats['candidates_raw'],
        'sample_pairs': [{k: p[k] for k in ('class', 'function', 'exec_thunk_addr',
                                             'true_body_addr', 'confidence', 'primary_source')}
                         for p in samples],
    }
    os.makedirs(OUT_DIR, exist_ok=True)
    with open(os.path.join(OUT_DIR, 'pairs.json'), 'w', encoding='utf-8') as fh:
        json.dump(pairs, fh, indent=1)
        fh.write('\n')
    with open(os.path.join(OUT_DIR, 'summary.json'), 'w', encoding='utf-8') as fh:
        json.dump(summary, fh, indent=1)
        fh.write('\n')
    print(json.dumps({k: summary[k] for k in ('ported_entries_scanned', 'pairs_found',
                                              'pairs_by_confidence')}))


if __name__ == '__main__':
    main()
