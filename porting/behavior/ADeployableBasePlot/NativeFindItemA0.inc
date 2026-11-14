// CONFIDENCE: HIGH
// Original 0x141c37980. Concrete body; shared interface binding is tracked separately.
// ADeployableBasePlot inventory-owner +0xa0, original 0x141c37980.
(void)bInStorageVault;
if (!NativeInventoryOwnerHasInventory())
{
    return nullptr;
}
UFortWorldItemDefinition* WorldDefinition = Cast<UFortWorldItemDefinition>(Definition);
AFortInventory* ItemInventory = PlotInventory;
return static_cast<UFortWorldItem*>(ItemInventory->NativeFindWorldItemByDefinition(WorldDefinition, false));
