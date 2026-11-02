"""Fail-closed hooks for the recovered widget brush and unreflected gamepad getter."""
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
FIELDS = json.loads((HERE / 'original-fields.json').read_text())
ICON = json.loads((HERE.parent / 'common_action_widget_icon_candidate/original-contract.json').read_text())
TYPES = json.loads((HERE.parent / 'common_input_settings_cache_runtime/original-types.json').read_text())


def validate_source(image_path):
    with Path(image_path).open('rb') as image:
        for va, raw in {**ICON['windows'], '0x14081b530': '0fb6417ac3'}.items():
            expected = bytes.fromhex(raw)
            image.seek(int(va, 16) - 0x140000000)
            if image.read(len(expected)) != expected:
                raise ValueError('CommonActionWidget/gamepad original code differs: ' + va)


def validate_types(gate):
    info = gate.info('0x14081edf0')
    if (info is None or info.fn != '0x14081edf0' or info.kind != 'enum'
            or info.cpp_name != 'ECommonGamepadType' or info.package != '/Script/CommonInput'
            or info.rec != TYPES['ECommonGamepadType'] or gate.reason(info) is not None):
        raise ValueError('original ECommonGamepadType identity or availability differs')


def apply_gamepad(model, block, owner_fns):
    if model['name'] != 'UCommonInputSubsystem':
        return model
    if (model.get('package') != '/Script/CommonInput' or model.get('parent') != 'ULocalPlayerSubsystem'
            or block.get('block_va') != '0x145a0e0d0' or '0x14081e010' not in owner_fns):
        raise ValueError('CommonInputSubsystem owner differs')
    if [p for p in block.get('properties', []) if p.get('name') == 'GamepadInputType' or p.get('enum_underlying_of') == 'GamepadInputType'] != FIELDS['gamepad_properties']:
        raise ValueError('original gamepad field differs')
    if [p.get('decl') for p in model.get('members_decl_order', []) if p.get('name') == 'GamepadInputType'] != ['ECommonGamepadType GamepadInputType;']:
        raise ValueError('gamepad field declaration differs')
    if model.get('common_action_gamepad_runtime') or any(m.get('name') == 'NativeGetGamepadInputType' for group in ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers') for m in model.get(group, [])):
        raise ValueError('gamepad alias collision')
    model['common_action_gamepad_runtime'] = True
    return model


def apply_icon(model, block, owner_fns, native_execs):
    if model['name'] != 'UCommonActionWidget':
        return model
    if (model.get('package') != '/Script/CommonUI' or model.get('parent') != 'UWidget'
            or block.get('block_va') != ICON['class_block'] or block.get('class_noregister_fn') != ICON['class_noregister_fn']
            or '0x140807790' not in owner_fns
            or [f for f in block.get('functions', []) if f.get('name') == 'GetIcon'] != [ICON['function']]
            or [p for p in block.get('properties', []) if p.get('name') == 'InputActions'] != FIELDS['input_actions']
            or native_execs.get('GetIcon') != ICON['native_func']):
        raise ValueError('original GetIcon owner, signature, field or exec differs')
    if [m.get('decl') for m in model.get('members_decl_order', []) if m.get('name') == 'InputActions'] != ['TArray<FDataTableRowHandle> InputActions;']:
        raise ValueError('InputActions declaration differs')
    # The eventual lifecycle policy must define this field and prove all mutations.
    # Until then this is deliberately unavailable, not a permanently -1 substitute.
    if not model.get('common_action_binding_key_lifecycle_verified'):
        raise ValueError('GetIcon requires the verified widget binding-key lifecycle')
    if model.get('common_action_widget_icon_runtime') or any(m.get('name') == 'GetIcon' for group in ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers') for m in model.get(group, [])):
        raise ValueError('GetIcon declaration collision')
    model['functions'].append({'name':'GetIcon', 'rpc':False, 'service':None, 'ret':'FSlateBrush', 'args':[],
        'const':True, 'notify':False, 'splice':False, 'blueprint_event':False, 'access':'public',
        'runtime_implementation':True, 'specifiers':['BlueprintCallable','BlueprintPure'], 'used':{}})
    model['common_action_widget_icon_runtime'] = True
    return model


def gamepad_declarations(model):
    return (HERE / 'gamepad-declarations.inc').read_text() if model.get('common_action_gamepad_runtime') else ''


def gamepad_definitions(model):
    return (HERE / 'gamepad-definitions.inc').read_text() if model.get('common_action_gamepad_runtime') else ''


def icon_definitions(model):
    return (HERE / 'icon-definitions.inc').read_text() if model.get('common_action_widget_icon_runtime') else ''
