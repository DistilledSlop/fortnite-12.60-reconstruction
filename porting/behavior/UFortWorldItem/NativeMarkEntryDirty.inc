// CONFIDENCE: HIGH
if (Entry.ParentInventory.IsValid())
{
    AFortInventory* Parent = Entry.ParentInventory.Get();
    Parent->NativeUpdateEntry(Entry);
}
Entry.bIsDirty = true;
