"""Fail-closed original and rendered-source gates for CommonButton receivers.

Call all gates before generation writes; the integrator must additionally validate
its Initialize and action-widget lifecycle policies and install the panel friend.
"""
import hashlib
import json
import os
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
FLAG = 'common_button_focus_selection_runtime'
HELPERS = ('NativeHovered', 'NativeUnhovered', 'NativeFocusReceived',
           'HandleButtonClicked', 'HandleButtonPressed', 'HandleButtonReleased',
           'NativeHandleFocusReceived', 'NativeOnHovered', 'NativeOnUnhovered',
           'NativeUpdateActionWidget', 'NativeDispatchTriggeredAction',
           'NativeRefreshActionWidgetVisibility', 'NativeSetSelectedInternal',
           'NativeSetIsSelected', 'NativeTriggeredInputAction')
GROUPS = ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers')


def validate_original_image(path):
    fd = os.open(path, os.O_RDONLY)
    try:
        for item in CONTRACT['windows']:
            data = os.pread(fd, item['length'], int(item['va'], 16) - 0x140000000)
            if len(data) != item['length'] or hashlib.sha256(data).hexdigest() != item['sha256']:
                raise ValueError('original instruction mismatch at ' + item['va'])
    finally:
        os.close(fd)


def validate_classes(blocks, name_to_block, native_funcs):
    identity = CONTRACT['identity']
    block = blocks.get(identity['block_va'])
    if name_to_block.get('UCommonButton') != identity['block_va'] or block is None or any(
            block.get(k) != v for k, v in identity.items()):
        raise ValueError('CommonButton identity mismatch')
    for group in ('properties', 'functions'):
        for expected in CONTRACT[group]:
            if [x for x in block.get(group, []) if x.get('name') == expected['name']] != [expected]:
                raise ValueError('original record mismatch: ' + expected['name'])
    for name, item in CONTRACT['external_properties'].items():
        external = blocks.get(item['block_va'])
        if (name_to_block.get(name) != item['block_va'] or not external or
                external.get('class_noregister_fn') != item['class_noregister_fn'] or
                external.get('package_fn') != item['package_fn'] or
                [x for x in external.get('properties', []) if x.get('name') == item['property']['name']] != [item['property']]):
            raise ValueError('external class/property mismatch: ' + name)
    owner = native_funcs.get('UCommonButton', {})
    for name, expected in CONTRACT['native_functions'].items():
        if owner.get(name) != expected:
            raise ValueError('native exec mismatch: ' + name)


def validate_model(model):
    if (model.get('name') != 'UCommonButton' or model.get('package') != '/Script/CommonUI'
            or model.get('block_va') != CONTRACT['identity']['block_va']):
        raise ValueError('CommonButton model identity mismatch')
    for dependency in ('common_button_style_rebuild_runtime', 'common_button_selection_notification_runtime',
                       'common_button_base_click_runtime', 'common_button_feedback_sound_identity_proven'):
        if not model.get(dependency):
            raise ValueError('missing shared receiver dependency: ' + dependency)


def apply(model, block, owner_fns):
    if model.get('name') != 'UCommonButton': return model
    validate_model(model)
    if block.get('block_va') != CONTRACT['identity']['block_va'] or '0x140805cb0' not in owner_fns:
        raise ValueError('CommonButton owner mismatch')
    if model.get(FLAG) or any(m.get('name') in HELPERS for group in GROUPS for m in model.get(group, [])):
        raise ValueError('receiver helper collision')
    model[FLAG] = True
    return model


def validate_output(model, button_header, panel_header, action_widget_header):
    validate_model(model)
    if not model.get(FLAG): raise ValueError('receiver policy not applied')
    for name in HELPERS:
        if len(re.findall(r'\b' + re.escape(name) + r'\b', button_header)) != 1:
            raise ValueError('receiver collision or omission: ' + name)
    for name in ('BP_OnHovered', 'BP_OnUnhovered'):
        if button_header.count('UFUNCTION(BlueprintImplementableEvent)\n\tvoid ' + name + '();') != 1:
            raise ValueError('hover event mismatch: ' + name)
    for name in ('HandleButtonClicked', 'HandleButtonPressed', 'HandleButtonReleased'):
        if button_header.count('UFUNCTION()\n    void ' + name + '();') != 1:
            raise ValueError('callback declaration mismatch: ' + name)
    if panel_header.count('friend class UCommonButton;') != 1 or 'FCommonPanelActionMap NativeActionRecords;' not in panel_header:
        raise ValueError('shared panel map inaccessible')
    for name in ('NativeHidden', 'NativeRefreshInputAction', 'SetInputAction'):
        if not re.search(r'\b' + name + r'\b', action_widget_header):
            raise ValueError('actual action widget dependency missing: ' + name)


def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get(FLAG) else ''


def definitions(model):
    if not model.get(FLAG): return ''
    return (HERE / 'definitions.inc').read_text() + '\n' + (HERE / 'routing-definitions.inc').read_text()
