// CONFIDENCE: HIGH
OutItems.Empty();
const FMcpProfileItemArray* Base = NativeFindBaseProfileGroup(BaseGroups, Key);
if (!Base)
{
    return 0;
}

int32 Matches = 0;
for (const FMcpProfileItemPtr& Item : *Base)
{
    // Both source pointers are required valid, as in the recovered path.
    const FMcpProfileItemPtr* Replacement = Overrides.Find(Item->ItemId);
    if (!Replacement)
    {
        OutItems.Add(Item);
    }
    else
    {
        ++Matches;
        if ((*Replacement)->Quantity > 0)
        {
            OutItems.Add(*Replacement);
        }
    }
}
return Matches;
