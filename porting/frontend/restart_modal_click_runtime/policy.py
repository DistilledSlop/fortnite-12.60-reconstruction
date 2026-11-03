"""Original UFortRestartClientModal::HandleButtonClicked (exec 0x1452c2a80, body 0x144f5f9f0).

Apply after the legacy panel operation queue policy (needs the panel's pop-self virtual and
stack-membership accessor). The launcher branch of 0x141ed81a0 is not reached from this caller.
"""
from pathlib import Path
HERE = Path(__file__).resolve().parent
CLASS = 'UFortRestartClientModal'
BLOCK = '0x146fc2220'
EXEC = '0x1452c2a80'
FLAGS = '0x4080401'
FLAG = 'restart_modal_click_runtime'


def apply(model, block, owner_fns, native_execs=None):
    if model.get('name') != CLASS:
        return model
    if model.get('package') != '/Script/FortniteUI' or block.get('block_va') != BLOCK:
        raise ValueError('restart modal owner mismatch')
    entry = (native_execs or {}).get('HandleButtonClicked') or {}
    if entry.get('exec_addr') != EXEC or entry.get('flags') != FLAGS or entry.get('params') != []:
        raise ValueError('original HandleButtonClicked exec/flags mismatch')
    if not model.get('legacy_panel_operation_queue_runtime') and model.get('parent') != 'UFortActivatablePanel':
        raise ValueError('restart modal parent mismatch')
    if model.get(FLAG) or any(f.get('name') == 'HandleButtonClicked' for f in model.get('functions', [])):
        raise ValueError('restart modal HandleButtonClicked collision')
    model['skipped'] = [i for i in model.get('skipped', [])
                        if not (i.get('item') == 'function' and i.get('name') == 'HandleButtonClicked')]
    model['functions'].append({'name': 'HandleButtonClicked', 'rpc': False, 'service': None, 'ret': 'void',
        'args': [], 'const': False, 'notify': False, 'splice': False, 'blueprint_event': False,
        'access': 'protected', 'runtime_implementation': True, 'specifiers': ['BlueprintCallable'], 'used': {}})
    model[FLAG] = True
    return model


def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get(FLAG) else ''


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get(FLAG) else ''
