"""Source-guarded native defaults for the CommonInput platform struct."""
import json
from pathlib import Path

EXPECTED = json.loads((Path(__file__).resolve().parent / 'original-types.json').read_text())
STRUCT = '0x14081eee0'
TYPES = ((STRUCT, 'struct', 'FCommonInputPlatformData', 'CommonInputPlatformData'),
         ('0x14081ee20', 'enum', 'ECommonInputType', 'ECommonInputType'),
         ('0x14081edf0', 'enum', 'ECommonGamepadType', 'ECommonGamepadType'))
MEMBERS = (
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


def validate(gate):
    for fn, kind, cpp, record in TYPES:
        info = gate.info(fn)
        if (info is None or info.fn != fn or info.kind != kind or info.cpp_name != cpp
                or info.package != '/Script/CommonInput' or info.rec != EXPECTED[record]):
            raise ValueError('original CommonInput platform type identity or metadata differs: ' + cpp)
    for fn, _, cpp, _ in TYPES:
        reason = gate.reason(gate.info(fn))
        if reason is not None:
            raise ValueError('original CommonInput platform type unavailable: ' + cpp + ': ' + reason)


def apply_struct(info, model):
    if info.fn == STRUCT or info.cpp_name == 'FCommonInputPlatformData':
        if (info.fn != STRUCT or info.kind != 'struct' or info.package != '/Script/CommonInput'
                or info.cpp_name != 'FCommonInputPlatformData'
                or info.rec != EXPECTED['CommonInputPlatformData']):
            raise ValueError('original CommonInputPlatformData identity or metadata differs')
        emitted = [(m.get('name'), m.get('decl')) for m in model.get('members', [])]
        if emitted != list(MEMBERS):
            raise ValueError('CommonInputPlatformData emitted member declarations differ')
        if model.get('common_input_platform_defaults'):
            raise ValueError('CommonInputPlatformData constructor collision')
        model['common_input_platform_defaults'] = True
    return model


def declarations(model):
    if not model.get('common_input_platform_defaults'):
        return []
    return ['\tFCommonInputPlatformData()',
            '\t\t: bSupported(false)',
            '\t\t, DefaultInputType(ECommonInputType::Gamepad)',
            '\t\t, bSupportsMouseAndKeyboard(false)',
            '\t\t, bSupportsGamepad(true)',
            '\t\t, DefaultGamepadType(ECommonGamepadType::XboxOneController)',
            '\t\t, bCanChangeGamepadType(true)',
            '\t\t, bSupportsTouch(false)',
            '\t{}', '']
