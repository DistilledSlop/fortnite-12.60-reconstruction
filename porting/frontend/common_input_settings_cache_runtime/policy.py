"""Fail-closed settings cache reconstruction, dependent on the constructor policy."""
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
TYPES = json.loads((HERE / 'original-types.json').read_text())
WINDOWS = json.loads((HERE / 'original-windows.json').read_text())['windows']
PLATFORM_MEMBERS = (
    ('bSupported', 'bool bSupported;'),
    ('DefaultInputType', 'ECommonInputType DefaultInputType;'),
    ('bSupportsMouseAndKeyboard', 'bool bSupportsMouseAndKeyboard;'),
    ('bSupportsGamepad', 'bool bSupportsGamepad;'),
    ('DefaultGamepadType', 'ECommonGamepadType DefaultGamepadType;'),
    ('bCanChangeGamepadType', 'bool bCanChangeGamepadType;'),
    ('bSupportsTouch', 'bool bSupportsTouch;'),
    ('ControllerData', 'TArray<TSoftClassPtr<UCommonInputControllerData>> ControllerData;'),
    ('ControllerDataClasses', 'TArray<TSubclassOf<UCommonInputControllerData>> ControllerDataClasses;'),
)
NAMES = ('NativeGetCurrentPlatformData', 'ValidateNativeInputDataCache',
         'InitializeNativeInputDataCache')


def validate_source(image_path):
    """Verify the mapped original image's four bounded code windows."""
    with Path(image_path).open('rb') as image:
        for window in WINDOWS:
            image.seek(int(window['va'], 16) - 0x140000000)
            data = image.read(window['size'])
            if len(data) != window['size'] or hashlib.sha256(data).hexdigest() != window['sha256']:
                raise ValueError('original settings cache code differs: ' + window['va'])


def validate_classes(blocks, name_to_block):
    identity = CONTRACT['identity']
    block = blocks.get(identity['block_va'])
    if (name_to_block.get('UCommonInputSettings') != identity['block_va'] or block is None
            or any(block.get(k) != v for k, v in identity.items())
            or block.get('properties') != CONTRACT['properties']):
        raise ValueError('original settings cache identity or properties differ')


def validate_types(gate):
    for fn, kind, cpp, record in (
            ('0x14081eee0', 'struct', 'FCommonInputPlatformData', 'CommonInputPlatformData'),
            ('0x14081ee20', 'enum', 'ECommonInputType', 'ECommonInputType'),
            ('0x14081edf0', 'enum', 'ECommonGamepadType', 'ECommonGamepadType')):
        info = gate.info(fn)
        if (info is None or info.fn != fn or info.kind != kind or info.cpp_name != cpp
                or info.package != '/Script/CommonInput' or info.rec != TYPES[record]):
            raise ValueError('original settings cache type differs: ' + cpp)
        reason = gate.reason(info)
        if reason is not None:
            raise ValueError('original settings cache type unavailable: ' + cpp + ': ' + reason)


def validate_platform_model(info, model):
    if (info.fn != '0x14081eee0' or info.kind != 'struct'
            or info.cpp_name != 'FCommonInputPlatformData' or info.package != '/Script/CommonInput'
            or info.rec != TYPES['CommonInputPlatformData']
            or [(m.get('name'), m.get('decl')) for m in model.get('members', [])] != list(PLATFORM_MEMBERS)
            or not model.get('common_input_platform_defaults')):
        raise ValueError('settings cache platform model differs or lacks native defaults')


def apply(model, block, owner_fns):
    if model['name'] != 'UCommonInputSettings':
        return model
    if (model.get('package') != '/Script/CommonInput'
            or model.get('block_va') != CONTRACT['identity']['block_va']
            or block.get('block_va') != CONTRACT['identity']['block_va']
            or '0x14081ed60' not in owner_fns
            or not model.get('common_input_settings_constructor')):
        raise ValueError('settings cache owner or constructor policy differs')
    for name, declaration in CONTRACT['members'].items():
        if [m.get('decl') for m in model.get('members_decl_order', []) if m.get('name') == name] != [declaration]:
            raise ValueError('settings cache emitted member differs: ' + name)
    if model.get('common_input_settings_cache_runtime') or any(
            m.get('name') in NAMES for group in
            ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers')
            for m in model.get(group, [])):
        raise ValueError('settings cache method collision')
    model['common_input_settings_cache_runtime'] = True
    return model


def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get('common_input_settings_cache_runtime') else ''


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get('common_input_settings_cache_runtime') else ''


def validate_rendered(model, header, source):
    """Call on the final rendered strings before either generated file is written."""
    if not model.get('common_input_settings_cache_runtime'):
        return
    for content, fragment, label in ((header, declarations(model), 'declarations'),
                                     (source, definitions(model), 'definitions')):
        if content.count(fragment.rstrip()) != 1:
            raise ValueError('rendered settings cache ' + label + ' differ')
    if header.count('bool NativeInputDataLoaded;') != 1:
        raise ValueError('rendered settings cache state missing or duplicated')
    for name in NAMES:
        if header.count(name + '(') != 1 or source.count('UCommonInputSettings::' + name + '(') != 1:
            raise ValueError('rendered settings cache method missing or duplicated: ' + name)
    for include in ("UObject/UObjectArray.h", "UCommonInputControllerData.h", "UCommonUIInputData.h"):
        if '#include "' + include + '"' not in source:
            raise ValueError("rendered settings cache lacks include: " + include)
