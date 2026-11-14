// CONFIDENCE: HIGH
// Original 0x1421c1800; borrowed group from inherited dynamic backend dispatch.
const FMcpProfileItemArray* Group = NativeFindItemsByTemplateKey(Key);
OutItems.Empty();
if (Group)
{
    if (Group->Num() > 0)
    {
        OutItems.Reserve(Group->Num());
    }
    for (const FMcpProfileItemPtr& Record : *Group)
    {
        // No shared-pointer copy or validity test in the original record loop.
        UFortAccountItem* Item = Cast<UFortAccountItem>(Record->ItemObject);
        if (Item && Item->GetItemDefinitionBP())
        {
            OutItems.Add(Item);
        }
    }
}
