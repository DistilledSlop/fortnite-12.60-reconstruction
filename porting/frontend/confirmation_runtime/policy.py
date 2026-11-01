"""Native confirmation description constructor and delegates, bound to original metadata."""
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent
EXPECTED = json.loads((HERE / 'original-types.json').read_text())
DESCRIPTION = '0x142b25370'
ENUM = '0x142b242c0'
HANDLE = '0x142b253a0'

def apply_struct(gate, info, model):
    if info.cpp_name != 'FFortDialogDescription_NUI':
        return model
    if (info.kind != 'struct' or info.fn != DESCRIPTION or info.package != '/Script/FortniteGame'
            or info.rec != EXPECTED['FortDialogDescription_NUI']):
        raise ValueError('original confirmation description identity or fields differ')
    for fn, kind, name in ((ENUM, 'enum', 'EFortDialogResult'),
                           (HANDLE, 'struct', 'FortDialogExternalLatentActionHandle')):
        dep = gate.info(fn)
        if (dep is None or dep.kind != kind or dep.package != '/Script/FortniteGame'
                or dep.rec != EXPECTED[name]):
            raise ValueError('original confirmation dependency mismatch: ' + name)
        reason = gate.reason(dep)
        if reason is not None:
            raise ValueError('confirmation dependency unavailable: ' + name + ': ' + reason)
        model['used'].setdefault(fn, set()).add('Enum' if kind == 'enum' else 'Struct')
    model['confirmation_runtime'] = True
    return model

def preamble(model):
    if not model.get('confirmation_runtime'):
        return []
    return ['// Project-local names for the original native single-cast callback contracts.',
            'DECLARE_DELEGATE_TwoParams(FNativeConfirmationResult, EFortDialogResult, FName);',
            'DECLARE_DELEGATE_FourParams(FNativeAdvancedConfirmationResult, EFortDialogResult, FName, bool, FFortDialogExternalLatentActionHandle);', '']

def declarations(model):
    if not model.get('confirmation_runtime'):
        return []
    return (HERE / 'declarations.inc').read_text().splitlines()

def footer(model):
    if not model.get('confirmation_runtime'):
        return []
    return ['static_assert(TStructOpsTypeTraits<FFortDialogDescription_NUI>::WithCopy,',
            '    "Confirmation description must copy its native delegates through C++ struct ops");', '']
