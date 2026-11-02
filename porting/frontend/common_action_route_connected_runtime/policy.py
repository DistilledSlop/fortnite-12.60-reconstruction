"""Generator fragments for the connected original CommonUI routing component.

Primary owns generator hooks, canonical-registry edits, peer source merge and
builds. This module performs no file writes. Names describe native identities;
the source declarations are not an original-binary-layout compatibility shim.
"""
import hashlib
import json
import os
from pathlib import Path
import re

HERE = Path(__file__).resolve().parent
FLAG = 'common_action_route_connected_runtime'

CONTRACTS = {
    'UCommonUIActionRouter': {
        'block': '0x145a07c70', 'getter': '0x1408132b0', 'parent': 'ULocalPlayerSubsystem',
        'decl': ('UCommonUIActionRouter.members.inc',),
        'body': ('UCommonUIActionRouter.bodies.inc', 'UCommonUIActionRouter.discovery.inc', 'UCommonUIActionRouter.lifecycle.inc'),
        'includes': ('CommonActionBindingRoute.h', 'CommonActionRouteTypes.h'),
        'cpp_includes': ('CommonActivatableWidgetRebuilt.h', 'UCommonActivatableWidget.h',
                         'UCommonUserWidget.h', 'UCommonInputSubsystem.h', 'UCommonUISubsystem.h',
                         'UCommonUIInputSettings.h', 'Subsystems/SubsystemCollection.h',
                         'Engine/GameInstance.h', 'Engine/LocalPlayer.h', 'Engine/GameViewportClient.h',
                         'GameFramework/PlayerController.h', 'Framework/Application/SlateApplication.h',
                         'Framework/Application/SlateUser.h', 'Slate/SObjectWidget.h',
                         'Slate/SGameLayerManager.h', 'Widgets/SViewport.h', 'Containers/Ticker.h',
                         'UObject/UObjectGlobals.h', 'Input/Events.h'),
    },
    'UCommonUserWidget': {
        'block': '0x145a0bb40', 'getter': '0x1408092e0', 'parent': 'UUserWidget',
        'owner_fn': '0x140816d40',
        'decl': ('UCommonUserWidget.registration.members.inc',),
        'body': ('UCommonUserWidget.registration.bodies.inc',),
        'includes': ('CommonActionBindingData.h',),
        'cpp_includes': ('UCommonUIActionRouter.h', 'Engine/LocalPlayer.h'),
    },
    'UCommonUISubsystem': {
        'block': '0x145a0b9c0', 'getter': '0x1408168a0', 'parent': 'UGameInstanceSubsystem',
        'owner_fn': '0x140816b10',
        'decl': ('UCommonUISubsystem.legacy.members.inc',), 'body': (),
        'includes': ('CommonActionRouteTypes.h',), 'cpp_includes': (),
    },
    'UCommonActivatableWidget': {
        'block': '0x145a004e0', 'getter': '0x140805ec0', 'parent': 'UCommonUserWidget',
        'owner_fn': '0x1408079b0',
        'decl': ('UCommonActivatableWidget.destruct.members.inc',),
        'body': ('UCommonActivatableWidget.destruct.bodies.inc',),
        'includes': (), 'cpp_includes': ('UCommonUIActionRouter.h', 'Engine/LocalPlayer.h'),
    },
}

# Original field order and identity. Fragment generation preserves the declared
# native router sequence; reflected fields stay in the generator's established
# order. UCommonUserWidget owns exactly one private binding array, original+230.
NATIVE_STATE_ORDER = {
    'UCommonUIActionRouter': (
        ('NativePendingActionOwners', 0x30), ('NativePendingActionWidgets', 0x40),
        ('NativeActionRouteRoots', 0x50), ('NativeActiveRouteRoot', 0x60),
        ('NativeStandaloneActionRoute', 0x70), ('NativeActionInputConfig', 0x80),
        ('NativeActionInputPreProcessor', 0x88), ('bNativeActionRoutingBlocked', 0x98),
        ('bNativeActionRoutingEnabled', 0x99), ('NativeActionRouteTicker', 0xa0),
        ('NativeActionBindingsChanged', 0xa8), ('NativeActionInputModeChanged', 0xc0),
        ('NativeActionRouteOverrideChanged', 0xd8), ('NativeQueuedActionKeys', 0xf0)),
    'UCommonUserWidget': (('NativeRegisteredActionBindingHandles', 0x230),),
    'UCommonUISubsystem': (('NativeLegacyInputManagerChanged', 0x58),),
}


