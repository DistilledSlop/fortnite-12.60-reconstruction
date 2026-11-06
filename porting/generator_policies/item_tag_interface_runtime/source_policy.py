"""Attach the recovered native gameplay-tag interface to the original class."""
NAME = 'UFortItemDefinition'
BLOCK = '0x146416900'
FLAG = 'item_tag_interface_runtime'


def apply(model, block, owner_fns):
    if model['name'] != NAME:
        return model
    if model.get('block_va') != BLOCK or block.get('block_va') != BLOCK:
        raise ValueError('item definition original class identity changed')
    if model.get(FLAG):
        raise ValueError('duplicate item tag interface policy')
    for group in ('functions', 'members_table_order', 'members_decl_order', 'fields', 'static_helpers'):
        if any(m.get('name') == 'GetOwnedGameplayTags' for m in model.get(group, [])):
            raise ValueError('item tag interface declaration collision: ' + group)
    bases = model.setdefault('native_interface_bases', [])
    if 'IGameplayTagAssetInterface' in bases:
        raise ValueError('duplicate item tag interface base')
    bases.append('IGameplayTagAssetInterface')
    model['splice'].setdefault('header_includes', set()).add('GameplayTagAssetInterface.h')
    model[FLAG] = True
    return model
