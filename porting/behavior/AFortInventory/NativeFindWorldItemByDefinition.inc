// CONFIDENCE: HIGH
// Original AFortInventory helper 0x1420065a0; method and flag names are assigned.
if (ItemDefinition)
{
    for (int32 Index = 0; Index < Inventory.ItemInstances.Num(); ++Index)
    {
        UFortWorldItem* Item = Inventory.ItemInstances[Index];
        if (Item && Item->GetItemDefinitionBP() == ItemDefinition &&
            (bIncludeGifted || !Inventory.ItemInstances[Index]->NativeWasGifted()))
        {
            return Inventory.ItemInstances[Index];
        }
    }
}
return nullptr;
