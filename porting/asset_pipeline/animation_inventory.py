#!/usr/bin/env python3
"""Summarize the --scan-export-classes TSV into the tracked animation_inventory.json (stdlib, deterministic).

Counts packages per animation-related export class (a package counts once per class it exports)
and lists every package with an AnimSequence export, which is the export_anims.py work list and the
animation_sequence_packages denominator read by progress/refresh_totals.py.

  python3 -I porting/asset_pipeline/animation_inventory.py [--classes <tsv>] [--print]
"""
import argparse
import json
import os
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
DEFAULT_CLASSES = Path('/mnt/vibrato/fortnite-asset-pipeline/export-classes.tsv')
OUT = HERE / 'animation_inventory.json'
# Export class names counted as animation-related (exact names and prefixes).
ANIM_CLASS = re.compile(r'^(AnimSequence|AnimMontage|AnimComposite|AnimStreamable|AnimBlueprint|AnimBlueprintGeneratedClass'
                        r'|BlendSpace.*|AimOffset.*|PoseAsset|Skeleton|ControlRig.*|Rig|RigHierarchy.*|MirrorDataTable'
                        r'|AnimCurveCompressionSettings|AnimBoneCompressionSettings|AnimSet|CameraAnim'
                        r'|LevelSequence|PhysicsAsset|SkeletalMesh)$')


def summarize(tsv):
    histogram, sequences, packages = {}, [], 0
    with open(tsv, encoding='utf-8') as fh:
        for line in fh:
            name, _, classes = line.rstrip('\n').partition('\t')
            packages += 1
            classes = set(classes.split(',')) if classes else set()
            for cls in classes:
                if ANIM_CLASS.match(cls):
                    histogram[cls] = histogram.get(cls, 0) + 1
            if 'AnimSequence' in classes:
                sequences.append(name)
    sequences.sort()
    return {
        'source': 'Cue4Inspect --scan-export-classes over /mnt/vibrato/fortnite-extracted-full (export-table class '
                  'names only; exports not deserialized). A package counts once per class among its exports.',
        'scanned_packages': packages,
        'class_package_counts': dict(sorted(histogram.items())),
        'animation_sequence_packages': len(sequences),
        'anim_sequence_packages': sequences,
    }


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--classes', type=Path, default=DEFAULT_CLASSES)
    ap.add_argument('--print', action='store_true', help='print the counts instead of writing the JSON')
    args = ap.parse_args(argv)
    if not args.classes.is_file():
        print(f'animation_inventory: missing {args.classes}', file=sys.stderr)
        return 2
    data = summarize(args.classes)
    if args.print:
        print(json.dumps({k: v for k, v in data.items() if k != 'anim_sequence_packages'}, indent=1))
        return 0
    tmp = OUT.with_name(OUT.name + '.tmp')
    tmp.write_text(json.dumps(data, indent=0, sort_keys=True) + '\n', encoding='utf-8')
    os.replace(tmp, OUT)
    print(f'animation_inventory: wrote {OUT} ({data["animation_sequence_packages"]} AnimSequence packages)')
    return 0


if __name__ == '__main__':
    sys.exit(main())
