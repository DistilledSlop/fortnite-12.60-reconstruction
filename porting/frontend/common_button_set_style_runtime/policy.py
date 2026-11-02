"""Exact original SetStyle gate, enabled only with its production producer graph."""
import hashlib
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
FLAG = 'common_button_set_style_runtime'

def validate_source(image):
    window = CONTRACT['window']
    with Path(image).open('rb') as stream:
        stream.seek(int(window['va'], 16) - 0x140000000)
        data = stream.read(window['length'])
    if len(data) != window['length'] or hashlib.sha256(data).hexdigest() != window['sha256']:
        raise ValueError('original CommonButton SetStyle code differs')

def validate_classes(blocks, names, native_funcs):
    identity = CONTRACT['identity']
    block = blocks.get(identity['block_va'])
    if names.get('UCommonButton') != identity['block_va'] or block is None or any(block.get(k) != v for k, v in identity.items()):
        raise ValueError('CommonButton SetStyle original owner differs')
    if [f for f in block.get('functions', []) if f['name'] == 'SetStyle'] != [CONTRACT['function']]:
        raise ValueError('CommonButton SetStyle reflection differs')
    if [p for p in block.get('properties', []) if p['name'] == 'Style'] != [CONTRACT['property']]:
        raise ValueError('CommonButton Style property differs')
    if native_funcs.get('UCommonButton', {}).get('SetStyle') != CONTRACT['native']:
        raise ValueError('CommonButton SetStyle native exec differs')

def apply(model, block, owner_fns):
    if model['name'] != 'UCommonButton': return model
    if (model.get('package') != '/Script/CommonUI' or model.get('block_va') != CONTRACT['identity']['block_va']
            or block.get('block_va') != CONTRACT['identity']['block_va'] or '0x140805cb0' not in owner_fns):
        raise ValueError('SetStyle model owner differs')
    for dependency in ('common_button_style_rebuild_runtime', 'common_button_initialize_runtime', 'common_button_focus_selection_runtime'):
        if not model.get(dependency): raise ValueError('SetStyle requires production graph: ' + dependency)
    if [m.get('decl') for m in model.get('members_decl_order', []) if m['name'] == 'Style'] != ['TSubclassOf<UCommonButtonStyle> Style;']:
        raise ValueError('SetStyle class declaration differs')
    if model.get(FLAG) or any(m.get('name') == 'SetStyle' for group in ('functions','members_table_order','members_decl_order','fields','helpers','static_helpers') for m in model.get(group, [])):
        raise ValueError('SetStyle declaration collision')
    model['functions'].append({'name':'SetStyle', 'rpc':False, 'service':None, 'ret':'void',
        'args':['TSubclassOf<UCommonButtonStyle> InStyle'], 'const':False, 'notify':False, 'splice':False,
        'blueprint_event':False, 'access':'public', 'runtime_implementation':True,
        'specifiers':['BlueprintCallable'], 'used':{}})
    model[FLAG] = True
    return model

def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get(FLAG) else ''

def validate_output(model, header, source):
    if model.get(FLAG) and (header.count('void SetStyle(TSubclassOf<UCommonButtonStyle> InStyle);') != 1
                           or source.count(definitions(model)) != 1):
        raise ValueError('SetStyle rendered output differs')
