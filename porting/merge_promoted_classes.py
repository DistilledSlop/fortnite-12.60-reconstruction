#!/usr/bin/env python3
"""Merge successful staged native generation without touching identical files."""
import filecmp
import json
import os
from pathlib import Path
import shutil
import sys
import tempfile


def files(root):
    result = []
    for path in root.rglob('*'):
        if path.is_symlink():
            raise ValueError(f'symlink in staged output: {path}')
        if path.is_file():
            result.append(path.relative_to(root))
        elif not path.is_dir():
            raise ValueError(f'unsupported staged output: {path}')
    return sorted(result)


def replace_if_changed(src, dst):
    if dst.is_file() and not dst.is_symlink() and filecmp.cmp(src, dst, shallow=False):
        return
    dst.parent.mkdir(parents=True, exist_ok=True)
    fd, temp = tempfile.mkstemp(prefix='.promotion-', dir=dst.parent)
    try:
        with os.fdopen(fd, 'wb') as out, src.open('rb') as inp:
            shutil.copyfileobj(inp, out)
        os.chmod(temp, src.stat().st_mode & 0o777)
        os.replace(temp, dst)
    finally:
        if os.path.exists(temp):
            os.unlink(temp)


def merge(staged_source, source, staged_reports, reports):
    source_files = files(staged_source)
    report_files = files(staged_reports)
    # A successful generator exit can still report rejected behavior splices.
    # Publishing those files would silently replace implementations with stubs.
    with (staged_reports / 'behavior_splice.json').open(encoding='utf-8') as fh:
        behavior = json.load(fh)
    rejected = behavior.get('rejected')
    if type(rejected) is not int or rejected != 0:
        raise ValueError(f'behavior splices rejected or invalid: {rejected!r}')
    # Reject unexpected output before touching the destination.
    for rel in source_files:
        p = rel.parts
        if not (len(p) >= 3 and p[1] == 'Generated' or
                len(p) == 2 and p[1] == p[0] + '.Build.cs' or
                len(p) == 3 and p[1] == 'Private' and p[2] == p[0] + '.cpp'):
            raise ValueError(f'unexpected staged source: {rel}')
    if any(len(p.parts) != 1 for p in report_files):
        raise ValueError('unexpected staged report directory')
    for rel in source_files:
        replace_if_changed(staged_source / rel, source / rel)
    for rel in report_files:
        replace_if_changed(staged_reports / rel, reports / rel)
    expected = {rel for rel in source_files if len(rel.parts) >= 3 and rel.parts[1] == 'Generated'}
    # Match the former Source/*/Generated cleanup, including whole stale modules.
    for generated in source.glob('*/Generated'):
        if not generated.is_dir() or generated.is_symlink():
            continue
        for root, dirs, names in os.walk(generated, topdown=False, followlinks=False):
            for name in names:
                path = Path(root) / name
                if path.relative_to(source) not in expected:
                    path.unlink()
            for name in dirs:
                path = Path(root) / name
                if path.is_symlink():
                    path.unlink()
                elif not any(path.iterdir()):
                    path.rmdir()
        if not any(generated.iterdir()):
            generated.rmdir()


if __name__ == '__main__':
    if len(sys.argv) != 5:
        sys.exit('usage: merge_promoted_classes.py STAGED_SOURCE SOURCE STAGED_REPORTS REPORTS')
    merge(*(Path(arg) for arg in sys.argv[1:]))
