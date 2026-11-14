// CONFIDENCE: HIGH
// Original 0x141ff83f0; borrowed group from inherited dynamic backend dispatch.
const FMcpProfileItemArray* Group = NativeFindItemsByTemplateKey(ProfileKey);
OutItems.Empty();
if (Group)
{
    OutItems.Reserve(Group->Num());
}
if (Group)
{
    for (const FMcpProfileItemPtr& Record : *Group)
    {
        // Original assumes every record's pointee is valid; do not silently skip null records.
        UFortItem* Item = Cast<UFortItem>(Record->ItemObject);
        if (Item && Item->GetItemDefinitionBP())
        {
            OutItems.Add(Item);
        }
    }
}
