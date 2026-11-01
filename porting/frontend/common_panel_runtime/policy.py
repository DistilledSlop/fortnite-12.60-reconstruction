"""Bounded original CommonActivatableWidget activation policy; no Panel stub."""
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent
EXPECTED = json.loads((HERE / 'base-original-policy.json').read_text())

def apply(model, block, owner_fns):
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

def header_prelude(model):
    result=dict(model)
    if result.get('common_activation_runtime'):
        result['includes']=sorted(set(result.get('includes', []))|{'CommonActivationNative.h'})
    return result

def declarations(model):
    return (HERE/'base-declarations.inc').read_text() if model.get('common_activation_runtime') else ''

def definitions(model):
    return ('\n#include "CommonActivationCore.h"\n'+(HERE/'base-definitions.inc').read_text()) if model.get('common_activation_runtime') else ''

def emit_headers(source_root, generated_names):
    if 'UCommonActivatableWidget' not in generated_names:return
    target=Path(source_root)/'CommonUI/Generated'
    for name in ['CommonActivationCore.h','CommonActivationNative.h']:
        (target/name).write_text((HERE/name).read_text())
