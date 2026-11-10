// CONFIDENCE: HIGH
// Original: exec thunk 0x142a43510; forwarder 0x14236b730. The forwarder clears the output array, has the interface at +0x598
// (vtable slot 0xa8) fill a temporary TArray, then appends those elements to the output and frees the temporary.
ItemArray.Empty();
TArray<UFortItem*> Found;
InventoryInterface->NativeFindItemInstancesForDefinition(ItemDefinition, Found); // vslot 0xa8, unnamed
ItemArray.Append(Found);
