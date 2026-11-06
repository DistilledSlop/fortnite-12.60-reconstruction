// CONFIDENCE: HIGH
if (GetLocalRole() != ROLE_Authority)
{
    return;
}
if (!Entry.bIsReplicatedCopy)
{
    const FGuid Guid = UFortWorldItem::NativeGetOrCreateEntryGuid(Entry);
    if (!Guid.IsValid())
    {
        return;
    }
    FFortItemEntry** Found = Inventory.NativeEntriesByGuid.Find(Guid);
    if (!Found || !*Found)
    {
        return;
    }
    FFortItemEntry* Destination = *Found;
    UFortWorldItem::NativeCopyEntry(*Destination, Entry);
    Destination->bIsDirty = false;
    Inventory.MarkItemDirty(*Destination);
}
else
{
    Inventory.MarkItemDirty(Entry);
}
NativeNotifyInventoryChanged();
