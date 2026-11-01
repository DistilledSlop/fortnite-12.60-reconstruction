"""Bounded original CommonActivatableWidget activation policy; no Panel stub."""
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent
EXPECTED = json.loads((HERE / 'base-original-policy.json').read_text())
BUTTON_EXPECTED = json.loads((HERE / 'button-original-policy.json').read_text())
MAIN_BUTTON_QUERY_EXPECTED = json.loads((HERE / 'main-button-query-original-policy.json').read_text())
MANAGER_EXPECTED = json.loads((HERE / 'manager-original-policy.json').read_text())

def apply(model, block, owner_fns):
    if model['name'] == 'UCommonButton':
        return apply_main_button_query(model, block, owner_fns)
    if model['name'] == 'UCommonButtonInternal':
        return apply_button(model, block, owner_fns)
    if model['name'] == 'UCommonInputManager':
        return apply_manager(model, block, owner_fns)
    if model['name'] != 'UCommonActivatableWidget':
        return model
    if block.get('block_va') != '0x145a004e0' or model['package'] != '/Script/CommonUI' or '0x1408079b0' not in owner_fns:
        raise ValueError('original CommonActivatableWidget owner mismatch')
    for name, expected in EXPECTED.items():
        found = [f for f in block.get('functions', []) if f.get('name') == name]
        if found != [expected]:
            raise ValueError('original activation signature mismatch: '+name)
        if any(f['name'] == name for f in model['functions']):
            raise ValueError('activation method already emitted: '+name)
        event = name == 'BP_OnDeactivated'
        model['functions'].append({'name':name, 'rpc':False, 'service':None,
            'ret':'bool' if name=='IsActivated' else 'void', 'args':[],
            'const':name=='IsActivated', 'notify':False, 'splice':False,
            'blueprint_event':event, 'access':'protected' if event else 'public',
            'runtime_implementation':not event,
            'specifiers':['BlueprintImplementableEvent'] if event else
                ['BlueprintCallable','BlueprintPure'] if name=='IsActivated' else ['BlueprintCallable'], 'used':{}})
    model['common_activation_runtime']=True
    return model

def apply_main_button_query(model, block, owner_fns):
    if (block.get('block_va') != '0x145a01210' or model['package'] != '/Script/CommonUI'
            or '0x140805cb0' not in owner_fns):
        raise ValueError('original CommonButton owner mismatch')
    found = [f for f in block.get('functions', []) if f.get('name') == 'IsInteractionEnabled']
    if found != [MAIN_BUTTON_QUERY_EXPECTED]:
        raise ValueError('original IsInteractionEnabled signature mismatch')
    names = {'IsInteractionEnabled', 'NativeIsInteractionEnabled',
             'bNativeButtonEnabled', 'bNativeInteractionEnabled'}
    for group in ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers'):
        if any(item.get('name') in names for item in model.get(group, [])):
            raise ValueError('CommonButton query declaration collision: ' + group)
    if model.get('common_button_query_runtime'):
        raise ValueError('CommonButton query runtime already emitted')
    model['functions'].append({'name':'IsInteractionEnabled', 'rpc':False, 'service':None,
        'ret':'bool', 'args':[], 'const':True, 'notify':False, 'splice':False,
        'blueprint_event':False, 'access':'public', 'runtime_implementation':True,
        'specifiers':['BlueprintCallable','BlueprintPure'], 'used':{}})
    model['common_button_query_runtime'] = True
    return model

