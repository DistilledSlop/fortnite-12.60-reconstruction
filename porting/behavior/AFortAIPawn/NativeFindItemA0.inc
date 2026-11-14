// CONFIDENCE: HIGH
// Original 0x1422c5d10. Concrete body; shared interface binding is tracked separately.
// AFortAIPawn inventory-owner +0xa0, original 0x1422c5d10.
(void)bInStorageVault;
UFortWorldItemDefinition* WorldDefinition = Cast<UFortWorldItemDefinition>(Definition);
AFortInventory* ItemInventory = Inventory;
return ItemInventory && WorldDefinition
    ? static_cast<UFortWorldItem*>(ItemInventory->NativeFindWorldItemByDefinition(WorldDefinition, false))
    : nullptr;
