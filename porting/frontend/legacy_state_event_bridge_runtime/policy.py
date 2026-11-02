"""Fail-closed policy for the recovered legacy NUI Blueprint event bridge."""
import hashlib
import json
import os
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
WINDOWS = json.loads((HERE / 'original-windows.json').read_text())['windows']
FLAG = 'legacy_state_event_bridge_runtime'
OWNER = 'UFortUIStateWidget_NUI'
ENUM_FN = '0x14531e170'
EVENT_ARGS = {'OnEnterState': 'EFortUIState PreviousUIState',
              'OnExitState': 'EFortUIState NextUIState'}
ALIASES = {'NativeActivateLegacyState510', 'NativeDeactivateLegacyState518'}


def validate_source(image_path):
    fd = os.open(image_path, os.O_RDONLY)
    try:
        for window in WINDOWS:
            data = os.pread(fd, window['size'], int(window['va'], 16) - 0x140000000)
            if len(data) != window['size'] or hashlib.sha256(data).hexdigest() != window['sha256']:
                raise ValueError('legacy state bridge original mismatch: ' + window['va'])
    finally:
        os.close(fd)


def _validate_block(block):
    if (not isinstance(block, dict)
            or any(block.get(k) != v for k, v in CONTRACT['identity'].items())
            or block.get('properties') != CONTRACT['properties']
            or [f for f in block.get('functions', []) if f.get('name') in EVENT_ARGS]
               != CONTRACT['functions']):
        raise ValueError('legacy state widget original class or event records mismatch')


def validate_classes(blocks, names):
    va = CONTRACT['identity']['block_va']
    if names.get(OWNER) != va:
        raise ValueError('legacy state widget class identity mismatch')
    _validate_block(blocks.get(va))


def validate_types(gate):
    info = gate.info(ENUM_FN)
    if (info is None or info.fn != ENUM_FN or info.kind != 'enum'
            or info.cpp_name != 'EFortUIState' or info.package != '/Script/FortniteUI'
            or info.rec != CONTRACT['enum'] or gate.reason(info) is not None):
        raise ValueError('original EFortUIState identity or availability mismatch')


def apply(model, block, owner_fns):
    if model.get('name') != OWNER:
        return model
    if (model.get('package') != '/Script/FortniteUI'
            or model.get('parent') != 'UFortActivatablePanel'
            or model.get('block_va') != CONTRACT['identity']['block_va']
            or CONTRACT['owner_fn'] not in owner_fns):
        raise ValueError('legacy state widget owner mismatch')
    _validate_block(block)
    forbidden = set(EVENT_ARGS) | ALIASES
    if model.get(FLAG) or any(item.get('name') in forbidden for group in
            ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers')
            for item in model.get(group, [])):
        raise ValueError('legacy state event bridge declaration collision')
    for name, arg in EVENT_ARGS.items():
        model['functions'].append({'name': name, 'rpc': False, 'service': None,
            'ret': 'void', 'args': [arg], 'static': False, 'const': False,
            'notify': False, 'splice': False, 'blueprint_event': True,
            'access': 'public', 'specifiers': ['BlueprintImplementableEvent'],
            'used': {ENUM_FN: {'Enum'}}})
    model.setdefault('used', {}).setdefault(ENUM_FN, set()).add('Enum')
    model['skipped'] = [s for s in model.get('skipped', []) if not
        (s.get('item') == 'function' and s.get('name') in EVENT_ARGS)]
    model[FLAG] = True
    return configure_model(model)


def configure_model(model):
    if model.get(FLAG):
        model['includes'] = sorted(set(model.get('includes', [])) | {'EFortUIState.h'})
    return model


def declarations(model):
    return (HERE / 'event.declarations.inc').read_text() if model.get(FLAG) else ''


def definitions(model):
    return (HERE / 'event.definitions.inc').read_text() if model.get(FLAG) else ''


def validate_output(model, header, cpp, enum_header):
    if (not model.get(FLAG) or model.get('name') != OWNER
            or header.count('#include "EFortUIState.h"') != 1
            or header.count(declarations(model).strip()) != 1
            or cpp.count(definitions(model).strip()) != 1):
        raise ValueError('legacy state event bridge output mismatch')
    for name, arg in EVENT_ARGS.items():
        signature = 'UFUNCTION(BlueprintImplementableEvent)\n\tvoid ' + name + '(' + arg + ');'
        if header.count(signature) != 1 or len(re.findall(r'\b' + name + r'\b', header)) != 1:
            raise ValueError('legacy state event output mismatch: ' + name)
    for name in ALIASES:
        if len(re.findall(r'\bUFortUIStateWidget_NUI::' + name + r'\s*\(', cpp)) != 1:
            raise ValueError('legacy state slot output mismatch: ' + name)
    if not re.search(r'enum\s+class\s+EFortUIState\s*:\s*uint8\b', enum_header):
        raise ValueError('legacy state enum declaration mismatch')
    for item in CONTRACT['enum']['enumerators']:
        name = item['name'].split('::')[-1]
        if not re.search(r'\b' + re.escape(name) + r'\s*=\s*' + str(item['value']) + r'\s*,', enum_header):
            raise ValueError('legacy state enum value mismatch: ' + name)
