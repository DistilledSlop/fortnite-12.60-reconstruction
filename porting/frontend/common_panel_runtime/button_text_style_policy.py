"""Two original CommonButton readers and the native selected-state foundation."""
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'button-text-style-original-policy.json').read_text())
NAMES = ('GetSelected', 'GetCurrentTextStyleClass')


def validate_classes(blocks, name_to_block):
    for name in ('UCommonButton', 'UCommonButtonStyle', 'UCommonTextStyle'):
        expected = CONTRACT[name]
        va = expected['identity']['block_va']
        block = blocks.get(va)
        if name_to_block.get(name) != va or block is None:
            raise ValueError('original class identity mismatch: ' + name)
        if any(block.get(k) != v for k, v in expected['identity'].items()):
            raise ValueError('original class metadata mismatch: ' + name)
        props = {p['name'] for p in expected['properties']}
        if [p for p in block.get('properties', []) if p.get('name') in props] != expected['properties']:
            raise ValueError('original text style properties mismatch: ' + name)


def apply(model, block, owner_fns, native_execs):
    if model['name'] != 'UCommonButton':
        return model
    if model.get('package') != '/Script/CommonUI' or block.get('block_va') != '0x145a01210' or '0x140805cb0' not in owner_fns:
        raise ValueError('original CommonButton owner mismatch')
    for f in CONTRACT['UCommonButton']['functions']:
        name = f['name']
        if [x for x in block.get('functions', []) if x.get('name') == name] != [f]:
            raise ValueError('original accessor signature mismatch: ' + name)
        if not isinstance(native_execs, dict) or native_execs.get(name) != CONTRACT['exec'][name]:
            raise ValueError('original accessor native exec mismatch: ' + name)
    for p in CONTRACT['UCommonButton']['properties']:
        if [x for x in block.get('properties', []) if x.get('name') == p['name']] != [p]:
            raise ValueError('original Style metadata mismatch')
    if [x.get('decl') for x in model.get('members_decl_order', []) if x.get('name') == 'Style'] != ['TSubclassOf<UCommonButtonStyle> Style;']:
        raise ValueError('typed Style field unavailable')
    collisions = set(NAMES) | {'bNativeSelected', 'NativeInternalButton', 'IsHovered', 'FCommonButtonTextStyleProbeAccess'}
    if model.get('common_button_text_style_runtime') or any(x.get('name') in collisions for group in ('functions','members_table_order','members_decl_order','fields','helpers','static_helpers') for x in model.get(group, [])):
        raise ValueError('CommonButton text style declaration collision')
    for name, ret in (('GetSelected', 'bool'), ('GetCurrentTextStyleClass', 'TSubclassOf<UCommonTextStyle>')):
        model['functions'].append({'name':name,'rpc':False,'service':None,'ret':ret,'args':[],
            'const':True,'notify':False,'splice':False,'blueprint_event':False,'access':'public',
            'runtime_implementation':True,'specifiers':['BlueprintCallable','BlueprintPure'],'used':{}})
    model['common_button_text_style_runtime'] = True
    return model


def header_prelude(model):
    result = dict(model)
    if result.get('common_button_text_style_runtime'):
        result['forward_decls'] = sorted(set(result.get('forward_decls', [])) | {'UCommonTextStyle', 'UCommonButtonInternal'})
    return result


def declarations(model):
    return (HERE / 'button-text-style-declarations.inc').read_text() if model.get('common_button_text_style_runtime') else ''


def definitions(model):
    return (HERE / 'button-text-style-definitions.inc').read_text() if model.get('common_button_text_style_runtime') else ''


def emit_headers(source_root, generated_names):
    if 'UCommonButton' in generated_names:
        target = Path(source_root) / 'CommonUI/Generated/CommonButtonTextStyleCore.h'
        target.write_text((HERE / 'CommonButtonTextStyleCore.h').read_text())
