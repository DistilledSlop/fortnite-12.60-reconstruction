// CONFIDENCE: HIGH
// Original 0x141694520. Concrete body; shared interface binding is tracked separately.
// AFortAthenaAIBotController inventory-owner +0xa0, 0x141694520
UFortWorldItemDefinition* WorldDefinition = Cast<UFortWorldItemDefinition>(Definition);
AFortInventory* ItemInventory = Inventory;
return ItemInventory && WorldDefinition
    ? static_cast<UFortWorldItem*>(ItemInventory->NativeFindWorldItemByDefinition(WorldDefinition, false))
    : nullptr;
