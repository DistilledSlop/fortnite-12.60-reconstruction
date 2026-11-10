// CONFIDENCE: HIGH
// Original: exec thunk 0x142a43440; forwarder 0x14236b710. The forwarder adds 0x598 to the controller and tail-jumps to
// vtable slot 0xa0 of that interface object, passing the definition and the bool through unchanged.
return InventoryInterface->NativeFindExistingItemForDefinition(ItemDefinition, bInStorageVault);
