// Uncounted free-function lift. Names correspond to the captured native addresses.
// Requires FQuickBar, FQuickBarSlot, UFortItem, FGuid and TArray definitions.

static int32 Native_142064b60(FQuickBarSlot& Slot, const FGuid& ItemGuid)
{
    // Native copies the argument before touching the slot or its array.
    const FGuid Guid = ItemGuid;
    Slot.bIsOccupied = false;
    Slot.bIsDirty = true;
    Slot.UsedByItemGuid = FGuid();
    Slot.UsedBySlotIndex = INDEX_NONE;
    if (!Guid.IsValid())
        return -1;

    // Stable compaction removes every matching GUID and returns the count removed.
    return Slot.Items.RemoveAll([&Guid](const FGuid& Entry) { return Entry == Guid; });
}

static void Native_142065b00(FQuickBar& Bar, const FGuid& Guid, int32 SlotIndex)
{
    if (!Guid.IsValid() || !Bar.Slots.IsValidIndex(SlotIndex) || !Bar.Slots[SlotIndex].bEnabled)
        return;

    const FGuid GuidCopy = Guid;
    Native_142064b60(Bar.Slots[SlotIndex], GuidCopy);
    for (int32 Index = 0; Index < Bar.Slots.Num(); ++Index)
    {
        FQuickBarSlot& Slot = Bar.Slots[Index];
        if (Slot.bIsOccupied)
        {
            const FGuid& Occupant = Slot.Items.Num() > 0 ? Slot.Items[0] : Slot.UsedByItemGuid;
            if (Occupant == Guid)
            {
                Slot.bIsOccupied = false;
                Slot.UsedByItemGuid = FGuid();
                Slot.bIsDirty = true;
                Slot.UsedBySlotIndex = INDEX_NONE;
            }
        }
    }
}

static void Native_142049220(FQuickBar& Bar, UFortItem* Item, int32 SlotIndex)
{
    if (!Item || !Bar.Slots.IsValidIndex(SlotIndex) || !Bar.Slots[SlotIndex].bEnabled)
        return;
    const FGuid Guid = Item->NativeGetItemGuid();
    if (!Guid.IsValid())
        return;

    FQuickBarSlot& Head = Bar.Slots[SlotIndex];
    if (Head.bEnabled && Guid.IsValid())
    {
        if (Head.Items.Num() == 0)
            Head.Items.SetNumUninitialized(1);
        Head.Items[0] = Guid;
        Head.bIsDirty = true;
    }
    const uint8 SlotCount = Item->GetNumberOfSlotsTaken();
    for (int32 Index = SlotIndex + 1; Index < SlotIndex + static_cast<int32>(SlotCount); ++Index)
    {
        if (Bar.Slots.IsValidIndex(Index) && Bar.Slots[Index].bEnabled)
        {
            FQuickBarSlot& Slot = Bar.Slots[Index];
            Slot.UsedByItemGuid = Guid;
            Slot.bIsDirty = true;
            Slot.bIsOccupied = true;
            Slot.UsedBySlotIndex = SlotIndex;
        }
    }
}
