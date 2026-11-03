// 0x1420397e0, proposed static wrapper over FFortItemEntry.
// Pending: AFortInventory::NativeUpdateEntry is not yet reconstructed.
if (Entry.ParentInventory.IsValid())
{
    Entry.ParentInventory.Get()->NativeUpdateEntry(Entry);
}
Entry.bIsDirty = true;
