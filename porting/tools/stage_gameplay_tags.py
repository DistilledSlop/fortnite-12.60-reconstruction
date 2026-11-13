#!/usr/bin/env python3
"""Validate pinned gameplay-tag inputs and atomically stage only missing files.

Configuration comes from native/Config; cooked tables come from --src-game/Content.
This proves staged bytes only. Final server runtime dictionary must be 011ff934.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import shutil
import sys
import tempfile

MANIFEST_SHA256 = '154adb4f65ccfd6f9a266a88e5e996e534422d9b1e69f94fbc71192b97a4d80a'


def sha256(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def no_symlinks(path):
    for part in (path, *path.parents):
        if part.is_symlink():
            raise ValueError(f'Symlink is not allowed: {part}')


def checked_file(path, entry):
    no_symlinks(path)
    if not path.is_file() or path.stat().st_size != entry['bytes'] or sha256(path) != entry['sha256']:
        raise ValueError(f'Missing or changed pinned file: {path}')


def load_manifest(path):
    if sha256(path) != MANIFEST_SHA256:
        raise ValueError(f'Gameplay tag manifest checksum changed: {path}')
    manifest = json.loads(path.read_text())
    seen = set()
    for entry in manifest['files']:
        rel = PurePosixPath(entry['path'])
        if rel.is_absolute() or '..' in rel.parts or str(rel) != entry['path'] or rel.parts[0] not in ('Config', 'Content') or str(rel) in seen:
            raise ValueError(f'Unsafe or repeated manifest path: {rel}')
        seen.add(str(rel))
    return manifest


def validate_config_set(root, entries, require_all):
    expected = {e['path'] for e in entries if e['path'].startswith('Config/')}
    tags = root / 'Config/Tags'
    no_symlinks(tags)
    found = set()
    for path in tags.rglob('*') if tags.exists() else ():
        no_symlinks(path)
        if path.suffix.lower() == '.ini':
            found.add(str(path.relative_to(root)))
    main = root / 'Config/DefaultGameplayTags.ini'
    if main.exists() or main.is_symlink():
        found.add('Config/DefaultGameplayTags.ini')
    if found - expected or (require_all and expected - found):
        raise ValueError(f'Gameplay tag config file set differs at {root}: extra={sorted(found-expected)}, missing={sorted(expected-found) if require_all else []}')


def stage(src_game, dst_stage, native, manifest_path, check_only=False):
    # No directories or files are created until every source and existing destination passes.
    manifest = load_manifest(manifest_path)
    for root in (src_game, dst_stage, native):
        no_symlinks(root)
        if root.exists() and not root.is_dir():
            raise ValueError(f'Expected directory: {root}')
    event = manifest['native_constructor_events']
    event_file = native.parent / event['path']
    no_symlinks(event_file)
    if sha256(event_file) != event['sha256']:
        raise ValueError(f'Native tag constructor event stream changed: {event_file}')
    entries = manifest['files']
    validate_config_set(native, entries, True)
    validate_config_set(dst_stage, entries, False)
    missing = []
    for entry in entries:
        rel = Path(entry['path'])
        source = (native if rel.parts[0] == 'Config' else src_game) / rel
        target = dst_stage / rel
        checked_file(source, entry)
        no_symlinks(target)
        for parent in target.parents:
            if parent.exists() and not parent.is_dir():
                raise ValueError(f'Destination parent is not a directory: {parent}')
        if target.exists():
            checked_file(target, entry)
        else:
            missing.append((source, target, entry))
    copied = 0
    if not check_only:
        for source, target, entry in missing:
            target.parent.mkdir(parents=True, exist_ok=True)
            no_symlinks(target.parent)
            fd, temporary = tempfile.mkstemp(prefix='.gameplay-tags-', dir=target.parent)
            temporary = Path(temporary)
            try:
                os.fchmod(fd, 0o644)
                with os.fdopen(fd, 'wb') as out, source.open('rb') as inp:
                    shutil.copyfileobj(inp, out)
                    out.flush()
                    os.fsync(out.fileno())
                checked_file(temporary, entry)
                # Publish without overwriting a concurrently-created destination.
                try:
                    os.link(temporary, target)
                    copied += 1
                except FileExistsError:
                    checked_file(target, entry)
            finally:
                temporary.unlink(missing_ok=True)
    return {'validated_files': len(entries), 'missing_files': len(missing), 'copied_files': copied, 'check_only': check_only, 'runtime_parity': manifest['runtime_parity']}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--src-game', type=Path, default=Path('/mnt/vibrato/fortnite-extracted-full/FortniteGame'))
    parser.add_argument('--dst-stage', type=Path, required=True)
    parser.add_argument('--check-only', action='store_true')
    args = parser.parse_args()
    porting = next(p for p in Path(__file__).resolve().parents if (p / 'native/FortniteLinux.uproject').is_file())
    result = stage(args.src_game.absolute(), args.dst_stage.absolute(), porting / 'native', Path(__file__).with_name('gameplay_tags_manifest.json'), args.check_only)
    print(json.dumps(result, sort_keys=True))


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, KeyError, StopIteration) as error:
        print(f'Gameplay tag staging refused: {error}', file=sys.stderr)
        sys.exit(1)
