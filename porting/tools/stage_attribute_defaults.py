#!/usr/bin/env python3
"""Stage selected original attribute defaults and their cooked content roots."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import stat
import subprocess
import sys
import tempfile

SECTIONS = {
    b'/script/gameplayabilities.abilitysystemglobals': {b'globalattributesetdefaultstablenames'},
    b'/script/fortnitegame.fortabilitysystemglobals': {b'bronlyattributesetdefaultstoexclude'},
    b'/script/fortnitegame.fortglobals': {b'bbronly'},
}
HEADER = re.compile(rb'^\s*\[([^\]\r\n]+)\]\s*$')
ENTRY = re.compile(rb'^\s*([+.!-]?)([A-Za-z0-9_]+)\s*=\s*(.*?)\s*$')
OBJECT = re.compile(rb'^(/Game/[A-Za-z0-9_/]+)\.([A-Za-z0-9_]+)$')


def selected(data):
    result = {key: [] for keys in SECTIONS.values() for key in keys}
    section = None
    for line in data.splitlines():
        match = HEADER.match(line)
        if match:
            section = match.group(1).strip().lower()
            continue
        match = ENTRY.match(line)
        if match and section in SECTIONS and match.group(2).lower() in SECTIONS[section]:
            op, key, value = match.groups()
            key = key.lower()
            if (key == b'bbronly' and op != b'') or (key != b'bbronly' and op != b'+'):
                raise ValueError('unsupported original config operator')
            result[key].append((line, value.strip(b'"')))
    tables = result[b'globalattributesetdefaultstablenames']
    exclusions = result[b'bronlyattributesetdefaultstoexclude']
    if len(tables) != 11 or len(exclusions) != 3 or len(result[b'bbronly']) > 1:
        raise ValueError('original attribute defaults are missing or incomplete')
    paths = [value for _, value in tables]
    if len(set(paths)) != len(paths) or len({v for _, v in exclusions}) != len(exclusions):
        raise ValueError('duplicate original attribute defaults')
    if not {v for _, v in exclusions}.issubset(paths):
        raise ValueError('original exclusions do not match tables')
    packages = []
    for path in paths:
        match = OBJECT.fullmatch(path)
        if not match:
            raise ValueError('invalid attribute table object path')
        packages.append(match.group(1).decode('ascii'))
    return result, packages


def merge(target, entries):
    lines = target.splitlines(keepends=True)
    output = []
    section = None
    seen = set()
    for line in lines:
        match = HEADER.match(line.rstrip(b'\r\n'))
        if match:
            section = match.group(1).strip().lower()
            if section in SECTIONS:
                if section in seen:
                    raise ValueError('duplicate target attribute section')
                seen.add(section)
            output.append(line if line.endswith(b"\n") else line + b"\n")
            if section in SECTIONS:
                for key in sorted(SECTIONS[section]):
                    for source_line, _ in entries[key]:
                        output.append(source_line + b'\n')
            continue
        match = ENTRY.match(line.rstrip(b'\r\n'))
        if match and section in SECTIONS and match.group(2).lower() in SECTIONS[section]:
            key = match.group(2).lower()
            if key != b'bbronly' or entries[key]:
                continue
        output.append(line)
    for section in SECTIONS:
        if section not in seen:
            if output and not output[-1].endswith(b'\n'):
                output.append(b'\n')
            output.append(b'\n[' + section + b']\n')
            for key in sorted(SECTIONS[section]):
                for source_line, _ in entries[key]:
                    output.append(source_line + b'\n')
    return b''.join(output)


def stage(original, target, out, content_root, stager, runner=subprocess.run, content_destination=None, receipt_target=None):
    original, target, out = map(Path, (original, target, out))
    if target.is_symlink() or not target.is_file() or out.exists():
        raise ValueError('target must be an existing regular file and evidence must not exist')
    source = original.read_bytes()
    entries, packages = selected(source)
    before = target.read_bytes()
    after = merge(before, entries)
    # Resolve the complete selected closure in isolation first. The shared content
    # stager skips existing files, so never let stale stage files mask mismatches.
    destination = Path(content_destination) if content_destination is not None else Path(__file__).resolve().parent.parent / 'generated/linux-stage/LinuxNoEditor/FortniteLinux/Content'
    with tempfile.TemporaryDirectory(prefix='attribute-content-') as closure_dir:
        closure = Path(closure_dir)
        runner([sys.executable, '-I', str(stager), '--src', str(content_root), '--dst', str(closure), '--max', '30000', '--strict', *packages], check=True)
        files = [p for p in closure.rglob('*') if p.is_file()]
        for source_file in files:
            dest = destination / source_file.relative_to(closure)
            if dest.exists() and (not dest.is_file() or dest.read_bytes() != source_file.read_bytes()):
                raise ValueError('staged content differs from selected source: ' + str(dest))
        for source_file in files:
            dest = destination / source_file.relative_to(closure)
            if not dest.exists():
                dest.parent.mkdir(parents=True, exist_ok=True)
                with tempfile.NamedTemporaryFile(dir=dest.parent, prefix='.attribute-content-', delete=False) as f:
                    temp_content = Path(f.name)
                    try:
                        with source_file.open('rb') as src:
                            shutil.copyfileobj(src, f)
                        f.flush()
                        os.fsync(f.fileno())
                    except BaseException:
                        temp_content.unlink(missing_ok=True)
                        raise
                try:
                    # Do not overwrite a file created by another stager.
                    os.link(temp_content, dest)
                finally:
                    temp_content.unlink(missing_ok=True)
    out.mkdir(parents=True)
    (out / 'DefaultGame.ini.before').write_bytes(before)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=target.parent, prefix='.DefaultGame.ini.', delete=False) as f:
            temporary = Path(f.name)
            f.write(after)
            f.flush()
            os.fsync(f.fileno())
        temporary.chmod(stat.S_IMODE(target.stat().st_mode))
        if target.read_bytes() != before:
            raise ValueError('target changed during staging')
        os.replace(temporary, target)
        temporary = None
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)
    receipt = {'source': str(original.resolve()), 'target': str(Path(receipt_target).resolve() if receipt_target else target.resolve()),
               'prepared_target': str(target.resolve()) if receipt_target else None,
               'source_sha256': hashlib.sha256(source).hexdigest(),
               'before_sha256': hashlib.sha256(before).hexdigest(),
               'after_sha256': hashlib.sha256(after).hexdigest(),
               'table_packages': packages, 'table_count': len(packages),
               'exclusion_count': len(entries[b'bronlyattributesetdefaultstoexclude']),
               'br_only_present': bool(entries[b'bbronly'])}
    (out / 'receipt.json').write_text(json.dumps(receipt, indent=2) + '\n')
    return receipt


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    for arg in ('original', 'target', 'out', 'content-root'):
        ap.add_argument('--' + arg, required=True)
    ap.add_argument('--receipt-target', help='intended final target when preparing in a temporary directory')
    args = ap.parse_args()
    try:
        receipt = stage(args.original, args.target, args.out, args.content_root,
                        Path(__file__).with_name('stage_server_content.py'), receipt_target=args.receipt_target)
    except (ValueError, OSError, subprocess.CalledProcessError) as exc:
        ap.exit(1, f'attribute defaults staging failed: {exc}\n')
    print(json.dumps({'receipt': str(Path(args.out).resolve() / 'receipt.json'),
                      'table_count': receipt['table_count'], 'exclusion_count': receipt['exclusion_count']}))


if __name__ == '__main__':
    main()
