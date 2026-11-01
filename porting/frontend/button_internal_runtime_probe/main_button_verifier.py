#!/usr/bin/env python3
"""Validate the controlled MainButton query transcript, not its provenance."""
import argparse
import json
from pathlib import Path
import re

MARKER = 'MAINBTN'
ROW = re.compile(r'MAINBTN v=1 seq=([0-9]+) event=([a-z_]+)(?: visibility=([0-9]+) bits=([0-9]+) direct=([01]) alias=([01]) reflected=([01]))? ok=([01])\s*$')


def verify(text):
    """Require all 44 exact, successful rows in their prescribed order."""
    count = 0
    for line in text.splitlines():
        if MARKER not in line:
            continue
        match = ROW.search(line)
        if line.count(MARKER) != 1 or not match or any(len(x) > 9 for x in match.groups() if x is not None):
            raise ValueError('malformed diagnostic row')
        count += 1
        if count > 44:
            raise ValueError(f'extra diagnostic row at position {count}')
        if count <= 3:
            expected = (str(count), ('begin', 'defaults', 'metadata')[count - 1], None, None, None, None, None, '1')
        elif count <= 43:
            visibility, bits = divmod(count - 4, 8)
            value = str(int(visibility not in (1, 2) and bits == 7))
            expected = (str(count), 'state', str(visibility), str(bits), value, value, value, '1')
        else:
            expected = ('44', 'complete', None, None, None, None, None, '1')
        if match.groups() != expected:
            raise ValueError(f'transcript mismatch at position {count}')
    if count != 44:
        raise ValueError(f'transcript incomplete: expected 44 rows, got {count}')
    return {'passed': True, 'rows': count, 'scope': 'controlled MainButton query diagnostic only; provenance and full UI not verified'}


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
