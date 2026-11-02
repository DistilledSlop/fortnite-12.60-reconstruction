"""Source-guarded CommonInput modality producer and Slate lifecycle policy."""
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
WINDOWS = json.loads((HERE / 'original-windows.json').read_text())['windows']
TYPES = json.loads((HERE.parent / 'common_input_settings_cache_runtime' / 'original-types.json').read_text())
SETTERS = {
    'SetCurrentInputType': ('ECommonInputType NewInputType', '0x14081ee20'),
    'SetGamepadInputType': ('ECommonGamepadType InGamepadInputType', '0x14081edf0'),
}
MEMBERS = {
    'bIsGamepadSimulatedClick': 'bool bIsGamepadSimulatedClick;',
    'CurrentInputLocks': 'TMap<FName, ECommonInputType> CurrentInputLocks;',
    'GamepadInputType': 'ECommonGamepadType GamepadInputType;',
    'CurrentInputType': 'ECommonInputType CurrentInputType;',
    'LastInputType': 'ECommonInputType LastInputType;',
    'LastTimeInputMethodThrashingBegan': 'double LastTimeInputMethodThrashingBegan;',
    'LastInputMethodChangeTime': 'double LastInputMethodChangeTime;',
    'NumberOfInputMethodChangesRecently': 'int32 NumberOfInputMethodChangesRecently;',
    'OnInputMethodChanged': 'FInputMethodChangedDelegate OnInputMethodChanged;',
}
ALIASES = set(SETTERS) | {'Initialize', 'Deinitialize', 'NativeSetInputLock',
    'NativeBroadcastInputMethodChanged', 'NativeInputTypeOverride', 'NativeSetInputTypeFilter',
    'NativeTickPreprocessorOrder', 'NativeHandleShowKeysChanged', 'NativeInputPreprocessor',
    'NativePreprocessorTicker'}
CPP_INCLUDES = {'CommonInputModuleRuntime.h', 'UCommonInputSettings.h', 'CommonInputPreprocessor.h',
    'Containers/Ticker.h', 'Engine/World.h', 'Framework/Application/SlateApplication.h',
    'HAL/IConsoleManager.h', 'HAL/PlatformTime.h'}


def validate_source(image_path):
    with Path(image_path).open('rb') as image:
        for window in WINDOWS:
            image.seek(int(window['va'], 16) - 0x140000000)
            data = image.read(window['size'])
            if len(data) != window['size'] or hashlib.sha256(data).hexdigest() != window['sha256']:
                raise ValueError('original input modality window differs: ' + window['va'])


def validate_classes(blocks, name_to_block):
    identity = CONTRACT['identity']
    block = blocks.get(identity['block_va'])
    if (name_to_block.get('UCommonInputSubsystem') != identity['block_va'] or block is None
            or any(block.get(k) != v for k, v in identity.items())
            or block.get('properties') != CONTRACT['properties']):
        raise ValueError('original input subsystem identity or fields differ')


def validate_types(gate):
    for fn, kind, cpp, record in (
            ('0x14081eee0', 'struct', 'FCommonInputPlatformData', 'CommonInputPlatformData'),
            ('0x14081ee20', 'enum', 'ECommonInputType', 'ECommonInputType'),
            ('0x14081edf0', 'enum', 'ECommonGamepadType', 'ECommonGamepadType')):
        info = gate.info(fn)
        if (info is None or info.fn != fn or info.kind != kind or info.cpp_name != cpp
                or info.package != '/Script/CommonInput' or info.rec != TYPES[record]):
            raise ValueError('original input modality type differs: ' + cpp)
        reason = gate.reason(info)
        if reason is not None:
            raise ValueError('original input modality type unavailable: ' + cpp + ': ' + reason)


def apply(model, block, owner_fns, native_execs=None):
    if model['name'] != 'UCommonInputSubsystem':
        return model
    identity = CONTRACT['identity']
    if (model.get('package') != '/Script/CommonInput' or model.get('parent') != 'ULocalPlayerSubsystem'
            or any(block.get(k) != v for k, v in identity.items())
            or block.get('properties') != CONTRACT['properties'] or '0x14081e010' not in owner_fns):
        raise ValueError('input modality owner or fields differ')
    if [f for f in block.get('functions', []) if f.get('name') in SETTERS] != CONTRACT['functions']:
        raise ValueError('input modality setter metadata differs')
    if not isinstance(native_execs, dict) or any(native_execs.get(k) != v for k, v in CONTRACT['native'].items()):
        raise ValueError('input modality setter exec differs')
    for name, declaration in MEMBERS.items():
        if [m.get('decl') for m in model.get('members_decl_order', []) if m.get('name') == name] != [declaration]:
            raise ValueError('input modality emitted field differs: ' + name)
    if model.get('common_input_modality_runtime') or any(
            m.get('name') in ALIASES for group in ('functions', 'members_table_order', 'members_decl_order',
                                                    'fields', 'helpers', 'static_helpers')
            for m in model.get(group, [])):
        raise ValueError('input modality declaration collision')
    for name, (arg, enum_fn) in SETTERS.items():
        model['functions'].append({'name': name, 'rpc': False, 'service': None, 'ret': 'void',
            'args': [arg], 'const': False, 'notify': False, 'splice': False, 'blueprint_event': False,
            'access': 'public', 'runtime_implementation': True, 'specifiers': ['BlueprintCallable'],
            'used': {enum_fn: {'Enum'}}})
    model['common_input_modality_runtime'] = True
    return model


def configure_model(model):
    """Call after generator resets model includes from the type gate."""
    if model.get('common_input_modality_runtime'):
        model['cpp_includes'] = sorted(set(model.get('cpp_includes', [])) | CPP_INCLUDES)
        model['forward_decls'] = sorted(set(model.get('forward_decls', [])) | {'IConsoleVariable'})
    return model


def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get('common_input_modality_runtime') else ''


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get('common_input_modality_runtime') else ''


def sources(module, blocks, name_to_block):
    if module != 'CommonInput':
        raise ValueError('input modality policy used for another module')
    validate_classes(blocks, name_to_block)
    return {name: (HERE / name).read_text() for name in ('CommonInputPreprocessor.h', 'CommonInputPreprocessorCore.h')}


def validate_rendered(model, header, source):
    if not model.get('common_input_modality_runtime'):
        return
    for content, fragment, label in ((header, declarations(model), 'declarations'),
                                     (source, definitions(model), 'definitions')):
        if content.count(fragment.rstrip()) != 1:
            raise ValueError('rendered input modality ' + label + ' differ')
    for name in SETTERS:
        if header.count(name + '(') != 1 or source.count('UCommonInputSubsystem::' + name + '(') != 1:
            raise ValueError('input modality setter missing or duplicated: ' + name)
    if header.count('FCommonInputNativeMethodChanged NativeInputMethodChanged;') != 1:
        raise ValueError('input modality native delegate missing or duplicated')
    if source.count('UCommonInputSubsystem::NativeBroadcastInputMethodChanged(') != 1:
        raise ValueError('input modality broadcaster missing or duplicated')
    for include in CPP_INCLUDES:
        if '#include "' + include + '"' not in source:
            raise ValueError('input modality source include missing: ' + include)
