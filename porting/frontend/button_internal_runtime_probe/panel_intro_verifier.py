#!/usr/bin/env python3
"""Strictly verify one complete diagnostic transcript, including every row and state."""
import argparse
import json
from pathlib import Path
import re

PATTERN = re.compile(r'PANELINTRO v=1 seq=([0-9]+) case=([a-z_]+) event=([a-z_]+) state=(-?[0-9]+) visibility=(-?[0-9]+) ok=([01])\s*$')

def expected():
    rows = [('driver','begin',-1,-1),('driver','subsystems',-1,-1)]
    def add(case, event, state, visibility): rows.append((case,event,state,visibility))
    for case in ('default','deferred','reentry'):
        add(case,'initial',0,4)
        if case == 'default':
            for event in ('defaults','metadata_begin','metadata_end','metadata_input'): add(case,event,0,4)
        add(case,'begin_intro',0,2)
        if case == 'deferred': add(case,'pending',0,2)
        add(case,'completion',0,2)
        if case == 'reentry': add(case,'completion',0,2)
        add(case,'pre',1,2)
        for event in ('base_bp','base_native','base_dynamic','panel_dynamic','post','active'): add(case,event,1,3)
        if case == 'default':
            for event in ('input_native_gamepad','input_bp_gamepad','input_native_other','input_bp_other','completion','already_active'): add(case,event,1,3)
        add(case,'cleanup',1,3)
    rows += [('driver','restored',-1,-1),('driver','complete',-1,-1)]
    return rows

def verify(text):
    """Require the exact ordered successful diagnostic rows."""
    want = expected()
    count = 0
    for line in text.splitlines():
        if 'PANELINTRO' not in line:
            continue
        match = PATTERN.search(line)
        if line.count('PANELINTRO') != 1 or not match or any(len(x) > 32 for x in match.groups()):
            raise ValueError('malformed diagnostic row')
        count += 1
        if count > len(want):
            raise ValueError(f'extra diagnostic row at position {count}')
        seq, case, event, state, visibility, ok = match.groups()
        expected_row = want[count - 1]
        if (seq != str(count) or ok != '1' or
                (case, event, state, visibility) != (expected_row[0], expected_row[1], str(expected_row[2]), str(expected_row[3]))):
            raise ValueError(f'transcript mismatch at position {count}')
    if count != len(want):
        raise ValueError(f'transcript incomplete: expected {len(want)} rows, got {count}')
    return {'passed': True, 'rows': count,
            'scope': 'controlled Panel intro diagnostic with C++ ProcessEvent interception and fixture; not Blueprint VM, authentic UI, or deactivation'}


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
