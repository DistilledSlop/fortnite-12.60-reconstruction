"""Proposed interface base only; integration is blocked on the native override."""
NAME = 'ABuildingActor'
BLOCK = '0x146252530'
FLAG = 'building_energy51_ability_interface'
def apply(model, block, owner_fns):
    if model['name'] != NAME:
        return model
    if model.get('block_va') != BLOCK or block.get('block_va') != BLOCK:
        raise ValueError('building actor original class identity changed')
    if model.get(FLAG):
        raise ValueError('duplicate building ability interface policy')
    bases = model.setdefault('native_interface_bases', [])
    if 'IAbilitySystemInterface' in bases:
        raise ValueError('duplicate building ability interface base')
    bases.append('IAbilitySystemInterface')
    model['splice'].setdefault('header_includes', set()).add('AbilitySystemInterface.h')
    model[FLAG] = True
    return model
