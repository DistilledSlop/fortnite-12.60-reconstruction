// CONFIDENCE: high
// Original 0x14204e850; assigned source helper name.
UFortItem* SlotItem = Item;
if (!SlotItem) return false;
const FGuid UnusedGuid = SlotItem->NativeGetItemGuid(); // native still invokes this virtual
const uint8 Size = SlotItem->GetNumberOfSlotsTaken();
for (int32 Index = StartSlot; Index < StartSlot + int32(Size); ++Index)
{
    if (!Bar.Slots.IsValidIndex(Index) || !Bar.Slots[Index].bEnabled ||
        !Bar.DataDefinition.QuickbarSlots.IsValidIndex(Index)) return false;
    const auto Type = uint8(SlotItem->NativeGetItemType()); // native invokes once per slot
    bool bAccepts = false;
    for (const auto Accepted : Bar.DataDefinition.QuickbarSlots[Index].AcceptedItemTypes)
        if (uint8(Accepted) == Type) { bAccepts = true; break; }
    if (!bAccepts) return false;
    const FQuickBarSlot& Slot = Bar.Slots[Index];
    if (Slot.bIsReserved) return false;
    FGuid Occupant;
    if (Slot.Items.Num() > 0) Occupant = Slot.Items[0];
    else if (Slot.bIsOccupied) Occupant = Slot.UsedByItemGuid;
    if (!bIgnoreOccupants && (Slot.Items.Num() != 0 || Slot.bIsOccupied) &&
        !MoveGuids.Contains(Occupant)) return false;
}
return true;