def _read(names):
    return '\n'.join((HERE / name).read_text().rstrip() for name in names) + ('\n' if names else '')


def validate_source(image_path=None):
    policy = json.loads((HERE / 'original-windows.json').read_text())
    fd = os.open(image_path or policy['image'], os.O_RDONLY)
    try:
        for item in policy['windows']:
            size = int(item['size'], 16)
            if not 0 < size <= 0x2000:
                raise ValueError('unbounded route evidence window')
            data = os.pread(fd, size, int(item['start'], 16) - int(policy['base'], 16))
            if len(data) != size or hashlib.sha256(data).hexdigest() != item['sha256']:
                raise ValueError('original route source mismatch: ' + item['name'])
    finally:
        os.close(fd)


def apply(model, block, owner_fns):
    contract = CONTRACTS.get(model.get('name'))
    if contract is None:
        return model
    if (model.get('package') != '/Script/CommonUI'
            or model.get('parent') != contract['parent']
            or block.get('block_va') != contract['block']
            or block.get('class_noregister_fn') != contract['getter']
            or (contract.get('owner_fn') and contract['owner_fn'] not in owner_fns)):
        raise ValueError('connected CommonUI route original class identity mismatch: ' + model['name'])
    declarations_text = _read(contract['decl'])
    native_names = set(re.findall(r'\b(?:Native|bNative)[A-Za-z0-9_]+\b', declarations_text))
    # Native override names are also checked; peer activatable policy owns other
    # methods and is allowed to coexist with this one NativeDestruct override.
    native_names.update(re.findall(r'virtual\s+(?:\w+[\s*&]+)+([A-Za-z_][A-Za-z0-9_]*)\s*\(', declarations_text))
    if model.get(FLAG) or any(item.get('name') in native_names
            for group in ('functions', 'fields', 'helpers', 'static_helpers', 'members_decl_order')
            for item in model.get(group, [])):
        raise ValueError('connected CommonUI route declaration collision: ' + model['name'])
    if model['name'] == 'UCommonUserWidget':
        pointer_fields = [item for item in block.get('properties', []) if item.get('name') == 'bConsumePointerInput']
        pointer_members = [item for item in model.get('members_decl_order', []) if item.get('name') == 'bConsumePointerInput']
        if (len(pointer_fields) != 1 or pointer_fields[0].get('set_bit_fn') != '0x1408179d0'
                or len(pointer_members) != 1 or pointer_members[0].get('decl') != 'bool bConsumePointerInput;'):
            raise ValueError('original CommonUserWidget pointer-input field identity differs')
        # Original setter1408179d0 supplies the Bool's otherwise absent offset.
        # The standard generator field merger can therefore put actual private
        # handle storage+230 before the reflected pointer-input Bool+250.
        pointer_members[0]['offset'] = 0x250
        model.setdefault('fields', []).append({
            'name': 'NativeRegisteredActionBindingHandles',
            'decl': 'TArray<int32> NativeRegisteredActionBindingHandles;',
            'access': 'private', 'offset': 0x230})
    for member, _ in NATIVE_STATE_ORDER.get(model['name'], ()):
        if model['name'] == 'UCommonUserWidget':
            continue
        if not re.search(r'\b' + re.escape(member) + r'\b[^;]*;', declarations_text):
            raise ValueError('missing actual native state: ' + member)
    model['includes'] = sorted(set(model.get('includes', [])) | set(contract['includes']))
    model['cpp_includes'] = sorted(set(model.get('cpp_includes', [])) | set(contract['cpp_includes']))
    model[FLAG] = True
    return model


