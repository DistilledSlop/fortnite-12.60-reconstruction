#!/usr/bin/env python3
"""Validate the controlled current input type getter transcript."""
import argparse
import json
from pathlib import Path

MARKER = 'INPUTTYPE'
VALUES = (0, 2, 1, 0, 3, 255, 1, 2)
EXPECTED = [f'{MARKER} v=1 seq={i} event={event} ok=1'
            for i, event in enumerate(('begin', 'context', 'metadata'), 1)]
EXPECTED += [f'{MARKER} v=1 seq={i + 4} event=state index={i} value={value} '
             f'direct={value} reflected={value} ok=1' for i, value in enumerate(VALUES)]
EXPECTED += [f'{MARKER} v=1 seq=12 event=restored ok=1',
             f'{MARKER} v=1 seq=13 event=complete ok=1']


def verify(text):
    """Require the exact successful rows in order, allowing engine log prefixes."""
    count = 0
    for line in text.splitlines():
        if MARKER not in line:
            continue
        if line.count(MARKER) != 1 or count >= len(EXPECTED):
            raise ValueError('malformed or extra diagnostic row')
        row = line[line.index(MARKER):]
        if row != EXPECTED[count]:
            raise ValueError(f'transcript mismatch at position {count + 1}')
        count += 1
    if count != len(EXPECTED):
        raise ValueError(f'transcript incomplete: expected {len(EXPECTED)} rows, got {count}')
    return {'passed': True, 'rows': count,
            'scope': 'current input type getter metadata and controlled LocalPlayerSubsystem instance only'}


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
