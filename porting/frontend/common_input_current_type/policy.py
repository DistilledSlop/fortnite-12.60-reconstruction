"""Source-guarded original CommonInputSubsystem current-type accessor."""
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
NAME = 'GetCurrentInputType'

def apply(model, block, owner_fns, native_execs=None):
    if model['name'] != 'UCommonInputSubsystem':
        return model
    if (model.get('package') != '/Script/CommonInput' or model.get('parent') != 'ULocalPlayerSubsystem'
            or block.get('block_va') != '0x145a0e0d0' or '0x14081e010' not in owner_fns):
        raise ValueError('original CommonInputSubsystem owner mismatch')
    if [f for f in block.get('functions', []) if f.get('name') == NAME] != [CONTRACT['function']]:
        raise ValueError('original GetCurrentInputType signature mismatch')
    props = [p for p in block.get('properties', []) if p.get('name') == 'CurrentInputType' or p.get('enum_underlying_of') == 'CurrentInputType']
    if props != CONTRACT['properties']:
        raise ValueError('original CurrentInputType property mismatch')
    emitted = [p for p in model.get('members_decl_order', []) if p.get('name') == 'CurrentInputType']
    if len(emitted) != 1 or emitted[0].get('decl') != 'ECommonInputType CurrentInputType;':
        raise ValueError('CurrentInputType enum field unavailable')
    if not isinstance(native_execs, dict) or native_execs.get(NAME) != CONTRACT['exec']:
        raise ValueError('original GetCurrentInputType native exec mismatch')
    if model.get('common_input_current_type_runtime') or any(item.get('name') == NAME for group in ('functions','members_table_order','members_decl_order','fields','helpers','static_helpers') for item in model.get(group, [])):
        raise ValueError('GetCurrentInputType declaration collision')
    model['functions'].append({'name':NAME,'rpc':False,'service':None,'ret':'ECommonInputType','args':[],
        'const':True,'notify':False,'splice':False,'blueprint_event':False,'access':'public',
        'runtime_implementation':True,'specifiers':['BlueprintCallable','BlueprintPure'],'used':{}})
    model['common_input_current_type_runtime'] = True
    return model

def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get('common_input_current_type_runtime') else ''

def validate_types(gate):
    info = gate.info('0x14081ee20')
    if (info is None or info.fn != '0x14081ee20' or info.kind != 'enum'
            or info.cpp_name != 'ECommonInputType' or info.package != '/Script/CommonInput'
            or info.rec != CONTRACT['enum']):
        raise ValueError('original ECommonInputType identity or metadata differs')
    reason = gate.reason(info)
    if reason is not None:
        raise ValueError('original ECommonInputType unavailable: ' + reason)
