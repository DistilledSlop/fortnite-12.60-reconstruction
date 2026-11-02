#!/usr/bin/env python3
"""Validate the engine-backed CommonButton base click diagnostic transcript."""
import argparse
import json
from pathlib import Path

MARKER = 'BTCLICK'
EVENTS = ('begin', 'context', 'reverse_mutation_dynamic_order', 'added_binding_next_broadcast', 'bp_synchronous_reentry', 'true_analytics_no_world', 'weak_receivers_and_dynamic_compaction', 'default_analytics_name_suffix', 'false_analytics_skips_world', 'real_world_game_instance', 'real_lookup_begin', 'real_lookup_end', 'true_analytics_real_world', 'complete')
EXPECTED = tuple(f'{MARKER} v=1 seq={i} event={event} ok=1'
                 for i, event in enumerate(EVENTS, 1))


def verify(text):
    """Require every diagnostic row exactly once and in order; allow log prefixes."""
    count = 0
    for line in text.splitlines():
        if MARKER not in line:
            continue
        if line.count(MARKER) != 1 or count >= len(EXPECTED):
            raise ValueError('malformed or extra diagnostic row')
        marker_at = line.index(MARKER)
        if marker_at and (line[marker_at - 1].isalnum() or line[marker_at - 1] == '_'):
            raise ValueError('malformed diagnostic marker')
        row = line[marker_at:]
        if row != EXPECTED[count]:
            raise ValueError(f'transcript mismatch at position {count + 1}')
        count += 1
    if count != len(EXPECTED):
        raise ValueError(f'transcript incomplete: expected {len(EXPECTED)} rows, got {count}')
    return {'passed': True, 'rows': count, 'scope': 'Actual BP/native/dynamic callback ordering, weak receivers, reentry and analytics context gating; discarded subsystem call needs separate debugger evidence; no Initialize or upstream click registration'}


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
