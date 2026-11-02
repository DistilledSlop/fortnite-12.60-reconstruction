"""Source-guarded constructor and cache state for UCommonInputSettings."""
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONTRACT = json.loads((HERE / 'original-policy.json').read_text())


def validate_classes(blocks, name_to_block):
    identity = CONTRACT['identity']
    block = blocks.get(identity['block_va'])
    if (name_to_block.get('UCommonInputSettings') != identity['block_va'] or block is None
            or any(block.get(k) != v for k, v in identity.items())
            or block.get('properties') != CONTRACT['properties']):
        raise ValueError('original CommonInputSettings identity or properties differ')


def apply(model, block, owner_fns):
    if model['name'] != 'UCommonInputSettings':
        return model
    if (model.get('package') != '/Script/CommonInput'
            or model.get('block_va') != CONTRACT['identity']['block_va']
            or block.get('block_va') != CONTRACT['identity']['block_va']
            or '0x14081ed60' not in owner_fns):
        raise ValueError('original CommonInputSettings owner differs')
    for name, decl in CONTRACT['members'].items():
        if [m.get('decl') for m in model.get('members_decl_order', []) if m.get('name') == name] != [decl]:
            raise ValueError('settings emitted member differs: ' + name)
    if model.get('common_input_settings_constructor') or any(
            m.get('name') == 'NativeInputDataLoaded'
            for group in ('functions', 'members_table_order', 'members_decl_order', 'fields', 'helpers', 'static_helpers')
            for m in model.get(group, [])):
        raise ValueError('settings constructor or cache declaration collision')
    model['common_input_settings_constructor'] = True
    return model


def declarations(model):
    return (HERE / 'declarations.inc').read_text() if model.get('common_input_settings_constructor') else ''


def definitions(model):
    return (HERE / 'definitions.inc').read_text() if model.get('common_input_settings_constructor') else ''
