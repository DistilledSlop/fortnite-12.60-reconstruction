#!/usr/bin/env python3
"""Strictly verify the confirmation description diagnostic transcript."""
import argparse
import json
from pathlib import Path

MARKER = 'CONFIRMATION_DESCRIPTION_PROBE'
SCOPE = ('actual engine constructor, layout, and delegate copying diagnostic; '
         'not UI, Blueprint execution, or parity')
ROWS = ('layout', 'cpp_defaults', 'cpp_copy_lifetime', 'cpp_assignment_lifetime',
        'struct_ops_copy', 'struct_initialize_defaults', 'copies_bound',
        'copies_keep_owners_alive', 'copied_callbacks_arguments',
        'script_copy_keeps_owners_alive', 'destroy_releases_owners', 'complete')
LAYOUT = (f'{MARKER} row=layout_values size=280 align=8 ordinary=248 advanced=264 '
          'struct_size=280 struct_align=8')


def verify(text):
    expected = (LAYOUT, *(f'{MARKER} row={row} passed=1' for row in ROWS))
    count = 0
    for line in text.splitlines():
        if MARKER not in line:
            continue
        if line.count(MARKER) != 1:
            raise ValueError('malformed diagnostic row')
        count += 1
        if count > len(expected):
            raise ValueError(f'extra diagnostic row at position {count}')
        if line[line.index(MARKER):].rstrip(' \t') != expected[count - 1]:
            raise ValueError(f'transcript mismatch at position {count}')
    if count != len(expected):
        raise ValueError(f'transcript incomplete: expected {len(expected)} rows, got {count}')
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
