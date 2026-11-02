#!/usr/bin/env python3
"""Validate the engine-backed CommonTextBlock SetStyle diagnostic transcript."""
import argparse
import json
from pathlib import Path

MARKER = 'TEXTSTYLE'
EVENTS = ('begin', 'context', 'defaults', 'metadata', 'direct_shadow_on_no_slate', 'processevent_shadow_on', 'repeated_shadow_off', 'null_style', 'missing_style_cdo', 'wrong_class_storage', 'wrong_class_setter', 'raw_empty_with_delegate', 'raw_nonempty', 'whitespace_nonempty', 'collapse_disabled', 'rebuilt_parent_and_order', 'wrong_class_parent_slate', 'null_style_parent_slate', 'bound_slate_raw_collapse', 'font_layout_updates', 'original_asset', 'fixture_restored', 'complete')
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
    assets = [line.split('TEXT_BLOCK_STYLE_ASSET style=', 1)[1] for line in text.splitlines() if 'TEXT_BLOCK_STYLE_ASSET style=' in line]
    if assets != ['/Game/UI/Foundation/Buttons/TextStyles/TextStyle-Button-Primary-M.TextStyle-Button-Primary-M_C']:
        raise ValueError('original serialized text style observation missing or duplicated')
    return {'passed': True, 'rows': count, 'serialized_style': assets[0],
            'scope': 'CommonTextBlock SetStyle, direct field/visibility/parent order and actual Slate text/font geometry; no full text lifecycle or UI parity'}


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
