#!/usr/bin/env python3
"""Stage original content-plugin roots and the hard-import closure of their maps.

The descriptor and Content directory, not a list of guessed mount names, define
eligible roots. Modules are never enabled. Existing staged files are preserved.
"""
import argparse
from collections import deque
import json
from pathlib import Path
import shutil
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from stage_server_content import DEFAULT_EXCLUDE_CLASSES, EXTS, parse_summary


def inventory(extracted, stage):
    roots = {}
    for tree in ('Engine', 'FortniteGame'):
        for descriptor in sorted((extracted / tree / 'Plugins').rglob('*.uplugin')):
            content = descriptor.parent / 'Content'
            if not content.is_dir():
                continue
            data = json.loads(descriptor.read_text(encoding='utf-8-sig'))
            if not data.get('CanContainContent', False):
                continue
            name = descriptor.stem
            if not name.isascii() or not name.replace('_', '').isalnum() or name in roots:
                raise ValueError('invalid or duplicate plugin name: ' + name)
            # Keep all plugin content below the project, including engine plugins;
            # existing engine mounts take precedence at runtime.
            relative = Path('OriginalPluginContent') / tree / descriptor.relative_to(extracted / tree / 'Plugins').parent / 'Content'
            roots[name] = (content, stage / relative, relative)
    return roots


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--extracted', type=Path, required=True)
    ap.add_argument('--stage', type=Path, required=True)
    ap.add_argument('--dry-run', action='store_true')
    args = ap.parse_args()
    roots = inventory(args.extracted, args.stage)
    if not roots:
        raise SystemExit('no original content plugins found')
    mappings = {'Game': (args.extracted / 'FortniteGame/Content', args.stage / 'Content')}
    mappings.update({name: (src, dst) for name, (src, dst, _) in roots.items()})
    queue = deque()
    for name, (src, _, _) in roots.items():
        for path in sorted(src.rglob('*.umap')):
            queue.append('/' + name + '/' + path.relative_to(src).with_suffix('').as_posix())
    visited, missing, skipped, external, unsupported, count = set(), set(), set(), set(), set(), 0
    while queue:
        pkg = queue.popleft()
        if pkg in visited:
            continue
        visited.add(pkg)
        parts = pkg.lstrip('/').split('/', 1)
        if len(parts) != 2 or parts[0] not in mappings:
            if len(parts) == 2 and parts[0] in ('Engine', 'Script'):
                external.add(parts[0])
            else:
                unsupported.add(parts[0])
            continue
        src, dst = mappings[parts[0]]
        base = src / parts[1]
        main_file = next((base.with_suffix(ext) for ext in ('.uasset', '.umap') if base.with_suffix(ext).is_file()), None)
        if not main_file:
            missing.add(pkg)
            continue
        names, imports, classes = parse_summary(main_file)
        if classes & DEFAULT_EXCLUDE_CLASSES:
            skipped.add(pkg)
            continue
        for _, cls, outer, name in imports:
            if outer == 0 and cls == 'Package' and name.startswith('/'):
                queue.append(name)
        for ext in EXTS:
            source = base.with_suffix(ext)
            target = (dst / parts[1]).with_suffix(ext)
            if source.is_file() and not target.exists():
                count += 1
                if not args.dry_run:
                    target.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copyfile(source, target)
    print('plugin roots:', len(roots), 'map/import packages:', len(visited), 'files:', count, 'missing:', len(missing), 'visual packages skipped:', len(skipped))
    print('external roots:', ', '.join(sorted(external)) or '(none)',
          'unsupported roots:', ', '.join(sorted(unsupported)) or '(none)')
    if unsupported:
        raise SystemExit('unsupported imported package roots; refusing stage')
    if missing:
        print('missing:', ', '.join(sorted(missing)[:30]), file=sys.stderr)
        raise SystemExit(1)
    if not args.dry_run:
        for _, dst, _ in roots.values():
            dst.mkdir(parents=True, exist_ok=True)
        manifest = args.stage / 'Config/OriginalPluginMounts.txt'
        manifest.parent.mkdir(parents=True, exist_ok=True)
        manifest.write_text(''.join(name + '|' + rel.as_posix() + '\n' for name, (_, _, rel) in sorted(roots.items())), encoding='utf-8')


if __name__ == '__main__':
    main()
