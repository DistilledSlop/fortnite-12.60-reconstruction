#!/usr/bin/env python3
"""Stage character definitions and the classes loaded through their soft references.

The pinned export-class census is generated with Cue4Inspect --scan-export-classes
against the 12.60 extracted root. Only /Game packages are handled here; plugin
packages have different mounts and are not Athena character registry entries.
The existing strict stager follows hard imports with its normal visual exclusions.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

MANIFEST_SHA1 = 'aafb2f7788864605068d85fccd1de7e627f18682'
CENSUS_SHA256 = '683dd62e9e97da83fddfff7608b12e7f06a49c9a4d028e41d567ea561f5f714e'
COUNTS = {'FortHeroSpecialization': 2567, 'CustomCharacterPart': 3674}
CHARACTER_PREFIXES = (
    '/Game/Athena/Items/Cosmetics/Characters/',
    '/Game/Heroes/Commando/CosmeticCharacterItemDefinitions/',
    '/Game/Heroes/Constructor/CosmeticCharacterItemDefinitions/',
    '/Game/Heroes/Ninja/CosmeticCharacterItemDefinitions/',
    '/Game/Heroes/Outlander/CosmeticCharacterItemDefinitions/',
)


def checked_package(path):
    if not isinstance(path, str) or not path.startswith('/Game/'):
        raise ValueError('invalid /Game package path')
    if any(part in ('', '.', '..') for part in path[1:].split('/')) or any(c in path for c in '\\\t\r\n:'):
        raise ValueError('unsafe package path')
    return path


def select_roots(manifest, census, manifest_sha1=MANIFEST_SHA1, census_sha256=CENSUS_SHA256,
                 expected_characters=952, expected_counts=COUNTS):
    manifest_bytes = Path(manifest).read_bytes()
    census_bytes = Path(census).read_bytes()
    if hashlib.sha1(manifest_bytes).hexdigest() != manifest_sha1:
        raise ValueError('character registry digest mismatch')
    if hashlib.sha256(census_bytes).hexdigest() != census_sha256:
        raise ValueError('export-class census digest mismatch')
    rows = json.loads(manifest_bytes)['rows']
    if len(rows) != expected_characters:
        raise ValueError('unexpected character registry count')
    roots = set()
    for row in rows:
        object_path = row['objectPath']
        package, dot, name = object_path.partition('.')
        checked_package(package)
        if not dot or not name or any(c in name for c in '/.\\\t\r\n:') or not package.startswith(CHARACTER_PREFIXES):
            raise ValueError('invalid character object path')
        roots.add(package)
    if len(roots) != expected_characters:
        raise ValueError('duplicate character package')
    counts = dict.fromkeys(expected_counts, 0)
    prefix = 'FortniteGame/Content/'
    for line in census_bytes.decode('utf-8').splitlines():
        package, tab, classes = line.partition('\t')
        if not tab:
            raise ValueError('malformed census row')
        matched = set(classes.split(',')) & expected_counts.keys()
        if not matched or not package.startswith(prefix):
            continue
        game_path = checked_package('/Game/' + package[len(prefix):])
        roots.add(game_path)
        for name in matched:
            counts[name] += 1
    if counts != expected_counts:
        raise ValueError('unexpected class census counts: ' + repr(counts))
    return sorted(roots)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest', type=Path, required=True)
    parser.add_argument('--census', type=Path, required=True)
    parser.add_argument('--src', type=Path, required=True, help='extracted FortniteGame/Content')
    parser.add_argument('--dst', type=Path, help='staged Content directory (default: server stage)')
    parser.add_argument('--dry-run', action='store_true')
    args = parser.parse_args()
    try:
        roots = select_roots(args.manifest, args.census)
        cmd = [sys.executable, '-I', str(Path(__file__).with_name('stage_server_content.py')),
               '--src', str(args.src), '--strict', '--max', '30000']
        if args.dst:
            cmd.extend(['--dst', str(args.dst)])
        if args.dry_run:
            cmd.append('--dry-run')
        print('character content roots:', len(roots), flush=True)
        subprocess.run(cmd + roots, check=True)
    except (OSError, ValueError, KeyError, TypeError, subprocess.CalledProcessError) as exc:
        parser.exit(1, f'Character content staging failed: {exc}\n')


if __name__ == '__main__':
    main()
