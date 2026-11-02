"""Fail-closed gates for the two CommonButton base selection notification slots."""
import json
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
FLAG = 'common_button_selection_notification_runtime'
HELPERS = ('FNativeSelectedChanged', 'NativeSelectedChanged', 'NativeOnSelected', 'NativeOnDeselected')
GROUPS = ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers')


def validate_classes(blocks, name_to_block, delegates):
    identity = CONTRACT['identity']
    va = identity['block_va']
    block = blocks.get(va)
    if name_to_block.get('UCommonButton') != va or block is None or any(
            block.get(k) != v for k, v in identity.items()):
        raise ValueError('CommonButton identity mismatch')
    for group, expected in [('properties', CONTRACT['property'])] + [
            ('functions', event) for event in CONTRACT['events']]:
        if [x for x in block.get(group, []) if x.get('name') == expected['name']] != [expected]:
            raise ValueError('CommonButton original record mismatch: ' + expected['name'])
    want = CONTRACT['delegate']
    related = [d for d in delegates if d.get('block_va') == want['block_va']
               or d.get('name') == want['name']
               or set(d.get('construct_fns', [])) & set(want['construct_fns'])]
    if related != [want]:
        raise ValueError('original selection delegate identity/signature mismatch or duplicate')


def validate_model(model):
    if (model.get('name') != 'UCommonButton' or model.get('package') != '/Script/CommonUI'
            or model.get('block_va') != CONTRACT['identity']['block_va']):
        raise ValueError('CommonButton model identity mismatch')
    for group in ('members_decl_order', 'members_table_order'):
        if group not in model:
            if group == 'members_decl_order':
                raise ValueError('dynamic delegate declaration list missing')
            continue
        if [m.get('decl') for m in model[group] if m.get('name') == 'OnSelectedChanged'] != [CONTRACT['member']]:
            raise ValueError('selection dynamic delegate member mismatch: ' + group)
    if not model.get('common_button_style_rebuild_runtime'):
        raise ValueError('shared text-style virtual adapter unavailable')


def apply(model, block, owner_fns):
    if model.get('name') != 'UCommonButton':
        return model
    validate_model(model)
    if block.get('block_va') != CONTRACT['identity']['block_va'] or '0x140805cb0' not in owner_fns:
        raise ValueError('CommonButton owner mismatch')
    if model.get(FLAG) or any(m.get('name') in HELPERS for group in GROUPS for m in model.get(group, [])):
        raise ValueError('selection notification helper collision')
    model[FLAG] = True
    return model


def validate_output(model, button_header, delegate_header):
    validate_model(model)
    if not model.get(FLAG):
        raise ValueError('selection notification policy not applied')
    for helper in HELPERS:
        if len(re.findall(r'\b' + re.escape(helper) + r'\b', button_header)) != (2 if helper == 'FNativeSelectedChanged' else 1):
            raise ValueError('rendered selection helper collision or omission: ' + helper)
    for event in CONTRACT['events']:
        name = event['name']
        declaration = 'UFUNCTION(BlueprintImplementableEvent)\n\tvoid ' + name + '();'
        if button_header.count(declaration) != 1 or len(re.findall(r'\b' + re.escape(name) + r'\b', button_header)) != 1:
            raise ValueError('BP event source mismatch: ' + name)
    if button_header.count('virtual void NativeOnCurrentTextStyleChanged();') != 1:
        raise ValueError('shared text-style virtual declaration mismatch')
    if (delegate_header.count(CONTRACT['delegate_header']) != 1 or
            len(re.findall(r'DECLARE_DYNAMIC_MULTICAST_DELEGATE\w*\s*\(\s*FCommonSelectedStateChanged\s*[,)]', delegate_header)) != 1):
        raise ValueError('selection dynamic delegate source mismatch')


def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get(FLAG) else ''


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get(FLAG) else ''
