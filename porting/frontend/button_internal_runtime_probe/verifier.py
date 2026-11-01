#!/usr/bin/env python3
"""Validate the controlled button diagnostic transcript, not its provenance."""

import argparse
import json
import re
from pathlib import Path

ROWS = '''begin defaults prebuild_disabled prebuild_enabled factory size hover_enabled interaction_disabled disabled_mouse_down interaction_enabled button_disabled button_enabled hover_leave enter_down enter_up other_key manager_active native_double dynamic_double double_result dynamic_click click_result dynamic_press dynamic_release native_focus focus_result manager_suspended suspended_double suspended_click manager_restored delegates_cleanup owners_released slate_destroyed complete'''.split()
MARKER = 'BTNINTERNAL'
PATTERN = re.compile(r'BTNINTERNAL v=1 seq=([0-9]+) event=([a-z_]+) ok=([01])\s*$')


def verify(text):
    """Require exactly one complete, successful, ordered diagnostic transcript."""
    actual = []
    for line in text.splitlines():
        if MARKER not in line:
            continue
        match = PATTERN.search(line)
        if line.count(MARKER) != 1 or not match:
            raise ValueError('malformed diagnostic row')
        # Limit integer conversion and error output for untrusted logs.
        if len(match[1]) > 9 or len(match[2]) > 80:
            raise ValueError('malformed diagnostic row')
        actual.append((int(match[1]), match[2], int(match[3])))
        index = len(actual) - 1
        if index >= len(ROWS):
            raise ValueError(f'transcript has extra diagnostic row at position {index + 1}')
        expected = (index + 1, ROWS[index], 1)
        if actual[-1] != expected:
            raise ValueError(f'transcript mismatch at position {index + 1}: expected {expected!r}, got {actual[-1]!r}')
    if len(actual) != len(ROWS):
        raise ValueError(f'transcript incomplete: expected {len(ROWS)} rows, got {len(actual)}')
    return {'passed': True, 'rows': len(actual), 'scope': 'controlled UCommonButtonInternal diagnostic only; provenance and full UI not verified'}


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
