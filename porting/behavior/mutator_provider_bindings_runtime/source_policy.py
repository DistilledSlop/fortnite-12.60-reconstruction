"""Restore the native owner contract and its two recovered implementations."""
BLOCKS = {'AFortGameModeAthena': '0x1463cf250', 'AFortGameStateAthena': '0x1463ea150'}


def apply(model, block, owner_fns):
    expected = BLOCKS.get(model['name'])
    if expected is None:
        return model
    if model.get('block_va') != expected or block.get('block_va') != expected:
        raise ValueError('mutator owner original class identity changed')
    bases = model.setdefault('native_interface_bases', [])
    if 'IFortMutatorOwner' not in bases:
        bases.append('IFortMutatorOwner')
    model['splice'].setdefault('header_includes', set()).add('UFortMutatorOwner.h')
    return model


def interface_declarations(name):
    if name != 'UFortMutatorOwner':
        return []
    # Assigned source contract for the observed +0x20 callback. No artificial
    # implementation is supplied; recovered concrete owners provide the body.
    return ['public:', '    virtual void NativeAppendOwnerMutators(TArray<class AFortGameplayMutator*>& OutMutators, UObject* Context) = 0;']
