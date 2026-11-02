"""Guard the original UFortUINotification soft-object accessor."""
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
NAME = 'GetImageOverride'


def apply(model, block, owner_fns, native_execs=None):
    if model['name'] != 'UFortUINotification':
        return model
    if (model.get('package') != '/Script/FortniteGame' or model.get('parent') != 'UObject'
            or block.get('block_va') != '0x14655a270' or '0x142b26590' not in owner_fns):
        raise ValueError('original UFortUINotification owner mismatch')
    if [f for f in block.get('functions', []) if f.get('name') == NAME] != [CONTRACT['function']]:
        raise ValueError('original GetImageOverride signature mismatch')
    if [p for p in block.get('properties', []) if p.get('name') == 'DisplayAsset'] != [CONTRACT['property']]:
        raise ValueError('original DisplayAsset property mismatch')
    emitted = [p for p in model.get('members_decl_order', []) if p.get('name') == 'DisplayAsset']
    if len(emitted) != 1 or emitted[0].get('decl') != 'TSoftObjectPtr<UObject> DisplayAsset;':
        raise ValueError('DisplayAsset soft object field unavailable')
    if not isinstance(native_execs, dict) or native_execs.get(NAME) != CONTRACT['exec']:
        raise ValueError('original GetImageOverride native exec mismatch')
    if model.get('notification_image_override_runtime') or any(item.get('name') == NAME for group in ('functions','members_table_order','members_decl_order','fields','helpers','static_helpers') for item in model.get(group, [])):
        raise ValueError('GetImageOverride declaration collision')
    model['functions'].append({'name':NAME, 'rpc':False, 'service':None,
        'ret':'TSoftObjectPtr<UObject>', 'args':[], 'const':False, 'notify':False,
        'splice':False, 'blueprint_event':False, 'access':'public',
        'runtime_implementation':True, 'specifiers':['BlueprintCallable'], 'used':{}})
    model['notification_image_override_runtime'] = True
    model['skipped'] = [s for s in model.get('skipped', []) if not
        (s.get('item') == 'function' and s.get('name') == NAME and s.get('category') == 'non_void_return')]
    return model


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get('notification_image_override_runtime') else ''


def validate_output(model, header, cpp):
    if not model.get('notification_image_override_runtime') or header.count('TSoftObjectPtr<UObject> GetImageOverride();') != 1:
        raise ValueError('GetImageOverride declaration missing or duplicated')
    if cpp.count(definitions(model).strip()) != 1:
        raise ValueError('GetImageOverride definition missing or duplicated')
