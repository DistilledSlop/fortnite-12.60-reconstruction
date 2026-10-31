#!/usr/bin/env python3
"""Compare Cue4Inspect --imports output with project headers, read-only.

The flat import listing omits function owners. A function name found somewhere in a
module is only a candidate, not proof that the importing owner exposes it. Engine
modules are excluded because their declarations live outside native/Source.
"""
import argparse
import re
from pathlib import Path

SOURCE = Path(__file__).resolve().parents[1] / 'native/Source'
IMPORT = re.compile(r'^/Script/([^ ]+) \| (Class|Function) \| (\S+)$')
DECL = re.compile(r'\b([A-Za-z_][A-Za-z0-9_]*)\s*\(')
CLASS = re.compile(r'\bclass\s+(?:\w+_API\s+)?([UAFI][A-Za-z0-9_]+)\s*[:{]')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('reports', nargs='+', type=Path, help='saved Cue4Inspect --imports output')
    args = parser.parse_args()
    modules = {p.name: p for p in SOURCE.iterdir() if p.is_dir()}
    indices = {}
    for module, root in modules.items():
        headers = list((root / 'Generated').glob('*.h'))
        if not headers:
            continue
        classes = {p.stem[1:] for p in headers if p.stem[:1] in 'UAFI'}
        functions = set()
        for p in root.rglob('*.h'):
            source = p.read_text(errors='replace')
            classes.update(name[1:] for name in CLASS.findall(source))
            functions.update(DECL.findall(source))
        indices[module] = (classes, functions)
    for report in args.reports:
        print(f'\n{report}:')
        entries = set()
        for line in report.read_text(errors='replace').splitlines():
            match = IMPORT.match(line)
            if match:
                entries.add(match.groups())
        if not entries:
            raise SystemExit(f'No script class/function imports in {report}')
        for module in sorted({m for m, _, _ in entries}):
            items = sorted((kind, name) for m, kind, name in entries if m == module)
            if module not in indices:
                print(f'  {module}: {len(items)} class/function names; no generated project module indexed (engine or missing module)')
                continue
            classes, functions = indices[module]
            missing_classes = [name for kind, name in items if kind == 'Class' and name not in classes]
            missing_functions = [name for kind, name in items if kind == 'Function' and name not in functions]
            print(f'  {module}: {len(items)} names; classes without project declaration: {", ".join(missing_classes) or "none"}')
            print(f'    function names absent from project module headers: {", ".join(missing_functions) or "none"}')
    print('\nFunction matches are module-wide only; verify owner, flags and signatures. Missing module does not imply absent engine code.')


if __name__ == '__main__':
    main()
