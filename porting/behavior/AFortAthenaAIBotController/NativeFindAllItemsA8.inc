// CONFIDENCE: HIGH
// Original 0x141694990. Concrete body; shared interface binding is tracked separately.
// AFortAthenaAIBotController inventory-owner +0xa8, 0x141694990
OutItems.Empty();
AFortInventory* ItemInventory = Inventory;
if (ItemInventory)
{
    ItemInventory->NativeFindAllWorldItems(Definition, OutItems, false);
}
return OutItems.Num();
