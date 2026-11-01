#!/usr/bin/env python3
"""Classify header-only super imports against original reflection and current headers."""
import argparse
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]

def parse_path(path):
    match = re.fullmatch(r'/Script/([A-Za-z][A-Za-z0-9_]*)\.([A-Za-z][A-Za-z0-9_]*):([A-Za-z][A-Za-z0-9_]*)', path)
    if not match:
        raise ValueError('invalid native super path')
    return '/Script/' + match[1], match[2], match[3]

def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def classify(path, classmap, parentmap, blocks, constructors, header_root, engine_root):
    package, cls, name = parse_path(path)
    matches = [(va, cpp) for va, cpp in classmap.items()
               if cpp in ('U' + cls, 'A' + cls) and parentmap.get(va, {}).get('package') == package]
    if len(matches) != 1:
        return {'classification': 'owner_unresolved_or_ambiguous', 'owner_candidates': matches}
    va, cpp = matches[0]
    block = blocks.get(va)
    if block is None:
        return {'classification': 'reflection_owner_missing', 'owner_block': va, 'cpp_class': cpp}
    functions = [f for f in block.get('functions', []) if f.get('name') == name]
    header_candidates = sorted(header_root.glob('*/Generated/' + cpp + '.h'))
    if package == '/Script/UMG' and cls == 'UserWidget':
        header_candidates = [engine_root / 'Engine/Source/Runtime/UMG/Public/Blueprint/UserWidget.h']
    header = header_candidates[0] if len(header_candidates) == 1 and header_candidates[0].is_file() else None
    declaration = bool(re.search(r'\b' + re.escape(name) + r'\s*\(', header.read_text())) if header else None
    base = {'owner_block': va, 'cpp_class': cpp, 'header': str(header) if header else None,
            'header_sha256': sha(header) if header else None, 'current_header_declares_name': declaration}
    if len(functions) != 1:
        return dict(base, classification='function_unresolved_or_ambiguous', function_count=len(functions))
    f = functions[0]
    owner_fns = {('0x' + x.removeprefix('0x')).lower() for x in constructors.get(va, [])}
    if not owner_fns:
        return dict(base, classification='owner_constructor_unresolved', function=f)
    candidates = f.get('candidates')
    if candidates:
        owned = [c for c in candidates if str(c.get('outer_fn', '')).lower() in owner_fns]
        if len(owned) != 1:
            return dict(base, classification='ambiguous_function_owner', function=f, owner_constructors=sorted(owner_fns))
        selected = owned[0]
        flags = selected.get('flags'); params = selected.get('params'); outer = selected.get('outer_fn')
        if not isinstance(params, list) or selected.get('nprop') != len(params):
            return dict(base, classification='flags_or_params_unresolved', function=f, owner_constructors=sorted(owner_fns))
    else:
        flags_list = f.get('flags') or []
        flags = flags_list[0] if len(flags_list) == 1 else None
        params = f.get('params'); outer = f.get('outer_fn')
        if str(outer or '').lower() not in owner_fns:
            return dict(base, classification='ambiguous_function_owner', function=f, owner_constructors=sorted(owner_fns))
    try: value = int(str(flags), 0)
    except (ValueError, TypeError): value = None
    if value is None or params is None:
        kind = 'flags_or_params_unresolved'
    elif value & 0x400:
        kind = 'native_requires_behavior'
    elif value & 0x08000800 == 0x08000800:
        kind = 'observed_non_native_blueprint_event'
    else:
        kind = 'other_non_native_function'
    return dict(base, classification=kind, owner_constructors=sorted(owner_fns), create_func=f.get('create_func'),
                outer_func=outer, flags=flags,
                access=([name for bit, name in ((0x20000, 'public'), (0x40000, 'private'), (0x80000, 'protected')) if value is not None and value & bit]),
                parameters=params, serialized_property_count=len(params) if params is not None else None)

def main():
    ap = argparse.ArgumentParser(); ap.add_argument('inventory'); ap.add_argument('output'); args = ap.parse_args()
    inp = Path(args.inventory); out = Path(args.output)
    if out.exists(): raise SystemExit('output already exists')
    reflection = ROOT / 'generated/reflection'
    def load(name): return json.loads((reflection / name).read_text())
    inventory = json.loads(inp.read_text())
    if inventory.get('schema') != 'fortnite.native-super-imports.v1': raise SystemExit('wrong input schema')
    classmap, parentmap = load('class_name_map.json'), load('class_parent_map.json')
    evidence_map = load('class_name_map.evidence.json')
    constructors = {x['block_va']: x.get('construct_fns', []) for x in evidence_map['blocks']}
    data = load('fortnite_game_reflection.json')
    blocks = {x['block_va']: x for x in data['classes_in_groups_with_fort_names']}
    names = sorted({x['path'] for x in inventory['targets']})
    rows = [dict(path=n, **classify(n, classmap, parentmap, blocks, constructors, ROOT / 'native/Source', ROOT.parents[1] / 'UnrealEngine')) for n in names]
    evidence = {x: sha(reflection / x) for x in ('class_name_map.json', 'class_parent_map.json', 'class_name_map.evidence.json', 'fortnite_game_reflection.json')}
    report = {'schema':'fortnite.native-super-classification.v1','inventory_sha256':sha(inp),'evidence_sha256':evidence,'targets':rows}
    with out.open('x') as f: json.dump(report, f, indent=2); f.write('\n')
if __name__ == '__main__': main()
