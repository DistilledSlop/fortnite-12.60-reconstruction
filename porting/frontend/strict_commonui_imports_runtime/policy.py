"""Strict CommonUI imports: original Panel records, with their real producers.

Apply only after Panel intro/outro and the connected action router policies.
The button/global-handler closure is deliberately not enabled by this policy.
"""
import hashlib
import json
import os
import struct
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'panel-original-policy.json').read_text())
FLAG = 'strict_commonui_panel_input_handlers_runtime'
NAMES = set(CONTRACT['functions'])
NATIVE_NAMES = {'NativeDestruct', 'NativeActionBindingRows', 'NativeSetInputActionHandler',
    'NativeIsHoldingInputAction', 'NativeNotifyInputActionsChanged', 'NativeRemoveRoutedInputActionHandlers'}
GROUPS = ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers')


def validate_original_image(path):
    fd = os.open(path, os.O_RDONLY)
    try:
        for item in CONTRACT['windows']:
            length = int(item['size'], 16)
            data = os.pread(fd, length, int(item['va'], 16) - 0x140000000)
            if len(data) != length or hashlib.sha256(data).hexdigest() != item['sha256']:
                raise ValueError('original Panel input source mismatch: ' + item['va'])
    finally:
        os.close(fd)


def validate_continuation_original_image(path):
    contract = json.loads((HERE / 'legacy-continuation-original.json').read_text())
    fd = os.open(path, os.O_RDONLY)
    try:
        for item in contract['windows']:
            length = int(item['size'], 16)
            data = os.pread(fd, length, int(item['va'], 16) - 0x140000000)
            if len(data) != length or hashlib.sha256(data).hexdigest() != item['sha256']:
                raise ValueError('original legacy continuation mismatch: ' + item['va'])
        for item in contract['vtables']:
            offset = int(item['table'], 16) + int(item['slot'], 16) - 0x140000000
            if os.pread(fd, 8, offset) != struct.pack('<Q', int(item['target'], 16)):
                raise ValueError('original legacy virtual mismatch')
    finally:
        os.close(fd)


def validate_classes(blocks, name_to_block, native_funcs):
    if name_to_block.get('UCommonActivatablePanel') != CONTRACT['block_va']:
        raise ValueError('Panel input owner mismatch')
    block = blocks[CONTRACT['block_va']]
    for name, expected in CONTRACT['functions'].items():
        if [f for f in block['functions'] if f['name'] == name] != [expected]:
            raise ValueError('original Panel input function mismatch: ' + name)
    for name, expected in CONTRACT['native_execs'].items():
        if native_funcs.get('UCommonActivatablePanel', {}).get(name) != expected:
            raise ValueError('original Panel input exec mismatch: ' + name)


def apply(model, block, owner_fns):
    name = model.get('name')
    if name == 'UCommonInputManager':
        if model.get('package') != '/Script/CommonUI' or block.get('block_va') != '0x145a05fd0' or '0x14080e9a0' not in owner_fns:
            raise ValueError('Panel binding notification manager owner mismatch')
        if not model.get('common_input_suspension_runtime'):
            raise ValueError('Panel binding notification requires original manager policy')
        if model.get(FLAG) or any(i.get('name') == 'NativePanelInputActionsChanged'
                for group in GROUPS for i in model.get(group, [])):
            raise ValueError('Panel binding notification collision')
        model[FLAG] = True
        return model
    if name != 'UCommonActivatablePanel':
        return model
    if (model.get('package'), model.get('parent'), block.get('block_va')) != (
            '/Script/CommonUI', 'UCommonActivatableWidget', CONTRACT['block_va']):
        raise ValueError('Panel input identity mismatch')
    if CONTRACT['owner'] not in owner_fns:
        raise ValueError('Panel input function owner mismatch')
    if not model.get('panel_outro_runtime'):
        raise ValueError('Panel input requires actual legacy record and held cleanup')
    if model.get(FLAG) or any(i.get('name') in NAMES | NATIVE_NAMES
            for group in GROUPS for i in model.get(group, [])):
        raise ValueError('Panel input declaration collision')
    for function, kind in [('0x140809720', 'Delegate'), ('0x1408097b0', 'Delegate'),
                           ('0x1446f2880', 'Struct')]:
        model.setdefault('used', {}).setdefault(function, set()).add(kind)
    model['forward_decls'] = sorted(set(model.get('forward_decls', [])) | {'UDataTable', 'UCommonPopupMenu'})
    model['skipped'] = [item for item in model.get('skipped', []) if not
        (item.get('item') == 'function' and item.get('name') in NAMES)]
    # These reflected functions have exact hand-written declarations below;
    # do not append duplicate declarations to model['functions'].
    model['strict_commonui_panel_functions_emitted'] = tuple(sorted(NAMES))
    model[FLAG] = True
    return model


