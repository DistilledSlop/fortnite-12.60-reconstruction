// CONFIDENCE: HIGH. Complete original inventory query.
// Original 0x142006b20. Appends; does not clear OutItems.
int32 Added = 0;
for (int32 Index = 0; Index < Inventory.ItemInstances.Num(); ++Index)
{
    UFortWorldItem* Item = Inventory.ItemInstances[Index];
    if (Item && Item->GetItemDefinitionBP() == ItemDefinition &&
        (bIncludeGifted || !Item->NativeWasGifted()))
    {
        OutItems.Add(Item);
        ++Added;
    }
}
return Added;
