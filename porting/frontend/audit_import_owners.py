#!/usr/bin/env python3
"""Report project function import owners, reflected signatures and header candidates."""
import argparse
import json
import re
from pathlib import Path

PORTING = Path(__file__).resolve().parents[1]
REF = PORTING / 'generated/reflection'
SOURCE = PORTING / 'native/Source'


def report(path, blocks, parents, reverse):
    data = json.loads(path.read_text())
    if not isinstance(data, dict) or not isinstance(data.get('imports'), list) or not data['imports']:
        raise ValueError('expected nonempty imports array')
    print(f"\n{data.get('package', path)}")
    for item in data['imports']:
        if not isinstance(item, dict):
            raise ValueError('invalid import item')
        chain = item.get('chain')
        if not isinstance(chain, list) or not chain or any(not isinstance(n, dict) or 'name' not in n for n in chain):
            raise ValueError('invalid import chain')
        if chain[-1]['index'] != item.get('index') or chain[-1]['name'] != item.get('name'):
            raise ValueError('import chain does not end at item')
        if any(a.get('index') != b.get('outerIndex') for a, b in zip(chain, chain[1:])):
            raise ValueError('inconsistent import outer chain')
        if item.get('kind') != 'Function':
            continue
        if len(chain) != 3 or not chain[0]['name'].startswith('/Script/'):
            continue
        module, owner, member = chain[0]['name'][8:], chain[1]['name'], item['name']
        root = SOURCE / module
        if not root.is_dir():
            continue
        choices = [prefix + owner for prefix in ('U', 'A', 'I') if prefix + owner in blocks]
        print(f'  {module}.{owner}:{member}')
        if len(choices) != 1:
            print('    owner reflection ambiguous or missing:', choices)
            continue
        cls = choices[0]
        lineage = parents.get(cls, [cls])
        matches = []
        for ancestor in lineage:
            block = blocks.get(ancestor)
            if not block:
                continue
            for f in block.get('functions', []):
                if f.get('name') != member:
                    continue
                candidates = f.get('candidates')
                if candidates:
                    candidates = [c for c in candidates if reverse.get(str(c.get('outer_fn','')).lower()) == block['block_va'].lower()]
                else:
                    candidates = [f]
                for candidate in candidates:
                    params = candidate.get('params')
                    if params is None:
                        signature = 'undecoded'
                    else:
                        signature = ', '.join(f"{p.get('name')}:{p.get('kind')}:{p.get('type_refs', {})}" for p in params if not p.get('enum_underlying_of')) or '(none)'
                    matches.append((ancestor, candidate.get('flags'), signature))
        print('    reflected:', matches or 'missing/ambiguous')
        found = []
        for header in root.rglob('*.h'):
            text = header.read_text(errors='replace')
            # Only count a header containing the declaring class, not a module-wide name match.
            for ancestor in lineage:
                if re.search(r'\bclass\s+(?:\w+_API\s+)?' + re.escape(ancestor) + r'\s*[:{]', text) and re.search(r'\b' + re.escape(member) + r'\s*\(', text):
                    found.append(str(header.relative_to(PORTING)))
        print('    owner/ancestor header name candidates:', ', '.join(found) or 'none')
    print('  Header candidates do not establish UFUNCTION flags or compatible signatures; engine ancestors are not scanned.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('reports', nargs='+', type=Path)
    args = parser.parse_args()
    names = json.loads((REF / 'class_name_map.json').read_text())
    raw = json.loads((REF / 'fortnite_game_reflection.json').read_text())
    blocks = {names[b['block_va']]: b for b in raw['classes_in_groups_with_fort_names'] if b['block_va'] in names}
    parent_data = json.loads((REF / 'class_parent_map.json').read_text())
    parents = {v['name']: v.get('chain', [v['name']]) for v in parent_data.values() if v.get('name')}
    evidence = json.loads((REF / 'class_name_map.evidence.json').read_text())
    reverse = {'0x' + fn.removeprefix('0x').lower(): record['block_va'].lower()
               for record in evidence['blocks'] for fn in record.get('construct_fns', [])}
    for path in args.reports:
        report(path, blocks, parents, reverse)

if __name__ == '__main__':
    main()
