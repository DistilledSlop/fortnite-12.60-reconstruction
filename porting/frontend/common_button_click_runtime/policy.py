"""Strict gates and source fragments for CommonButton base click virtuals.

Call every validation hook before writing any generated output; see HANDOFF.md.
"""
import json
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
HELPERS = ('NativeClicked', 'NativeOnClicked', 'NativeGetClickAnalytics',
           'FCommonButtonBaseClickProbeAccess')
GROUPS = ('functions', 'members_table_order', 'members_decl_order', 'fields',
          'helpers', 'static_helpers')
FLAG = 'common_button_base_click_runtime'


def validate_classes(blocks, name_to_block, delegates):
    expected = CONTRACT['identity']
    va = expected['block_va']
    block = blocks.get(va)
    if name_to_block.get('UCommonButton') != va or block is None or any(
            block.get(k) != v for k, v in expected.items()):
        raise ValueError('CommonButton identity mismatch')
    for group, key in (('properties', 'property'), ('functions', 'event')):
        item = CONTRACT[key]
        if [x for x in block.get(group, []) if x.get('name') == item['name']] != [item]:
            raise ValueError('CommonButton ' + key + ' mismatch')
    # Examine the full table, not just a by-VA map that could hide duplicates.
    want = CONTRACT['delegate']
    related = [d for d in delegates if d.get('block_va') == want['block_va']
               or d.get('name') == want['name']
               or set(d.get('construct_fns', [])) & set(want['construct_fns'])]
    if related != [want]:
        raise ValueError('original delegate identity/signature mismatch or duplicate')


def validate_model(model):
    if (model.get('name') != 'UCommonButton' or model.get('package') != '/Script/CommonUI'
            or model.get('block_va') != CONTRACT['identity']['block_va']):
        raise ValueError('CommonButton model identity mismatch')
    for group in ('members_decl_order', 'members_table_order'):
        if group not in model:
            if group == 'members_decl_order':
                raise ValueError('dynamic delegate declaration list missing')
            continue
        if [m.get('decl') for m in model[group] if m.get('name') == 'OnButtonClicked'] != [CONTRACT['member']]:
            raise ValueError('dynamic delegate member mismatch: ' + group)


def apply(model, block, owner_fns):
    if model.get('name') != 'UCommonButton':
        return model
    validate_model(model)
    if block.get('block_va') != CONTRACT['identity']['block_va'] or '0x140805cb0' not in owner_fns:
        raise ValueError('CommonButton owner mismatch')
    if model.get(FLAG) or any(m.get('name') in HELPERS for group in GROUPS
                              for m in model.get(group, [])):
        raise ValueError('click helper collision')
    model[FLAG] = True
    return model


def validate_output(model, button_header, delegate_header, subsystem_header):
    """Check fully rendered in-memory headers, including all other policy insertions."""
    validate_model(model)
    if not model.get(FLAG):
        raise ValueError('click policy not applied')
    for helper in HELPERS:
        if len(re.findall(r'\b' + re.escape(helper) + r'\b', button_header)) != 1:
            raise ValueError('rendered click helper collision or omission: ' + helper)
    event = 'UFUNCTION(BlueprintImplementableEvent)\n\tvoid BP_OnClicked();'
    if button_header.count(event) != 1 or len(re.findall(r'\bBP_OnClicked\b', button_header)) != 1:
        raise ValueError('BP event source mismatch')
    if (delegate_header.count(CONTRACT['delegate_header']) != 1 or
            len(re.findall(r'DECLARE_DYNAMIC_MULTICAST_DELEGATE\w*\s*\(\s*FCommonButtonClicked\s*[,)]', delegate_header)) != 1):
        raise ValueError('dynamic delegate source mismatch')
    if subsystem_header.count('class COMMONUI_API UCommonUISubsystem : public UGameInstanceSubsystem') != 1:
        raise ValueError('subsystem class source mismatch')


def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get(FLAG) else ''


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get(FLAG) else ''
