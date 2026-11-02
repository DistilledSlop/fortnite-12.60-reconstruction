#!/usr/bin/env python3
"""Validate the engine-backed CommonButton text-style accessor diagnostic transcript."""
import argparse
import json
from pathlib import Path

MARKER = 'BTTEXT'
EVENTS = ('begin', 'context', 'hover_default', 'metadata_selected', 'metadata_style', 'native_state_not_reflected',
          'defaults', 'independent_true', 'independent_flip', 'normal', 'normal_hover',
          'selected', 'selected_hover', 'disabled_precedence', 'disabled_fallback',
          'selected_hover_fallback', 'repeated_hover', 'normal_fallback', 'null_candidates',
          'null_style', 'invalid_style', 'invalid_candidate', 'loaded_asset', 'hover_weak_expired', 'restored', 'complete')
EXPECTED = tuple(f'{MARKER} v=1 seq={i} event={event}' +
                 (' available=1' if event == 'loaded_asset' else '') + ' ok=1'
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
    assets = [line.split('BUTTON_STYLE_ASSET ', 1)[1] for line in text.splitlines() if 'BUTTON_STYLE_ASSET ' in line]
    if len(assets) != 1:
        raise ValueError('missing or duplicate original button style observation')
    values = dict(part.split('=', 1) for part in assets[0].split())
    if values.get('button') != '/Game/UI/Foundation/Buttons/IconTextButton.IconTextButton_C' or any(not values.get(key, '').startswith('/') for key in ('style', 'text')):
        raise ValueError('original serialized style identity missing')
    return {'passed': True, 'rows': count, 'serialized_asset': values,
            'scope': 'CommonButton state/text-style accessors with controlled selected/hover and original serialized style; no production transitions or interactive UI'}


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
