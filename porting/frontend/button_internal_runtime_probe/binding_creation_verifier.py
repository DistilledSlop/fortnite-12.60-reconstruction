#!/usr/bin/env python3
"""Validate the engine-backed action binding creation diagnostic transcript."""
import argparse
import json
from pathlib import Path

MARKER = 'BINDCREATE'
EVENTS = ('begin', 'context', 'settings', 'no_action', 'empty_handler', 'stale_handler',
          'invalid_tag', 'invalid_row', 'independent_validation', 'first_id_zero',
          'duplicate', 'duplicate_changed_fields', 'different_event', 'different_owner',
          'tag_and_row', 'legacy_duplicate', 'legacy_data', 'remove_recreate',
          'no_callbacks', 'owned_callback', 'cleanup', 'restored', 'complete')
EXPECTED = tuple(f'{MARKER} v=1 seq={i} event={event} ok=1'
                 for i, event in enumerate(EVENTS, 1))


OWNER = 'BindingCreateOwner'
TAG = 'Probe.Binding.Create'
EMPTY_HANDLER = f'Cannot bind widget [{OWNER}] to action [{TAG}] - empty handler delegate.'
MISSING_SUFFIX = ' - provided tag does not map to an existing UI input action. It can be added under Project Settings->UI Input.'
DUPLICATE_SUFFIX = '[Pressed]! A given widget can only bind the same action once. Unregister existing binding first if you wish to change any aspect of the binding'
DIAGNOSTICS = (
    f'Cannot create action binding for widget [{OWNER}] - no action provided.',
    EMPTY_HANDLER, EMPTY_HANDLER,
    f'Cannot bind widget [{OWNER}] to action [Probe.Binding.Missing]' + MISSING_SUFFIX,
    f'Cannot bind widget [{OWNER}] to action [Legacy_BindingCreateTable_Missing] - provided legacy data table row does not resolve to valid data.',
    f'Cannot bind widget [{OWNER}] to action [Legacy_BindingCreateTable_Missing]' + MISSING_SUFFIX,
    f'Widget [{OWNER}] is already bound to action [{TAG}] ' + DUPLICATE_SUFFIX,
    f'Widget [{OWNER}] is already bound to action [{TAG}] ' + DUPLICATE_SUFFIX,
    f'Widget [{OWNER}] is already bound to action [Legacy_BindingCreateTable_Valid] ' + DUPLICATE_SUFFIX,
)


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
    prefix = 'LogUIActionRouter: Error: '
    diagnostics = tuple(line.split(prefix, 1)[1] for line in text.splitlines() if prefix in line)
    if diagnostics != DIAGNOSTICS:
        raise ValueError('creation diagnostic category, severity, messages or order differ')
    warning = "UDataTable::FindRow : '' requested row 'Missing' not in DataTable"
    if text.count(warning) != 1 or text.index(warning) > text.index(DIAGNOSTICS[4]):
        raise ValueError('independent row validation or table-before-entry logging differs')
    return {'passed': True, 'rows': count, 'error_diagnostics': len(diagnostics),
            'scope': 'engine-backed binding creation validation, owned registry insertion and exact diagnostics; no router association or input dispatch'}


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
