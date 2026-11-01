#!/usr/bin/env python3
"""Strictly verify the common input action row diagnostic transcript."""
import argparse
import json
import re
from pathlib import Path

MARKER = 'COMMON_INPUT_ACTION_ROW_PROBE'
SCOPE = ('C++ and reflected default construction and layout, struct copy capability, '
         'and original InputActionDataTable loading and row-struct identity; '
         'not input routing, UI behavior, asset contents, or parity')
LAYOUT = (f'{MARKER} row=layout_values row_size=1064 row_align=8 display=8 '
          'hold_display=32 keyboard=56 gamepad=224 touch=896 info_size=168 '
          'info_align=8 key=0 state=24 requires_hold=25 hold_time=28 brush=32')
DEFAULTS = ('text', 'keyboard', 'gamepad0', 'gamepad1', 'gamepad2', 'gamepad3', 'touch')
ROWS = ('row_layout', 'info_layout', *(f'cpp_{x}' for x in DEFAULTS),
        'row_struct_ops_copy', 'info_struct_ops_copy', *(f'script_{x}' for x in DEFAULTS),
        'info_script_initialize', 'asset_load', 'asset_row_struct', 'asset_nonempty')


def verify(text):
    lines = []
    for line in text.splitlines():
        if MARKER not in line:
            continue
        if line.count(MARKER) != 1:
            raise ValueError('malformed diagnostic row')
        lines.append(line[line.index(MARKER):].rstrip(' \t'))
    expected = [LAYOUT, *(f'{MARKER} row={row} passed=1' for row in ROWS)]
    if lines[:len(expected)] != expected:
        raise ValueError('missing, failed, malformed, or reordered diagnostic row')
    tail = lines[len(expected):]
    if not tail:
        raise ValueError('missing asset values')
    count_match = re.fullmatch(re.escape(MARKER) + r' row=asset_values row_count=([1-9][0-9]*)', tail[0])
    if not count_match:
        raise ValueError('invalid asset row count')
    count = int(count_match.group(1))
    sample_count = min(5, count)
    if len(tail) != sample_count + 2:
        raise ValueError('missing or extra sample or terminal row')
    names = []
    for index, line in enumerate(tail[1:-1]):
        match = re.fullmatch(re.escape(MARKER) + rf' row=asset_sample index={index} name=([^\x00-\x1f\x7f]+)', line)
        if not match or match.group(1).strip() != match.group(1):
            raise ValueError('malformed asset sample')
        names.append(match.group(1))
    folded = [name.lower() for name in names]
    if any(a >= b for a, b in zip(folded, folded[1:])):
        raise ValueError('asset samples are duplicated or unordered')
    if tail[-1] != f'{MARKER} row=complete passed=1':
        raise ValueError('missing or failed terminal row')
    return {'passed': True, 'rows': len(lines), 'asset_row_count': count,
            'asset_samples': names, 'scope': SCOPE}


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
