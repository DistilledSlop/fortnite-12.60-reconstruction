"""Strict original CommonTextBlock SetStyle and SynchronizeProperties boundary."""
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'text-block-style-original-policy.json').read_text())

def validate_classes(blocks, name_to_block):
    for name in ('UCommonTextBlock', 'UCommonTextStyle'):
        expected = CONTRACT[name]
        va = expected['identity']['block_va']
        block = blocks.get(va)
        if name_to_block.get(name) != va or block is None or any(block.get(k) != v for k, v in expected['identity'].items()):
            raise ValueError('original text block style class identity mismatch: ' + name)
        if block.get('properties') != expected['properties']:
            raise ValueError('original text block style properties mismatch: ' + name)

def apply(model, block, owner_fns, native_execs):
    if model['name'] != 'UCommonTextBlock':
        return model
    if model.get('package') != '/Script/CommonUI' or model.get('parent') != 'UTextBlock' or block.get('block_va') != '0x145a08a80' or '0x140812e80' not in owner_fns:
        raise ValueError('original CommonTextBlock owner mismatch')
    if [f for f in block.get('functions', []) if f.get('name') == 'SetStyle'] != CONTRACT['UCommonTextBlock']['functions']:
        raise ValueError('original SetStyle signature mismatch')
    if not isinstance(native_execs, dict) or native_execs.get('SetStyle') != CONTRACT['exec']:
        raise ValueError('original SetStyle native exec mismatch')
    if block.get('properties') != CONTRACT['UCommonTextBlock']['properties']:
        raise ValueError('original CommonTextBlock properties mismatch')
    decls = {'Style':'TSubclassOf<UCommonTextStyle> Style;', 'ScrollStyle':'TSubclassOf<UCommonTextScrollStyle> ScrollStyle;', 'bDisplayAllCaps':'bool bDisplayAllCaps;', 'bAutoCollapseWithEmptyText':'bool bAutoCollapseWithEmptyText;', 'MobileFontSizeMultiplier':'float MobileFontSizeMultiplier;'}
    for name, decl in decls.items():
        if [m.get('decl') for m in model.get('members_decl_order', []) if m.get('name') == name] != [decl]:
            raise ValueError('required CommonTextBlock member mismatch: ' + name)
    collisions = {'SetStyle', 'SynchronizeProperties', 'UCommonTextBlock'}
    if model.get('common_text_block_style_runtime') or any(x.get('name') in collisions for group in ('functions','members_table_order','members_decl_order','fields','helpers','static_helpers') for x in model.get(group, [])):
        raise ValueError('CommonTextBlock style declaration collision')
    model['functions'].append({'name':'SetStyle','rpc':False,'service':None,'ret':'void','args':['TSubclassOf<UCommonTextStyle> InStyle'], 'const':False,'notify':False,'splice':False,'blueprint_event':False,'access':'public','runtime_implementation':True,'specifiers':['BlueprintCallable'],'used':{}})
    model['common_text_block_style_runtime'] = True
    return model

def declarations(model):
    return (HERE / 'text-block-style-declarations.inc').read_text() if model.get('common_text_block_style_runtime') else ''

def definitions(model):
    return (HERE / 'text-block-style-definitions.inc').read_text() if model.get('common_text_block_style_runtime') else ''
