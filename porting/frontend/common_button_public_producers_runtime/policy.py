"""Unregistered, fail-closed public selection producer splice.

Input-action producers have separate unfinished routing dependencies; this policy
intentionally does not declare or implement either action setter.
"""
import hashlib
import json
import os
import re
import struct
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
FLAG = 'common_button_public_selection_producers_runtime'
NAMES = ('ClearSelection', 'SetIsSelected', 'SetSelectedInternal')
GROUPS = ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers')


def validate_original_image(path):
    fd = os.open(path, os.O_RDONLY)
    try:
        for item in CONTRACT['windows']:
            data = os.pread(fd, item['length'], int(item['va'], 16) - 0x140000000)
            if len(data) != item['length'] or hashlib.sha256(data).hexdigest() != item['sha256']:
                raise ValueError('original source mismatch: ' + item['va'])
        for item in CONTRACT['vtables']:
            pos = int(item['table'], 16) + int(item['slot'], 16) - 0x140000000
            if os.pread(fd, 8, pos) != struct.pack('<Q', int(item['target'], 16)):
                raise ValueError('original virtual slot mismatch')
    finally:
        os.close(fd)


def validate_classes(blocks, name_to_block, native_funcs):
    identity = CONTRACT['identity']
    block = blocks.get(identity['block_va'])
    if name_to_block.get('UCommonButton') != identity['block_va'] or block is None or any(block.get(k) != v for k, v in identity.items()):
        raise ValueError('CommonButton identity mismatch')
    for record in CONTRACT['functions']:
        if [x for x in block.get('functions', []) if x.get('name') == record['name']] != [record]:
            raise ValueError('original function mismatch: ' + record['name'])
    owner = native_funcs.get('UCommonButton', {})
    for name, record in CONTRACT['native_functions'].items():
        if owner.get(name) != record:
            raise ValueError('original native exec mismatch: ' + name)


def validate_model(model):
    if (model.get('name'), model.get('package'), model.get('block_va')) != ('UCommonButton', '/Script/CommonUI', CONTRACT['identity']['block_va']):
        raise ValueError('CommonButton model identity mismatch')
    if not model.get('common_button_focus_selection_runtime'):
        raise ValueError('complete native selection transition dependency missing')


def apply(model, block, owner_fns):
    if model.get('name') != 'UCommonButton': return model
    validate_model(model)
    if block.get('block_va') != CONTRACT['identity']['block_va'] or '0x140805cb0' not in owner_fns:
        raise ValueError('CommonButton owner mismatch')
    if model.get(FLAG) or any(m.get('name') in NAMES for group in GROUPS for m in model.get(group, [])):
        raise ValueError('selection producer collision')
    model[FLAG] = True
    return model


def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get(FLAG) else ''


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get(FLAG) else ''


def validate_output(model, header, cpp):
    validate_model(model)
    if not model.get(FLAG): raise ValueError('selection producer policy not applied')
    for name in NAMES:
        if len(re.findall(r'\b' + name + r'\b', header)) != 1:
            raise ValueError('selection producer missing/duplicate: ' + name)
    for path, text in [('declarations.inc', header), ('definitions.inc', cpp)]:
        if text.count((HERE / path).read_text().strip()) != 1:
            raise ValueError('producer source mismatch: ' + path)
