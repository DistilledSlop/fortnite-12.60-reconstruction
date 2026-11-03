"""Original legacy Panel operation queue; intentionally not registered by this module.

Call apply after common_panel_runtime's intro/outro/suspension policies; call
validate_types with the generator TypeGate, then use the emit hooks below.
The manager's actual tick registration and held-handler tick remain lifecycle
integration work. NativeTickOperationSuspension is the complete operation branch.
"""
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent
ORIGINAL = json.loads((HERE / 'original-policy.json').read_text())
PUBLIC_ARGS = {
    'PushActivatablePanel': ['UCommonActivatablePanel* ActivatablePanel', 'bool bIntroPanel', 'bool bOutroPanelBelow'],
    'PopActivatablePanel': ['UCommonActivatablePanel* ActivatablePanel'],
    'SuspendStartingOperationProcessing': [], 'ResumeStartingOperationProcessing': [],
}

def apply(model, block, owner_fns, native_execs=None):
    name = model['name']
    if name not in ('UCommonInputManager', 'UCommonActivatablePanel'):
        return model
    manager = name == 'UCommonInputManager'
    va = '0x145a05fd0' if manager else '0x145a00360'
    owner = '0x14080e9a0' if manager else '0x140807980'
    if model.get('package') != '/Script/CommonUI' or block.get('block_va') != va or owner not in owner_fns:
        raise ValueError('legacy panel queue owner mismatch: ' + name)
    if block.get('properties') != ORIGINAL[va]['properties']:
        raise ValueError('legacy panel queue property mismatch: ' + name)
    required = ['common_input_suspension_runtime'] if manager else ['panel_intro_activation_runtime', 'panel_outro_runtime']
    if not all(model.get(flag) for flag in required):
        raise ValueError('legacy panel queue requires real panel activation/held cleanup and manager suspension')
    if model.get('legacy_panel_operation_queue_runtime'):
        raise ValueError('legacy panel queue already emitted')
    for n, signature in ORIGINAL[va]['functions'].items():
        if [f for f in block.get('functions', []) if f.get('name') == n] != [signature]:
            raise ValueError('original legacy queue signature mismatch: ' + n)
        if manager and (not isinstance(native_execs, dict) or (native_execs.get(n) or {}).get('exec_addr') != ORIGINAL['native_execs'][n]):
            raise ValueError('original legacy queue exec mismatch: ' + n)
    # Reject collisions for every native declaration, not just reflected methods.
    import re
    decls = (HERE / ('manager-declarations.inc' if manager else 'panel-declarations.inc')).read_text()
    names = set(re.findall(r'\b(?:Native\w+|bNative\w+)\b', decls)) | set(ORIGINAL[va]['functions'])
    for group in ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers'):
        if any(item.get('name') in names for item in model.get(group, [])):
            raise ValueError('legacy panel queue declaration collision: ' + group)
    for n in ORIGINAL[va]['functions']:
        model['functions'].append({'name': n, 'rpc': False, 'service': None, 'ret': 'void',
            'args': PUBLIC_ARGS[n] if manager else [], 'const': False, 'notify': False, 'splice': False,
            'blueprint_event': not manager, 'access': 'public' if manager else 'protected',
            'runtime_implementation': manager,
            'specifiers': ['BlueprintCallable'] if manager else ['BlueprintImplementableEvent'], 'used': {}})
    model['legacy_panel_operation_queue_runtime'] = 'manager' if manager else 'panel'
    return model

def validate_types(gate):
    info = gate.info('0x14080fc10')
    if info is None or info.package != '/Script/CommonUI' or info.rec != ORIGINAL['operation']:
        raise ValueError('original Operation type mismatch')
    reason = gate.reason(info)
    if reason is not None:
        raise ValueError('Operation unavailable: ' + reason)
    model = gate.models[info.fn]
    expected = ['EOperation Operation;', 'UCommonActivatablePanel* Panel;', 'bool bIntroPanel;',
                'bool bActivatePanel;', 'bool bOutroPanelBelow;']
    if [m['decl'] for m in model['members']] != expected or model.get('legacy_panel_operation_guid'):
        raise ValueError('Operation declaration/layout collision')
    model['legacy_panel_operation_guid'] = True

def struct_declarations(model):
    return ['\t// Unreflected original +14..23 operation identifier.', '\tFGuid NativeOperationId;', ''] if model.get('legacy_panel_operation_guid') else []

def struct_postlude(model):
    if not model.get('legacy_panel_operation_guid'):
        return ''
    return '\n'.join([
        'static_assert(STRUCT_OFFSET(FOperation, Operation) == 0x00, "Operation kind ABI");',
        'static_assert(STRUCT_OFFSET(FOperation, Panel) == 0x08, "Operation panel ABI");',
        'static_assert(STRUCT_OFFSET(FOperation, bIntroPanel) == 0x10, "Operation intro ABI");',
        'static_assert(STRUCT_OFFSET(FOperation, bActivatePanel) == 0x11, "Operation activate ABI");',
        'static_assert(STRUCT_OFFSET(FOperation, bOutroPanelBelow) == 0x12, "Operation outro ABI");',
        'static_assert(STRUCT_OFFSET(FOperation, NativeOperationId) == 0x14, "Operation ID ABI");',
        'static_assert(sizeof(FOperation) == 0x28 && alignof(FOperation) == 8, "Operation ABI");', ''])

def header_prelude(model):
    result = dict(model)
    if model.get('legacy_panel_operation_queue_runtime') == 'manager':
        result['includes'] = sorted(set(result.get('includes', [])) | {'LegacyPanelOperationNative.h'})
    return result

def declarations(model):
    tag = model.get('legacy_panel_operation_queue_runtime')
    return (HERE / (tag + '-declarations.inc')).read_text() if tag else ''

def definitions(model):
    tag = model.get('legacy_panel_operation_queue_runtime')
    return (HERE / (tag + '-definitions.inc')).read_text() if tag else ''

def emit_headers(source_root, generated_names):
    if 'UCommonInputManager' in generated_names:
        target = Path(source_root) / 'CommonUI/Generated'
        target.mkdir(parents=True, exist_ok=True)
        (target / 'LegacyPanelOperationNative.h').write_text((HERE / 'LegacyPanelOperationNative.h').read_text())
