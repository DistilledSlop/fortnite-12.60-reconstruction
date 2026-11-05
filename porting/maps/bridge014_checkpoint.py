#!/usr/bin/env python3
"""Validate and optionally promote the recorded isolated Bridge014 import."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import sys

ROOT = Path(__file__).resolve().parents[2]
LOCK = ROOT / 'porting/maps/bridge014_checkpoint_hashes.json'
SELECTION = ROOT / 'porting/maps/bridge014_mesh_selection.json'


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def checked_path(root, relative):
    rel = Path(relative)
    if rel.is_absolute() or '..' in rel.parts or not rel.parts:
        raise ValueError(f'unsafe path: {relative}')
    path = root
    for part in rel.parts:
        path = path / part
        if path.is_symlink():
            raise ValueError(f'symlink in asset path: {path}')
    return path


def verify_inventory(root, expected, label):
    if not root.is_dir():
        raise ValueError(f'missing {label} directory: {root}')
    actual = {p.relative_to(root).as_posix() for p in root.rglob('*') if p.is_file() or p.is_symlink()}
    if actual != set(expected):
        raise ValueError(f'{label} inventory differs: missing={sorted(set(expected)-actual)}, extra={sorted(actual-set(expected))}')
    for relative, wanted in expected.items():
        path = checked_path(root, relative)
        if not path.is_file() or digest(path) != wanted:
            raise ValueError(f'{label} hash mismatch: {path}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--checkpoint', type=Path, default=ROOT / 'porting/generated/bridge014_import_checkpoint/Content', help='isolated imported Content directory')
    parser.add_argument('--glbs', type=Path, default=ROOT / 'porting/generated/bridge014_meshes/glb', help='six exported GLB checkpoint directory')
    parser.add_argument('--target', type=Path, default=ROOT / 'porting/native/Content', help='destination Content directory')
    parser.add_argument('--promote', action='store_true', help='copy missing assets; existing identical files are left untouched')
    args = parser.parse_args()
    lock = json.loads(LOCK.read_text())
    if digest(SELECTION) != lock['selection_sha256']:
        raise ValueError('selection changed since checkpoint was recorded')
    verify_inventory(args.checkpoint, lock['assets'], 'imported asset')
    verify_inventory(args.glbs, lock['glbs'], 'GLB')
    if not args.target.is_dir():
        raise ValueError(f'missing target Content directory: {args.target}')
    missing = []
    for relative, wanted in lock['assets'].items():
        dest = checked_path(args.target, relative)
        # Sidecars may belong to a different package even when the uasset is absent.
        for suffix in ('.uexp', '.ubulk', '.uptnl', '.m.ubulk'):
            sidecar = checked_path(args.target, str(Path(relative).with_suffix(suffix)))
            if sidecar.exists():
                raise ValueError(f'target package has an unexpected sidecar: {sidecar}')
        if dest.exists():
            if not dest.is_file() or digest(dest) != wanted:
                raise ValueError(f'target differs; refusing to overwrite: {dest}')
        else:
            missing.append(relative)
    if args.promote:
        for relative in missing:
            source = checked_path(args.checkpoint, relative)
            dest = checked_path(args.target, relative)
            dest.parent.mkdir(parents=True, exist_ok=True)
            # Exclusive create ensures that a concurrent writer is never clobbered.
            try:
                with source.open('rb') as src, dest.open('xb') as out:
                    shutil.copyfileobj(src, out)
                if digest(dest) != lock['assets'][relative]:
                    raise ValueError(f'copied file hash mismatch: {dest}')
            except FileExistsError:
                raise ValueError(f'target appeared during promotion: {dest}') from None
    print(f"Validated {len(lock['glbs'])} GLBs and {len(lock['assets'])} imported assets; "
          f"{len(missing)} {'copied' if args.promote else 'missing from target'}.")


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, KeyError) as exc:
        print(f'Bridge014 checkpoint: {exc}', file=sys.stderr)
        sys.exit(1)
