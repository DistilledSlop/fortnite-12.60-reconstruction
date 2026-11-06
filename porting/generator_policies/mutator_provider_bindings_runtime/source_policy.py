"""Restore original owner/context interfaces with recovered concrete callbacks."""
BLOCKS = {
    'UFortItem': ('0x1464134b0', 'IMcpItemAware'),
    'ADeployableBasePlot': ('0x146292820', 'IFortInventoryOwnerInterface'),
    'AFortAIPawn': ('0x1462afb30', 'IFortInventoryOwnerInterface'),
    'AFortPlayerController': ('0x14649da30', 'IFortInventoryOwnerInterface'),
    'AFortVolume': ('0x1465647c0', 'IFortMutatorOwner'),
    'AFortMinigame': ('0x146446ef0', 'IFortMutatorOwner'),
    'AFortGameModeAthena': ('0x1463cf250', 'IFortMutatorOwner'),
    'AFortGameStateAthena': ('0x1463ea150', 'IFortMutatorOwner'),
    'AFortPlayerStateAthena': ('0x1464d3e50', 'IFortMutatorContext'),
}


def apply(model, block, owner_fns):
    binding = BLOCKS.get(model['name'])
    if binding is None:
        return model
    expected, interface = binding
    if model.get('block_va') != expected or block.get('block_va') != expected:
        raise ValueError('mutator provider original class identity changed')
    bases = model.setdefault('native_interface_bases', [])
    if interface not in bases:
        bases.append(interface)
    model['splice'].setdefault('header_includes', set()).add('U' + interface[1:] + '.h')
    return model


def interface_declarations(name):
    # Assigned native contracts. Concrete original providers supply the bodies.
    if name == 'UFortMutatorOwner':
        return ['public:', '    virtual void NativeAppendOwnerMutators(TArray<class AFortGameplayMutator*>& OutMutators, UObject* Context) = 0;']
    if name == 'UFortMutatorContext':
        return ['public:', '    virtual void NativeAppendContextOwners(TArray<UObject*>& OutOwners) = 0;']
    if name == 'UFortInventoryOwnerInterface':
        return ['public:', '    virtual bool NativeHasInfiniteAmmo(class UFortWeaponItemDefinition* WeaponDefinition) = 0;', '    virtual class AFortInventory* NativeQuickBarInventory() = 0;', '    virtual class AFortWeapon* NativeQuickBarHeldWeapon() = 0;']
    if name == 'UMcpItemAware':
        return ['public:', '    TWeakPtr<FMcpProfileItem, ESPMode::NotThreadSafe> NativeMcpItem;', '    virtual bool NativePopulateFromMcpItem(FMcpProfileItem* Item) = 0;']
    return []
