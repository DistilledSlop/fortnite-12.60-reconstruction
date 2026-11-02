"""Unintegrated CommonButton style reconstruction; never adds SetStyle."""
import json
from pathlib import Path
HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())
HELPERS = ('NativeGetStyleCDO', 'NativeRebuildStyle', 'NativeRefreshCurrentStyle',
           'NativeRefreshDimensions', 'NativeOnCurrentTextStyleChanged',
           'FCommonButtonStyleRebuildProbeAccess')

def validate_classes(blocks, name_to_block, optional_sound_block):
    for name, expected in CONTRACT['classes'].items():
        va = expected['identity']['block_va']
        block = blocks.get(va)
        if name_to_block.get(name) != va or block is None:
            raise ValueError('original class identity mismatch: ' + name)
        if any(block.get(k) != v for k, v in expected['identity'].items()):
            raise ValueError('original class metadata mismatch: ' + name)
        names = {p['name'] for p in expected['properties']}
        if [p for p in block.get('properties', []) if p.get('name') in names] != expected['properties']:
            raise ValueError('original style property metadata mismatch: ' + name)
    event = CONTRACT['event']
    button = blocks[CONTRACT['classes']['UCommonButton']['identity']['block_va']]
    if [f for f in button.get('functions', []) if f.get('name') == event['name']] != [event]:
        raise ValueError('original text-style event metadata mismatch')
    if optional_sound_block != CONTRACT['optional_sound']:
        raise ValueError('original optional sound struct metadata mismatch')

def validate_members(model, expected):
    members = model.get('members_decl_order', model.get('members', []))
    for name, decl in expected.items():
        if [m.get('decl') for m in members if m.get('name') == name] != [decl]:
            raise ValueError('style rebuild member declaration mismatch: ' + name)

def validate_models(models, optional_sound_model, internal_declarations):
    # Invoke after normal model construction, with models keyed by C++ name.
    for name, expected in CONTRACT['classes'].items():
        model = models[name]
        if model.get('package') != '/Script/CommonUI':
            raise ValueError('style rebuild model package mismatch: ' + name)
        validate_members(model, expected['members'])
    validate_members(optional_sound_model, CONTRACT['optional_sound_members'])
    for declaration in ('void NativeSetMinimumWidth(int32 InWidth);',
                        'void NativeSetMinimumHeight(int32 InHeight);'):
        if internal_declarations.count(declaration) != 1:
            raise ValueError('real internal minimum-size adapter unavailable: ' + declaration)
    button = models['UCommonButton']
    if not button.get('common_button_text_style_runtime') or not button.get('common_button_query_runtime'):
        raise ValueError('shared weak, selected and enabled foundation unavailable')

def apply(model, block, owner_fns):
    if model['name'] != 'UCommonButton':
        return model
    if model.get('package') != '/Script/CommonUI' or block.get('block_va') != '0x145a01210' or '0x140805cb0' not in owner_fns:
        raise ValueError('original CommonButton rebuild owner mismatch')
    validate_members(model, CONTRACT['classes']['UCommonButton']['members'])
    if not model.get('common_button_text_style_runtime') or not model.get('common_button_query_runtime'):
        raise ValueError('shared weak, selected and enabled foundation unavailable')
    if model.get('common_button_style_rebuild_runtime') or any(m.get('name') in HELPERS
            for group in ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers')
            for m in model.get(group, [])):
        raise ValueError('style rebuild declaration collision')
    model['common_button_style_rebuild_runtime'] = True
    return model

def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get('common_button_style_rebuild_runtime') else ''

def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get('common_button_style_rebuild_runtime') else ''
