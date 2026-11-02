"""Source-guarded IsErebus implementation for the supplied Win64 baseline."""
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
NAME = 'IsErebus'


def apply(model, block, owner_fns, native_execs=None):
    if model['name'] != 'UFortGlobals':
        return model
    if (model.get('package') != '/Script/FortniteGame' or model.get('parent') != 'UObject'
            or model.get('block_va') != CONTRACT['class_block_va']
            or block.get('block_va') != CONTRACT['class_block_va']
            or block.get('class_noregister_fn') != CONTRACT['class_noregister_fn']
            or block.get('base_fn') != CONTRACT['base_fn']
            or block.get('package_fn') != CONTRACT['package_fn']
            or CONTRACT['owner_fn'] not in owner_fns):
        raise ValueError('original FortGlobals owner mismatch')
    if [f for f in block.get('functions', []) if f.get('name') == NAME] != [CONTRACT['function']]:
        raise ValueError('original IsErebus signature mismatch')
    if not isinstance(native_execs, dict) or native_execs.get(NAME) != CONTRACT['exec']:
        raise ValueError('original IsErebus native exec mismatch')
    if model.get('fort_globals_erebus_runtime') or any(
            item.get('name') == NAME
            for group in ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers')
            for item in model.get(group, [])):
        raise ValueError('IsErebus declaration collision')
    model['functions'].append({'name': NAME, 'rpc': False, 'service': None, 'ret': 'bool', 'args': [],
        'static': True, 'const': False, 'notify': False, 'splice': False, 'blueprint_event': False,
        'access': 'public', 'runtime_implementation': True, 'specifiers': ['BlueprintCallable', 'BlueprintPure'],
        'used': {}})
    model['fort_globals_erebus_runtime'] = True
    return model


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get('fort_globals_erebus_runtime') else ''
