"""Original asset-imported CommonUI row and recovered native defaults."""
import json
from pathlib import Path

EXPECTED = json.loads((Path(__file__).resolve().parent / 'original-types.json').read_text())
ROOT = '0x140818010'
INFO = '0x140818040'
ENUM = '0x140817fb0'
TYPES = ((ROOT, 'struct', 'FCommonInputActionData', 'CommonInputActionData'),
         (INFO, 'struct', 'FCommonInputTypeInfo', 'CommonInputTypeInfo'),
         (ENUM, 'enum', 'EInputActionState', 'EInputActionState'))


def validate(gate):
    for fn, kind, cpp, record in TYPES:
        info = gate.info(fn)
        if (info is None or info.fn != fn or info.kind != kind or info.cpp_name != cpp
                or info.package != '/Script/CommonUI' or info.rec != EXPECTED[record]):
            raise ValueError('original CommonUI asset type identity or metadata differs: ' + cpp)
    for fn, _, cpp, _ in TYPES:
        reason = gate.reason(gate.info(fn))
        if reason is not None:
            raise ValueError('original CommonUI asset type unavailable: ' + cpp + ': ' + reason)


def add_roots(gate, seeds):
    validate(gate)
    seeds.setdefault(ROOT, set()).add('Struct')


def apply_struct(info, model):
    if info.fn == INFO or info.cpp_name == 'FCommonInputTypeInfo':
        if (info.fn != INFO or info.kind != 'struct' or info.package != '/Script/CommonUI'
                or info.cpp_name != 'FCommonInputTypeInfo' or info.rec != EXPECTED['CommonInputTypeInfo']):
            raise ValueError('original CommonInputTypeInfo identity or metadata differs')
        model['common_input_defaults'] = True
    return model


def declarations(model):
    if not model.get('common_input_defaults'):
        return []
    return ['\tFCommonInputTypeInfo()',
            '\t\t: OverrrideState(EInputActionState::Enabled)',
            '\t\t, bActionRequiresHold(false)',
            '\t\t, HoldTime(0.5f)',
            '\t{',
            '\t\tOverrideBrush.DrawAs = ESlateBrushDrawType::NoDrawType;',
            '\t}', '']
