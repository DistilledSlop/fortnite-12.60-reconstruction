#!/usr/bin/env python3
"""Owner-applied, fail-closed migration; defaults to a read-only plan."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'migration-contract.json').read_text())
BLOCKLIST_LINE = 'AFortGameModeAthena\thand-written in porting/native/Source (not generated)\n'


def plan(porting, restore_original_game_state=False):
    header = porting / 'native/Source/FortniteGame/Public/FortniteAthenaGameplay.h'
    cpp = porting / 'native/Source/FortniteGame/Private/FortniteAthenaGameplay.cpp'
    blocklist = porting / 'native_class_blocklist.txt'
    classlist = porting / 'native_class_set.txt'
    old_header = (HERE / 'fixture-header.expected.h').read_bytes()
    new_header = (HERE / 'fixture-header.after.h').read_bytes()
    if header.read_bytes() != old_header:
        raise ValueError('manual Athena header differs from the reviewed complete snapshot')
    if hashlib.sha256(cpp.read_bytes()).hexdigest() != CONTRACT['fixture_cpp_sha256']:
        raise ValueError('fixture cpp changed; review preservation before migrating')
    old_blocklist = blocklist.read_text()
    if old_blocklist.count(BLOCKLIST_LINE) != 1:
        raise ValueError('Athena blocklist entry differs from reviewed input')
    old_classlist = classlist.read_text()
    listed = [line.split('#', 1)[0].strip() for line in old_classlist.splitlines()]
    if CONTRACT['name'] in listed:
        raise ValueError('Athena is already explicitly listed; review partial migration')
    new_classlist = old_classlist.replace('(AFortGameModeAthena, ', '(')
    new_classlist = new_classlist.rstrip() + '\n\n# Original Athena class; existing runtime support retained.\nAFortGameModeAthena\n'
    changes = [(header, new_header), (blocklist, old_blocklist.replace(BLOCKLIST_LINE, '').encode()),
               (classlist, new_classlist.encode())]
    if restore_original_game_state:
        old_cpp = cpp.read_text()
        include = '#include "AFortPlayerControllerAthena.h"\n'
        constructor = 'AFortGameModeAthena::AFortGameModeAthena()\n{\n'
        if old_cpp.count(include) != 1 or old_cpp.count(constructor) != 1:
            raise ValueError('original fixture include/constructor anchor differs')
        new_cpp = old_cpp.replace(include, include + '#include "AFortGameStateAthena.h"\n')
        new_cpp = new_cpp.replace(constructor, constructor +
            '    // Original constructor 0x14192e508: GameStateClass (+0x230).\n'
            '    // The rest of this constructor remains existing Linux fixture support.\n'
            '    GameStateClass = AFortGameStateAthena::StaticClass();\n')
        changes.append((cpp, new_cpp.encode()))
    return changes


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--porting-root', type=Path, default=HERE.parents[1])
    parser.add_argument('--apply', action='store_true')
    parser.add_argument('--restore-original-game-state-class', action='store_true',
                        help='also add the original constructor GameStateClass write; requires constructor image validation')
    parser.add_argument('--original-image', type=Path,
                        help='original memory image for the optional GameStateClass restoration guard')
    args = parser.parse_args()
    if args.restore_original_game_state_class:
        if args.original_image is None:
            parser.error('--restore-original-game-state-class requires --original-image')
        spec = importlib.util.spec_from_file_location('athena_mode_source_policy', HERE / 'source_policy.py')
        policy = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(policy)
        policy.validate_constructor_image(args.original_image)
    changes = plan(args.porting_root.resolve(), args.restore_original_game_state_class)
    for path, content in changes:
        if args.apply:
            path.write_bytes(content)
        print(('updated ' if args.apply else 'would update ') + str(path))
    if args.restore_original_game_state_class:
        print('Existing fixture cpp retained with original GameStateClass store/header added. No generation/build performed.')
    else:
        print('Private/FortniteAthenaGameplay.cpp is retained byte for byte. No generation/build performed.')


if __name__ == '__main__':
    main()