def apply_manager(model, block, owner_fns):
    if block.get('block_va') != '0x145a05fd0' or model['package'] != '/Script/CommonUI' or '0x14080e9a0' not in owner_fns:
        raise ValueError('original CommonInputManager owner mismatch')
    found = [f for f in block.get('functions', []) if f.get('name') == 'IsInputSuspended']
    if found != [MANAGER_EXPECTED]:
        raise ValueError('original IsInputSuspended signature mismatch')
    if any(f['name'] == 'IsInputSuspended' for f in model['functions']):
        raise ValueError('IsInputSuspended already emitted')
    model['functions'].append({'name':'IsInputSuspended', 'rpc':False, 'service':None,
        'ret':'bool', 'args':[], 'const':True, 'notify':False, 'splice':False,
        'blueprint_event':False, 'access':'public', 'runtime_implementation':True,
        'specifiers':['BlueprintCallable','BlueprintPure'], 'used':{}})
    model['common_input_suspension_runtime']=True
    return model

def apply_button(model, block, owner_fns):
    # This original owner has five properties and no reflected functions. All
    # runtime additions below are native C++ only, never invented UFUNCTIONs.
    if (model['package'] != '/Script/CommonUI' or model.get('parent') != 'UButton'
            or owner_fns != {'0x1408096f0'} or block != BUTTON_EXPECTED):
        raise ValueError('original CommonButtonInternal owner/property/signature mismatch')
    if (model.get('common_button_internal_runtime') or model.get('functions')
            or model.get('helpers') or model.get('fields')):
        raise ValueError('CommonButtonInternal runtime declarations already emitted or conflicting')
    model['common_button_internal_runtime'] = True
    return model

def header_prelude(model):
    result=dict(model)
    if result.get('common_activation_runtime'):
        result['includes']=sorted(set(result.get('includes', []))|{'CommonActivationNative.h'})
    if result.get('common_button_internal_runtime'):
        result['includes']=sorted(set(result.get('includes', []))|{'Widgets/Input/SButton.h','Input/Reply.h'})
        result['forward_decls']=sorted(set(result.get('forward_decls', []))|{'SBox','SCommonButton'})
    return result

def declarations(model):
    if model.get('common_button_query_runtime'): return (HERE/'main-button-query-declarations.inc').read_text()
    if model.get('common_button_internal_runtime'): return (HERE/'button-declarations.inc').read_text()
    if model.get('common_activation_runtime'): return (HERE/'base-declarations.inc').read_text()
    if model.get('common_input_suspension_runtime'): return (HERE/'manager-declarations.inc').read_text()
    return ''

def definitions(model):
    if model.get('common_button_query_runtime'): return (HERE/'main-button-query-definitions.inc').read_text()
    if model.get('common_button_internal_runtime'): return (HERE/'button-definitions.inc').read_text()
    if model.get('common_activation_runtime'): return '\n#include "CommonActivationCore.h"\n'+(HERE/'base-definitions.inc').read_text()
    if model.get('common_input_suspension_runtime'): return (HERE/'manager-definitions.inc').read_text()
    return ''

def emit_headers(source_root, generated_names):
    if 'UCommonActivatableWidget' not in generated_names:return
    target=Path(source_root)/'CommonUI/Generated'
    for name in ['CommonActivationCore.h','CommonActivationNative.h']:
        (target/name).write_text((HERE/name).read_text())

# Bounded Panel intro activation extension; original base/button/manager policies above retained.
import importlib.util as _intro_import
_intro_spec=_intro_import.spec_from_file_location('panel_intro_activation_policy',HERE/'panel_intro_policy.py')
_intro=_intro_import.module_from_spec(_intro_spec);_intro_spec.loader.exec_module(_intro)
_prior_apply=apply
_prior_header_prelude=header_prelude
_prior_declarations=declarations
_prior_definitions=definitions
_prior_emit_headers=emit_headers
def apply(model,block,owner_fns):
    return _intro.apply(_prior_apply(model,block,owner_fns),block,owner_fns)
def header_prelude(model):return _intro.header_prelude(_prior_header_prelude(model))
def declarations(model):return _prior_declarations(model)+_intro.declarations(model)
def definitions(model):return _prior_definitions(model)+_intro.definitions(model)
def emit_headers(source_root,generated_names):
    _prior_emit_headers(source_root,generated_names)
    _intro.emit_headers(source_root,generated_names)
