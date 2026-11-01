#!/usr/bin/env python3
"""Strictly verify the controlled panel outro diagnostic transcript."""
import argparse
import json
from pathlib import Path
import re

PATTERN = re.compile(r'PANELOUTRO v=1 seq=([0-9]{1,32}) case=([a-z_]{1,32}) event=([a-z_]{1,32}) state=(-?[0-9]{1,32}) introed=(-?[0-9]{1,32}) ok=([01])[ \t]*$')
SCOPE = ('controlled panel outro lifecycle and held-action diagnostic with C++ ProcessEvent '
         'interception and fixtures; not Blueprint VM, authentic UI, or full interface routing')


def expected():
    rows = [('driver', 'begin', -1, -1), ('driver', 'subsystems', -1, -1)]
    def add(case, events, state, introed):
        rows.extend((case, event, state, introed) for event in events)
    def notify(case):
        add(case, ['completion'], 1, 0)
        add(case, ['pre', 'base_bp', 'base_native', 'base_dynamic', 'panel_dynamic', 'post'], 0, 0)
    def held():
        add('held', ['invalid_unchanged', 'empty', 'negative', 'zero', 'missing',
                     'progress', 'nan', 'progress', 'swap', 'cleanup'], 0, 0)
    add('default', ['initial'], 0, 0)
    add('default', ['active', 'progress', 'begin_outro'], 1, 1)
    notify('default')
    add('default', ['inactive'], 0, 0)
    add('default', ['reactivated', 'begin_outro'], 1, 1)
    notify('default')
    add('default', ['loop_inactive'], 0, 0)
    held()
    add('deferred', ['initial'], 0, 0)
    add('deferred', ['active', 'progress', 'begin_outro', 'begin_outro', 'pending'], 1, 1)
    notify('deferred')
    add('deferred', ['inactive'], 0, 0)
    held()
    rows.extend([('driver', 'restored', -1, -1), ('driver', 'complete', -1, -1)])
    return rows


def verify(text):
    want = expected()
    count = 0
    for line in text.splitlines():
        if 'PANELOUTRO' not in line:
            continue
        match = PATTERN.search(line)
        if line.count('PANELOUTRO') != 1 or not match:
            raise ValueError('malformed diagnostic row')
        count += 1
        if count > len(want):
            raise ValueError(f'extra diagnostic row at position {count}')
        seq, case, event, state, introed, ok = match.groups()
        expected_row = want[count - 1]
        if (seq != str(count) or ok != '1' or
                (case, event, state, introed) != (expected_row[0], expected_row[1], str(expected_row[2]), str(expected_row[3]))):
            raise ValueError(f'transcript mismatch at position {count}')
    if count != len(want):
        raise ValueError(f'transcript incomplete: expected {len(want)} rows, got {count}')
    return {'passed': True, 'rows': count, 'scope': SCOPE}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('log', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    try:
        result = verify(args.log.read_text(errors='replace'))
    except (ValueError, OSError) as exc:
        result = {'passed': False, 'error': str(exc)[:300]}
    args.output.write_text(json.dumps(result, indent=2) + '\n')
    raise SystemExit(0 if result['passed'] else 1)


if __name__ == '__main__':
    main()
