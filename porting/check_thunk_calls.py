#!/usr/bin/env python3
"""Cross-check each final.json body choice against the reflected signature (read-only).

For every group B/C row, find the call(s) to the chosen body in the thunk decompile, count arguments and compare
with the reflection: expected = (0 if static else 1) + number of non-ReturnValue params. Also check that the
result is stored when the function returns a value. Rows with several candidate callees get the one whose
arg count matches (else `ambiguous`). Output generated/thunk-table/checks.json + checks_summary.json.
Run from repo root: python3 -I porting/check_thunk_calls.py
"""
import collections, json, re

R = 'porting/generated'
nf = json.load(open(f'{R}/reflection/native_funcs.json'))
table = {(r['cls'], r['fn']): r for r in json.load(open(f'{R}/thunk-table/table.json'))}
final = json.load(open(f'{R}/thunk-table/final.json'))


def split_args(s, i):
    """s[i] is just after '('. -> list of top-level args, or None if unbalanced."""
    depth, cur, args = 1, '', []
    while i < len(s):
        ch = s[i]
        if ch in '([':
            depth += 1
        elif ch in ')]':
            depth -= 1
            if depth == 0:
                if cur.strip():
                    args.append(cur.strip())
                return args
        if ch == ',' and depth == 1:
            args.append(cur.strip())
            cur = ''
        else:
            cur += ch
        i += 1
    return None


def calls_to(text, addr):
    name = 'FUN_' + addr[2:]
    out = []
    for m in re.finditer(r'\b' + name + r'\(', text):
        a = split_args(text, m.end())
        if a is not None:
            out.append(len(a))
    return out


def virt_calls(text):
    out = []
    for m in re.finditer(r'\(\*\*\(code \*\*\)\([^;]*?\)\)\(', text):
        a = split_args(text, m.end())
        if a is not None:
            out.append(len(a))
    return out


res, st = [], collections.Counter()
for e in final:
    if e['group'] not in ('B', 'C'):
        continue
    d = nf[e['cls']][e['fn']]
    fl = d.get('flags')
    flags = fl if isinstance(fl, int) else int(fl, 16) if fl else 0
    params = d.get('params') or []
    nret = sum(1 for p in params if p['name'] == 'ReturnValue')
    expected = (0 if flags & 0x2000 else 1) + len(params) - nret
    t = open(table[(e['cls'], e['fn'])]['src'], errors='ignore').read().split('*/', 1)[-1]
    row = dict(cls=e['cls'], fn=e['fn'], group=e['group'], expected_args=expected, has_return=bool(nret))
    if e['group'] == 'B':
        per = {a: calls_to(t, a) for a in e['callees']}
        row['arg_counts'] = per
        match = [a for a, cs in per.items() if expected in cs]
    else:
        v = virt_calls(t)
        row['arg_counts'] = {'virtual': v}
        match = e['targets'] if expected in v else []
    row['matching_callees'] = match
    # a returned value should be stored through the result pointer (param_3 for exec thunks)
    row['result_stored'] = bool(re.search(r'\*\(?[\w ]*\*?\)?\s*param_3\s*=|\*param_3\s*=|param_3\[', t)) or not nret
    n = len(e.get('callees') or e.get('targets') or [])
    if match and len(match) == 1:
        row['verdict'] = 'ok' if row['result_stored'] else 'ok_args_result_unclear'
    elif match:
        row['verdict'] = 'ambiguous_multiple_match'
    elif any(row['arg_counts'].values()):
        row['verdict'] = 'argcount_mismatch'
    else:
        row['verdict'] = 'call_not_found_in_thunk'
    if n == 1 and row['verdict'] == 'ambiguous_multiple_match':
        row['verdict'] = 'ok'
    st[(e['group'], row['verdict'])] += 1
    res.append(row)
json.dump(res, open(f'{R}/thunk-table/checks.json', 'w'))
summ = {f'{g}/{v}': n for (g, v), n in sorted(st.items())}
json.dump(summ, open(f'{R}/thunk-table/checks_summary.json', 'w'), indent=1)
print(summ)
