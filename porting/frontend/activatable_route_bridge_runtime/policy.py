"""Generator fragments for the route-facing UCommonActivatableWidget bridge."""
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
FLAG = 'activatable_route_bridge_runtime'
EVENTS = json.loads((HERE / 'original-events.json').read_text())


def validate_source(image_path):
    windows = json.loads((HERE / 'original-windows.json').read_text())['windows']
    fd = os.open(image_path, os.O_RDONLY)
    try:
        for window in windows:
            data = os.pread(fd, window['size'], int(window['va'], 16) - 0x140000000)
            if len(data) != window['size'] or hashlib.sha256(data).hexdigest() != window['sha256']:
                raise ValueError('activatable route source mismatch: ' + window['va'])
    finally:
        os.close(fd)


def apply(model, block, owner_fns):
    if model.get('name') != 'UCommonActivatableWidget':
        return model
    if (model.get('package') != '/Script/CommonUI'
            or block.get('block_va') != '0x145a004e0'
            or '0x1408079b0' not in owner_fns):
        raise ValueError('original activatable widget owner mismatch')
    names = {'NativeGetActionRouteOverride', 'NativeGetActionRouteInputConfig',
             'NativeGetActionRouteDesiredFocusTarget', 'NativeWidgetReleased',
             'ReleaseSlateResources', 'RebuildWidget', 'NativeConstruct',
             'NativeHandleBackAction', 'NativeDispatchBackAction'}
    if model.get(FLAG) or any(item.get('name') in names for group in
            ('functions', 'fields', 'helpers', 'static_helpers', 'members_decl_order')
            for item in model.get(group, [])):
        raise ValueError('activatable route bridge collision')
    model['includes'] = sorted(set(model.get('includes', [])) | {'CommonActionRouteTypes.h'})
    for name, expected in EVENTS.items():
        if [f for f in block.get('functions', []) if f.get('name') == name] != [expected]:
            raise ValueError('activatable route Blueprint event mismatch: ' + name)
        if any(f.get('name') == name for f in model['functions']):
            raise ValueError('activatable route Blueprint event collision: ' + name)
        model['functions'].append(dict(name=name, ret='UWidget*' if name == 'BP_GetDesiredFocusTarget' else 'bool',
            args=[], const=name == 'BP_GetDesiredFocusTarget', static=False, rpc=False, service=None,
            notify=False, splice=False, blueprint_event=True, access='protected',
            specifiers=['BlueprintImplementableEvent'], used={}))
    model['skipped'] = [s for s in model.get('skipped', []) if not
        (s.get('item') == 'function' and s.get('name') in EVENTS)]
    model['cpp_includes'] = sorted(set(model.get('cpp_includes', [])) | {'CommonActivatableWidgetRebuilt.h', 'CommonActionBindingData.h',
        'CommonInputModuleRuntime.h', 'UCommonInputSettings.h', 'UCommonUIInputData.h'})
    model[FLAG] = True
    return model


def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get(FLAG) else ''


def configure_model(model):
    if model.get(FLAG):
        model['includes'] = sorted(set(model.get('includes', [])) | {'CommonActionRouteTypes.h'})
    return model


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get(FLAG) else ''


def validate_output(model, header, cpp):
    if (not model.get(FLAG) or header.count(declarations(model).strip()) != 1
            or cpp.count(definitions(model).strip()) != 1):
        raise ValueError('activatable route bridge missing or duplicated')


def sources():
    return {name: (HERE / name).read_text() for name in
            ('CommonActivatableWidgetRebuilt.h', 'CommonActivatableWidgetRebuilt.cpp')}
