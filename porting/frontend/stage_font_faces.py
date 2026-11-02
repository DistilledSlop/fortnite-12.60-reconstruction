#!/usr/bin/env python3
"""Stage original font-face sidecars beside matching recovered cooked packages."""

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import sys


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def stage(source, target, subtree=Path('.'), check=False):
    source = source.resolve()
    target = target.resolve()
    if not source.is_dir() or not target.is_dir():
        raise ValueError('source and stage content roots must be existing directories')
    if subtree.is_absolute() or '..' in subtree.parts:
        raise ValueError('subtree must be a relative path within the content roots')
    scan = source / subtree
    if not scan.is_dir():
        raise ValueError(f'source subtree is not a directory: {scan}')
    records = []
    for font in sorted(scan.rglob('*.ufont')):
        rel = font.relative_to(source)
        dest = target / rel
        record = {'path': rel.as_posix(), 'sha256': digest(font), 'status': None, 'errors': []}
        for suffix in ('.uasset', '.uexp'):
            src_package = font.with_suffix(suffix)
            dst_package = dest.with_suffix(suffix)
            if not src_package.is_file():
                record['errors'].append(f'missing source package {src_package.relative_to(source)}')
            elif not dst_package.is_file():
                record['errors'].append(f'missing staged package {dst_package.relative_to(target)}')
            elif digest(src_package) != digest(dst_package):
                record['errors'].append(f'mismatched staged package {dst_package.relative_to(target)}')
        if dest.exists():
            if not dest.is_file() or digest(dest) != record['sha256']:
                record['errors'].append(f'conflicting staged payload {rel}')
            else:
                record['status'] = 'identical'
        elif check:
            record['errors'].append(f'missing staged payload {rel}')
        else:
            record['status'] = 'pending'
        if record['errors']:
            record['status'] = 'error'
        records.append(record)
    # Validate the complete selection before writing anything.
    if not check and not any(r['errors'] for r in records):
        for record in records:
            if record['status'] == 'pending':
                rel = Path(record['path'])
                shutil.copyfile(source / rel, target / rel)
                if digest(target / rel) != record['sha256']:
                    record['errors'].append(f'payload changed during copy {rel}')
                    record['status'] = 'error'
                else:
                    record['status'] = 'copied'
    return {'source_content': str(source), 'stage_content': str(target),
            'subtree': subtree.as_posix(), 'check': check, 'count': len(records),
            'ok': not any(r['errors'] for r in records), 'files': records}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-content', required=True, type=Path)
    parser.add_argument('--stage-content', required=True, type=Path)
    parser.add_argument('--subtree', type=Path, default=Path('.'))
    parser.add_argument('--check', action='store_true', help='verify only; write JSON receipt to stdout')
    args = parser.parse_args(argv)
    try:
        receipt = stage(args.source_content, args.stage_content, args.subtree, args.check)
    except (OSError, ValueError) as exc:
        print(json.dumps({'ok': False, 'error': str(exc)}, indent=2))
        return 1
    print(json.dumps(receipt, indent=2))
    return 0 if receipt['ok'] else 1


if __name__ == '__main__':
    sys.exit(main())
