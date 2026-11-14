// CONFIDENCE: HIGH
// Original 0x1422c5e80. Concrete body; shared interface binding is tracked separately.
// AFortAIPawn inventory-owner +0xa8, original 0x1422c5e80.
OutItems.Empty();
AFortInventory* ItemInventory = Inventory;
if (ItemInventory)
{
    ItemInventory->NativeFindAllWorldItems(Definition, OutItems, false);
}
return OutItems.Num();
