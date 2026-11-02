"""Fail-closed original metadata gates for CommonButton double-click receiver."""
import json
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
FLAG = 'common_button_double_click_runtime'
HELPERS = ('NativeDoubleClicked', 'NativeOnDoubleClicked', 'NativeHandleDoubleClicked')
GROUPS = ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers')


def validate_classes(blocks, name_to_block, delegates):
    expected = CONTRACT['identity']
    va = expected['block_va']
    block = blocks.get(va)
    if name_to_block.get('UCommonButton') != va or block is None or any(block.get(k) != v for k, v in expected.items()):
        raise ValueError('CommonButton identity mismatch')
    for group, key in (('properties', 'property'), ('properties', 'bit'), ('functions', 'event'), ('functions', 'stop')):
        item = CONTRACT[key]
        if [x for x in block.get(group, []) if x.get('name') == item['name']] != [item]:
            raise ValueError('CommonButton ' + key + ' metadata mismatch')
    want = CONTRACT['delegate']
    related = [d for d in delegates if d.get('block_va') == want['block_va'] or d.get('name') == want['name'] or set(d.get('construct_fns', [])) & set(want['construct_fns'])]
    if related != [want]:
        raise ValueError('original delegate identity/signature mismatch or duplicate')


def validate_model(model):
    if (model.get('name') != 'UCommonButton' or model.get('package') != '/Script/CommonUI' or model.get('block_va') != CONTRACT['identity']['block_va']):
        raise ValueError('CommonButton model identity mismatch')
    for group in ('members_decl_order', 'members_table_order'):
        if group not in model:
            if group == 'members_decl_order':
                raise ValueError('declaration list missing')
            continue
        for name, key in (('OnButtonDoubleClicked', 'member'), ('bStopDoubleClickPropagation', 'bit_member')):
            if [m.get('decl') for m in model[group] if m.get('name') == name] != [CONTRACT[key]]:
                raise ValueError(name + ' member mismatch: ' + group)


def apply(model, block, owner_fns, native_execs=None):
    if model.get('name') != 'UCommonButton':
        return model
    validate_model(model)
    if block.get('block_va') != CONTRACT['identity']['block_va'] or '0x140805cb0' not in owner_fns:
        raise ValueError('CommonButton owner mismatch')
    if not isinstance(native_execs, dict) or native_execs.get('StopDoubleClickPropagation') != CONTRACT['exec']:
        raise ValueError('original StopDoubleClickPropagation native exec mismatch')
    if model.get('splice', {}).get('accepted', []).count('UCommonButton::StopDoubleClickPropagation') != 1:
        raise ValueError('StopDoubleClickPropagation snippet not accepted exactly once')
    if model.get(FLAG) or any(m.get('name') in HELPERS for group in GROUPS for m in model.get(group, [])):
        raise ValueError('double click helper collision')
    model[FLAG] = True
    return model


def validate_output(model, button_header, delegate_header, button_cpp):
    validate_model(model)
    if not model.get(FLAG):
        raise ValueError('double click policy not applied')
    for helper in HELPERS:
        if len(re.findall(r'\b' + re.escape(helper) + r'\b', button_header)) != 1:
            raise ValueError('rendered double click helper collision or omission: ' + helper)
    for name, decl in (('BP_OnDoubleClicked', 'UFUNCTION(BlueprintImplementableEvent)\n\tvoid BP_OnDoubleClicked();'), ('StopDoubleClickPropagation', 'UFUNCTION(BlueprintCallable)\n\tvoid StopDoubleClickPropagation();')):
        if button_header.count(decl) != 1 or len(re.findall(r'\b' + name + r'\b', button_header)) != 1:
            raise ValueError(name + ' declaration mismatch')
    if delegate_header.count(CONTRACT['delegate_header']) != 1 or len(re.findall(r'DECLARE_DYNAMIC_MULTICAST_DELEGATE\w*\s*\(\s*FCommonButtonClicked\s*[,)]', delegate_header)) != 1:
        raise ValueError('dynamic delegate source mismatch')
    body = (HERE / 'StopDoubleClickPropagation.inc').read_text().strip('\n')
    rendered = '\n'.join('\t' + line.rstrip() if line.strip() else '' for line in body.split('\n'))
    definition = 'void UCommonButton::StopDoubleClickPropagation()\n{\n' + rendered + '\n}'
    if button_cpp.count(definition) != 1 or len(re.findall(r'\bUCommonButton::StopDoubleClickPropagation\s*\(', button_cpp)) != 1:
        raise ValueError('StopDoubleClickPropagation definition mismatch')


def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get(FLAG) else ''


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get(FLAG) else ''
