// UNCOUNTED source lift; integration dependencies below are declarations only.
// Do not compile or promote until their virtual bindings and slot type exist.
// Reflection identifies FortAlterationSlotStatus (16 bytes): Alteration at 0,
// MinRequiredLevel at 8, MinHostItemRarity at 12.  The source spelling below
// denotes that reflected type and does not introduce a replacement layout.

// Virtual +0x490, reflected GetAlterationSlots: returns a fresh array by value.
TArray<FFortAlterationSlotStatus> Unresolved_ItemVirtual_490(const UFortItem* Item);
// Virtual +0x3f0, reflected GetRarity, returns EFortRarity (uint8).
EFortRarity Unresolved_ItemVirtual_3f0(const UFortItem* Item);
// Virtual +0x3a0: writes/returns FText; bool argument is passed unchanged.
FText Unresolved_ItemVirtual_3a0(const UFortItem* Item, bool bWithoutPrefix);

// 0x14202b8d0, UFortItem::GetActiveAlterations.
TArray<UFortAlterationItemDefinition*> Rough_GetActiveAlterations(
    const UFortItem* Item, int32 PretendLevel)
{
    if (PretendLevel < 0)
        PretendLevel = Item->GetLevel(); // proven virtual +0x268

    TArray<FFortAlterationSlotStatus> Slots = Unresolved_ItemVirtual_490(Item);
    TArray<UFortAlterationItemDefinition*> Result;
    if (Slots.Num() > 0)
        Result.Reserve(Slots.Num());
    for (const FFortAlterationSlotStatus& Slot : Slots) {
        if (Slot.MinRequiredLevel <= PretendLevel) {
            // Call even when Alteration is null, and re-read for each qualifying slot.
            const EFortRarity Rarity = Unresolved_ItemVirtual_3f0(Item);
            if (static_cast<uint8>(Slot.MinHostItemRarity) <= static_cast<uint8>(Rarity)
                && Slot.Alteration != nullptr)
                Result.Add(Slot.Alteration);
        }
    }
    return Result;
}

// 0x14202d250, UFortItem::GetDisplayNameWithoutPrefix.
FText Rough_GetDisplayNameWithoutPrefix(const UFortItem* Item)
{
    return Unresolved_ItemVirtual_3a0(Item, true);
}

// exec 0x1429588c0: advances the script frame, invokes virtual +0x490 into
// temporary array storage, moves that result to the caller's result storage,
// then destroys any remaining temporary. The reflected C++ virtual return
// expresses the intended value semantics; exec frame handling is generated.
TArray<FFortAlterationSlotStatus> Rough_GetAlterationSlotsExecValue(const UFortItem* Item)
{
    return Unresolved_ItemVirtual_490(Item);
}