def validate_dependencies(models):
    """Call on the complete models before output. No registration fallback."""
    for name in ('UCommonUserWidget', 'UCommonUIActionRouter', 'UCommonActivatableWidget'):
        if not models.get(name, {}).get('common_action_route_connected_runtime'):
            raise ValueError('Panel input requires real connected route dependency: ' + name)
    if not models.get('UCommonInputManager', {}).get(FLAG):
        raise ValueError('Panel input requires manager binding-change multicast')


def header_prelude(model):
    result = dict(model)
    if model.get(FLAG) and model.get('name') == 'UCommonActivatablePanel':
        result['includes'] = sorted(set(model.get('includes', [])) | {
            'CommonActionBindingData.h', 'FCommonActionCommited.h', 'FCommonActionProgressSingle.h'})
    return result


def declarations(model):
    if not model.get(FLAG):
        return ''
    if model.get('name') == 'UCommonInputManager':
        return (HERE / 'manager-action-notification-declarations.inc').read_text()
    return (HERE / 'panel-public-declarations.inc').read_text() + (HERE / 'panel-declarations.inc').read_text()


def finalize_header(model, header):
    """Place recovered +310 state before existing +360 state exactly once.

    The legacy map stays owned by the existing Panel outro policy. The new map
    is not UPROPERTY: original state owns binding IDs, not UObject references.
    This preserves relative C++ construction/destruction order without claiming
    portable C++ byte layout matches the shipping ABI.
    """
    if model.get(FLAG) and model.get('name') == 'UCommonActivatablePanel':
        anchor = '    FCommonPanelActionMap NativeActionRecords;'
        state = (HERE / 'panel-routed-state-declarations.inc').read_text()
        if header.count(anchor) != 1 or ' NativeActionBindingRows;' in header:
            raise ValueError('Panel existing legacy/routed state missing or duplicated')
        header = header.replace(anchor, state + anchor)
    return header


def definitions(model):
    return (HERE / 'panel-definitions.inc').read_text() if model.get(FLAG) and model.get('name') == 'UCommonActivatablePanel' else ''


def validate_output(model, header, cpp):
    if not model.get(FLAG):
        return
    if header.count(declarations(model).strip()) != 1:
        raise ValueError('Panel input declaration splice mismatch')
    if model.get('name') == 'UCommonActivatablePanel':
        state = (HERE / 'panel-routed-state-declarations.inc').read_text().strip()
        if header.count(state) != 1 or header.count(' FCommonPanelActionMap NativeActionRecords;') != 1:
            raise ValueError('Panel input actual native state missing or duplicated')
        if header.index(' NativeActionBindingRows;') >= header.index(' FCommonPanelActionMap NativeActionRecords;'):
            raise ValueError('Panel routed and legacy map source order mismatch')
        if any(item.get('item') == 'function' and item.get('name') in NAMES for item in model.get('skipped', [])):
            raise ValueError('emitted Panel input function remains in skipped inventory')
    body = definitions(model).strip()
    if body and cpp.count(body) != 1:
        raise ValueError('Panel input definition splice mismatch')
