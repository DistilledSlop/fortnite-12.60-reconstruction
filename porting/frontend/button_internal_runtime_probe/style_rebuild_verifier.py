#!/usr/bin/env python3
"""Validate the engine-backed CommonButton style rebuild diagnostic transcript."""
import argparse
import json
from pathlib import Path

MARKER = 'BTREBUILD'
EVENTS = ('begin', 'context', 'sound_comparison_detects_legacy_name_and_cache', 'brush_comparison_detects_image_type', 'sound_matrix_00', 'sound_matrix_01', 'sound_matrix_02', 'sound_matrix_03', 'sound_matrix_04', 'sound_matrix_05', 'sound_matrix_06', 'sound_matrix_07', 'sound_matrix_08', 'sound_matrix_09', 'sound_matrix_10', 'sound_matrix_11', 'sound_matrix_12', 'sound_matrix_13', 'sound_matrix_14', 'sound_matrix_15', 'sound_matrix_16', 'sound_matrix_17', 'sound_matrix_18', 'sound_matrix_19', 'sound_matrix_20', 'sound_matrix_21', 'sound_matrix_22', 'sound_matrix_23', 'sound_matrix_24', 'sound_matrix_25', 'sound_matrix_26', 'sound_matrix_27', 'sound_matrix_28', 'sound_matrix_29', 'sound_matrix_30', 'sound_matrix_31', 'sound_matrix_32', 'sound_matrix_33', 'sound_matrix_34', 'sound_matrix_35', 'sound_matrix_36', 'sound_matrix_37', 'sound_matrix_38', 'sound_matrix_39', 'sound_matrix_40', 'sound_matrix_41', 'sound_matrix_42', 'sound_matrix_43', 'sound_matrix_44', 'sound_matrix_45', 'sound_matrix_46', 'sound_matrix_47', 'sound_matrix_48', 'sound_matrix_49', 'sound_matrix_50', 'sound_matrix_51', 'sound_matrix_52', 'sound_matrix_53', 'sound_matrix_54', 'sound_matrix_55', 'sound_matrix_56', 'sound_matrix_57', 'sound_matrix_58', 'sound_matrix_59', 'sound_matrix_60', 'sound_matrix_61', 'sound_matrix_62', 'sound_matrix_63', 'normal_brush_mapping', 'selected_brush_mapping', 'disabled_inherits_normal', 'padding', 'actual_sbox_dimensions', 'actual_reflected_event_font', 'live_style_0', 'live_style_1', 'live_style_2', 'live_style_3', 'legacy_sound_lookup_and_copy_without_comparison_resolution', 'actual_mid_parent_outer', 'single_material_all_brushes', 'single_nonmaterial_no_mid', 'temporary_brush_destruction_after_event_and_dimensions', 'real_shared_release_rereads_single', 'override_presence_cached_before_brush_release', 'event_re_resolves_cdo_and_weak', 'actual_event_synchronous_reentry', 'valid_style_class_cdo_absent_before', 'style_cdo_resolver_does_not_create', 'missing_cdo_rebuild_no_creation_or_effects', 'null_style_no_refresh', 'weak_missing_refresh_has_no_event', 'complete')
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
    return {'passed': True, 'rows': count, 'scope': 'Real style/brush/sound/MID, SBox and native/reflected callback-reentry diagnostic; no setter registration, production lifecycle, pixels/audio or private handle ownership proof'}


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
