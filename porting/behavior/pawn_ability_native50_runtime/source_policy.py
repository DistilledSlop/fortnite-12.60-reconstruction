"""Restore the native pawn ability-system interface and virtual dispatch."""
NAME = 'AFortPawn'
BLOCK = '0x146484820'
FLAG = 'pawn_ability_native50_runtime'
def apply(model, block, owner_fns):
    if model['name'] != NAME:
        return model
    if model.get('block_va') != BLOCK or block.get('block_va') != BLOCK:
        raise ValueError('pawn original class identity changed')
    if model.get(FLAG):
        raise ValueError('duplicate pawn ability interface policy')
    bases = model.setdefault('native_interface_bases', [])
    if 'IAbilitySystemInterface' in bases:
        raise ValueError('duplicate pawn ability interface base')
    bases.append('IAbilitySystemInterface')
    model['splice'].setdefault('header_includes', set()).add('AbilitySystemInterface.h')
    model[FLAG] = True
    return model
