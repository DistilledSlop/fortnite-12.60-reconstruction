#!/usr/bin/env python3
"""Verify the original row receipt plus the CommonInput batch observations."""
import argparse
import json
from pathlib import Path
import common_input_action_row_verifier as original

BATCH = ('batch_platform_initialize', 'batch_settings_cdo_fresh', 'batch_first_match_stops',
         'batch_brush_copy', 'batch_keyset_nonconsuming', 'batch_keyset_exact_count')


def verify(text):
    lines = text.splitlines()
    marked = [i for i, line in enumerate(lines) if original.MARKER in line]
    expected = [f'{original.MARKER} row={row} passed=1' for row in BATCH]
    if len(marked) < len(BATCH) + 1:
        raise ValueError('missing batch or terminal rows')
    positions = marked[-len(BATCH)-1:-1]
    if [lines[i][lines[i].index(original.MARKER):].rstrip(' \t') for i in positions] != expected:
        raise ValueError('missing, failed, malformed, or reordered batch row')
    stripped = '\n'.join(line for i, line in enumerate(lines) if i not in positions)
    result = original.verify(stripped)
    result['rows'] += len(BATCH)
    result['scope'] += ('; reflected platform initialization, settings CDO/fresh config consistency, '
                        'and controlled controller-selection and brush lookup')
    return result


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
