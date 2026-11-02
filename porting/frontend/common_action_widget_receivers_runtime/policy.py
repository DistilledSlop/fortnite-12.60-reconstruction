"""Fail-closed source and reflection gates for the connected widget receivers."""
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
LIFECYCLE = json.loads((HERE.parent / 'common_action_widget_key_lifecycle_evidence/original-contract.json').read_text())
# Bounded windows for refresh, owner resolution, first-row info selection.
EXTRA = json.loads((HERE / 'refresh-windows.json').read_text())
ADDITIONAL = json.loads((HERE / 'additional-contract.json').read_text())


def validate_source(image_path):
    fd = os.open(image_path, os.O_RDONLY)
    try:
        for window in LIFECYCLE['windows'] + EXTRA:
            data = os.pread(fd, window['size'], int(window['va'], 16) - 0x140000000)
            if len(data) != window['size'] or hashlib.sha256(data).hexdigest() != window['sha256']:
                raise ValueError('CommonActionWidget source window differs: ' + window['name'])
    finally:
        os.close(fd)


def apply(model, block, owner_fns, native_execs):
    if model['name'] != 'UCommonActionWidget':
        return model
    if (model.get('package') != '/Script/CommonUI' or model.get('parent') != 'UWidget'
            or block.get('block_va') != LIFECYCLE['class_block']
            or block.get('class_noregister_fn') != LIFECYCLE['class_noregister_fn']
            or '0x140807790' not in owner_fns):
        raise ValueError('CommonActionWidget owner differs')
    names = ('SetInputAction', 'SetInputActions')
    if [f for f in block.get('functions', []) if f.get('name') in names] != LIFECYCLE['functions']:
        raise ValueError('CommonActionWidget action setter reflection differs')
    if any(native_execs.get(name) != LIFECYCLE['native'][name] for name in names):
        raise ValueError('CommonActionWidget action setter native join differs')
    if [p.get('decl') for p in model.get('members_decl_order', []) if p.get('name') == 'InputActions'] != ['TArray<FDataTableRowHandle> InputActions;']:
        raise ValueError('InputActions declaration differs')
    extra_names = ('GetDisplayText', 'IsHeldAction', 'SetIconRimBrush')
    if [f for f in block.get('functions', []) if f.get('name') in extra_names] != ADDITIONAL['functions']:
        raise ValueError('CommonActionWidget additional reflection differs')
    if any(native_execs.get(name) != ADDITIONAL['native'][name] for name in extra_names):
        raise ValueError('CommonActionWidget additional native join differs')
    aliases = set(names) | set(extra_names) | {'NativeSetBindingKey', 'NativeRefreshInputAction', 'NativeBindInputMethodChanged',
                            'NativeResolveInputSubsystem', 'NativeHandleInputMethodChanged', 'NativeBindingKey', 'NativeHidden'}
    if model.get('common_action_widget_receivers_runtime') or any(
        m.get('name') in aliases for group in ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers') for m in model.get(group, [])):
        raise ValueError('CommonActionWidget receiver name collision')
    for name, arg in [('SetInputAction', 'FDataTableRowHandle InputActionRow'),
                      ('SetInputActions', 'TArray<FDataTableRowHandle> NewInputActions')]:
        model['functions'].append({'name': name, 'rpc': False, 'service': None, 'ret': 'void', 'args': [arg],
                                   'const': False, 'notify': False, 'splice': False, 'blueprint_event': False,
                                   'access': 'public', 'runtime_implementation': True,
                                   'specifiers': ['BlueprintCallable'], 'used': {}})
    for name, ret, args, const in [
        ('GetDisplayText', 'FText', [], True), ('IsHeldAction', 'bool', [], True),
        ('SetIconRimBrush', 'void', ['FSlateBrush InIconRimBrush'], False)]:
        model['functions'].append({'name': name, 'rpc': False, 'service': None, 'ret': ret, 'args': args,
                                   'const': const, 'notify': False, 'splice': False, 'blueprint_event': False,
                                   'access': 'public', 'runtime_implementation': True,
                                   'specifiers': ['BlueprintCallable', 'BlueprintPure'] if const else ['BlueprintCallable'],
                                   'used': {}})
    model['common_action_widget_receivers_runtime'] = True
    return model


def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get('common_action_widget_receivers_runtime') else ''


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get('common_action_widget_receivers_runtime') else ''
