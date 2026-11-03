"""Promote the original Athena class while retaining existing Linux fixture support.

The declarations below are compatibility methods implemented by the unchanged
Private/FortniteAthenaGameplay.cpp. They are not recovered original native bodies.
"""
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
ORIGINAL = json.loads((HERE / 'original-class.json').read_text())
HIERARCHY = json.loads((HERE / 'original-hierarchy.json').read_text())
CONTRACT = json.loads((HERE / 'migration-contract.json').read_text())
FLAG = 'athena_mode_existing_fixture_support'
COMPATIBILITY = {'AFortGameModeAthena', 'StartPlay', 'RestartPlayer',
                 'PositionPawnOnTerrain', 'SpawnApolloBackground'}


def validate_classes(blocks, name_to_block, parents):
    if (name_to_block.get(CONTRACT['name']) != CONTRACT['block_va']
            or blocks.get(CONTRACT['block_va']) != ORIGINAL):
        raise ValueError('original Athena class identity/property/function metadata changed')
    for va, expected in HIERARCHY.items():
        if parents.get(va) != expected:
            raise ValueError('original Athena ancestor chain changed: ' + expected['name'])


def validate_original_image(path):
    fd = os.open(path, os.O_RDONLY)
    try:
        for item in CONTRACT['windows']:
            data = os.pread(fd, item['size'], int(item['va'], 16) - 0x140000000)
            if len(data) != item['size'] or hashlib.sha256(data).hexdigest() != item['sha256']:
                raise ValueError('original Athena interface evidence changed: ' + item['va'])
    finally:
        os.close(fd)


def validate_constructor_image(path):
    contract = json.loads((HERE / 'constructor-defaults.json').read_text())
    fd = os.open(path, os.O_RDONLY)
    try:
        for item in contract['windows']:
            data = os.pread(fd, item['size'], int(item['va'], 16) - 0x140000000)
            if len(data) != item['size'] or hashlib.sha256(data).hexdigest() != item['sha256']:
                raise ValueError('original Athena constructor/default provider changed: ' + item['va'])
    finally:
        os.close(fd)


def apply(model, block, owner_fns):
    if model['name'] != CONTRACT['name']:
        return model
    if (block != ORIGINAL or model.get('block_va') != CONTRACT['block_va']
            or model.get('parent') != CONTRACT['parent']
            or model.get('package') != '/Script/FortniteGame'
            or CONTRACT['owner_fn'] not in owner_fns):
        raise ValueError('original Athena owner/parent/schema mismatch')
    if model.get(FLAG) or model.get('native_interface_bases'):
        raise ValueError('duplicate Athena policy or competing native interfaces')
    for group in ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers'):
        collisions = COMPATIBILITY & {m.get('name') for m in model.get(group, [])}
        if collisions:
            raise ValueError('Athena existing fixture declaration collision: ' + ', '.join(sorted(collisions)))
    # Retain both original native interfaces. Their original byte offsets are
    # provenance, not layout offsets to force onto the Linux C++ ABI.
    model['native_interface_bases'] = CONTRACT['interfaces'][:]
    model['splice'].setdefault('header_includes', set()).update({
        'UGameplayMutatorObserverInterface.h', 'UFortMutatorOwner.h'})
    for function in model.get('functions', []):
        if function['name'] == 'OnAircraftEnteredDropZone':
            if function.get('original_flags') != 0x80401:
                raise ValueError('original aircraft callback flags changed')
            function['specifiers'] = [s for s in function['specifiers'] if s != 'BlueprintCallable']
            function['access'] = 'protected'
    model[FLAG] = True
    return model


def validate_emitted_properties(model, split_properties):
    """Integration gate: never silently discard an original Athena property.

    Receives the generator's existing property splitter so container-inner and
    enum-underlying records are interpreted by the same source contract.
    Unrecovered native functions remain explicitly unported in original-class.
    """
    if not model.get(FLAG):
        return
    properties, notes = split_properties(ORIGINAL['properties'])
    expected = [entry['name'] for entry, _children in properties]
    actual = [member['name'] for member in model['members_table_order']]
    if notes or actual != expected:
        missing = sorted(set(expected) - set(actual))
        extra = sorted(set(actual) - set(expected))
        raise ValueError('Athena original property emission incomplete: missing=%r extra=%r notes=%r'
                         % (missing, extra, notes))


def declarations(model):
    return (HERE / 'fixture.declarations.inc').read_text() if model.get(FLAG) else ''