def declarations(model):
    return _read(CONTRACTS[model['name']]['decl']) if model.get(FLAG) else ''


def configure_model(model):
    if model.get(FLAG):
        model['includes'] = sorted(set(model.get('includes', [])) | set(CONTRACTS[model['name']]['includes']))
    return model


def definitions(model):
    if not model.get(FLAG):
        return ''
    result = _read(CONTRACTS[model['name']]['body'])
    if model['name'] == 'UCommonUIActionRouter':
        peer = HERE.parent / 'common_action_route_dispatch_runtime'
        result += '\n' + (peer / 'UCommonUIActionRouter.dispatch.inc').read_text()
        result += '\n' + (peer / 'UCommonUIActionRouter.tick.inc').read_text()
    return result


def validate_output(model, header, source):
    if not model.get(FLAG):
        return
    for text, output in ((declarations(model), header), (definitions(model), source)):
        if text.strip() and output.count(text.strip()) != 1:
            raise ValueError('connected route fragment missing/duplicated: ' + model['name'])
    previous = -1
    state_text = header if model['name'] == 'UCommonUserWidget' else declarations(model)
    for member, _ in NATIVE_STATE_ORDER.get(model['name'], ()):
        locations = list(re.finditer(r'(?m)^[ \t]*[^{};\n]+\b' + re.escape(member) + r'\b[^{};\n]*;', state_text))
        if len(locations) != 1 or locations[0].start() <= previous:
            raise ValueError('connected native state declaration order mismatch: ' + member)
        previous = locations[0].start()
    if model['name'] == 'UCommonUserWidget':
        if header.index('TArray<int32> NativeRegisteredActionBindingHandles;') > header.index('bool bConsumePointerInput;'):
            raise ValueError('CommonUserWidget private handle storage is out of original field order')
    if model['name'] in ('UCommonUserWidget', 'UCommonActivatableWidget'):
        if source.count('void ' + model['name'] + '::NativeDestruct()') != 1:
            raise ValueError('native destruction override missing/duplicated')


def sources():
    """Standalone owned files only; primary merges peer dispatch/cursor sources."""
    return {name: (HERE / name).read_text() for name in
            ('CommonActionRouteTypes.h', 'CommonActionBindingRoute.h', 'CommonActionBindingRoute.cpp')}


def merged_sources():
    result = sources()
    result['CommonActionBindingRoute.cpp'] += '\n' + (HERE.parent / 'common_analog_cursor_tick_runtime' / 'CommonActionBindingRoute.scroll.bodies.inc').read_text()
    peers = {
        'common_action_route_dispatch_runtime': ('CommonActionBindingRoute.dispatch.cpp',),
        'common_analog_cursor_runtime': ('FCommonAnalogCursor.h', 'FCommonAnalogCursor.cpp'),
        'common_analog_cursor_tick_runtime': ('FCommonAnalogCursor.tick.cpp',),
    }
    for folder, filenames in peers.items():
        for filename in filenames:
            result[filename] = (HERE.parent / folder / filename).read_text()
    return result


def registry_cleanup_contract():
    """Append to canonical existing CommonActionBindingData, never create a map.

    The declaration belongs INSIDE namespace CommonActionBindingData in the
    header. The fully qualified definition is appended at file scope in the
    canonical registry CPP and accesses its existing anonymous-namespace Records.
    """
    return {
        'header_namespace': 'CommonActionBindingData',
        'header_declarations': (HERE / 'CommonActionBindingData.cleanup.declarations.inc').read_text(),
        'source_includes': ('CommonActionBindingRoute.h',),
        'source_definitions': (HERE / 'CommonActionBindingData.cleanup.definitions.inc').read_text(),
        'required_existing_storage': 'TMap<int32, FCommonActionBindingRecord> Records;',
    }
