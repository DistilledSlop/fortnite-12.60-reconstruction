// UNCOUNTED, UNCOMPILED native lift: 0x14204f2a0 and 0x14204e850.
// External operations below are declarations of unresolved native dependencies, not implementations.
// The pending collection is represented by the native TMap shape indicated by its 0x28-byte
// entries, GUID hashing and bit-array iterator. 142055070 replaces the pair at an
// existing key and releases the newly allocated sparse slot, preserving the old index.
struct FQuickbarMovePair { UFortItem* Item; int32 Slot; };
struct FQuickbarPendingValue { UFortItem* Item; int32 FromSlot; int32 ToSlot; };
// Include Misc/Crc.h when this draft is integrated. Native 14203fab0 hashes the
// 16-byte GUID through 142c9fd40, the verified deprecated CRC used by the item list.
struct FQuickbarPendingGuidKeyFuncs : TDefaultMapHashableKeyFuncs<FGuid, FQuickbarPendingValue, false>
{
    static uint32 GetKeyHash(KeyInitType Key)
    {
        return FCrc::MemCrc_DEPRECATED(&Key, sizeof(FGuid), 0);
    }
};
using FQuickbarPending = TMap<FGuid, FQuickbarPendingValue, FDefaultSetAllocator, FQuickbarPendingGuidKeyFuncs>;

// The exact free-function lifts are included once at file scope.
#include "../quickbar_helpers108_runtime/QuickbarHelpers.draft.inc"
bool Native_14204f0f0(AFortQuickBars* Owner, UFortItem* Item, FQuickBar& Bar,
                      int32 StartSlot, bool bReverse, const TArray<FGuid>& MoveGuids,
                      bool NativeFlag, int32& FoundSlot);
// 14204ec80 casts inherited Owner then dereferences the inventory interface
// without a null-owner guard. The recovered controller helper preserves its query.
UFortItem* NativeOwnerInventoryLookup_598_b0(AFortQuickBars* Owner, const FGuid& Guid)
{
    return Cast<AFortPlayerController>(Owner->GetOwner())->NativeFindItemByGuid(Guid);
}
// 14203f030 reserves for the sum of live entries, then visits Source in sparse
// bit-array order. Add replaces duplicates in place (14203fab0 -> 142055070).
// These containers are distinct at the call site.
void Native_14203f030(FQuickbarPending& Destination, const FQuickbarPending& Source)
{
    if (Source.Num() != 0) Destination.Reserve(Destination.Num() + Source.Num());
    for (const auto& Entry : Source)
        Destination.Add(Entry.Key, Entry.Value);
}


// 0x14204e850. The owner argument is present in the native ABI but unused here.
bool Native_14204e850(AFortQuickBars* Owner, UFortItem* Item, const FQuickBar& Bar,
                      int32 StartSlot, const TArray<FGuid>& MoveGuids, bool bIgnoreOccupants)
{
    return Owner->NativeCanFitItemInSlots(Item, Bar, StartSlot, MoveGuids, bIgnoreOccupants);
}

// 0x14204f0f0. Native probes the starting slot once even when it is outside
// the normal search range, then bounds-checks after each step.
bool Native_14204f0f0(AFortQuickBars* Owner, UFortItem* Item, FQuickBar& Bar,
                      int32 StartSlot, bool bReverse, const TArray<FGuid>& MoveGuids,
                      bool NativeFlag, int32& FoundSlot)
{
    return Owner->NativeFindItemSlot(Item, Bar, StartSlot, bReverse, MoveGuids, NativeFlag, FoundSlot);
}

// 0x14204ec80: validate the target span and plan pushes for its occupants.
// Native obtains an AFortPlayerController from Owner->Owner, then calls its +0x598
// inventory interface virtual +0xb0. If that item lookup returns null it skips it.
bool Native_14204ec80(AFortQuickBars* Owner, UFortItem* Item, FQuickBar& Bar,
                      int32 Slot, bool bPush, const TArray<FGuid>& MoveGuids,
                      bool NativeFlag, FQuickbarPending& LocalPending)
{
    if (!Item) return false;
    const uint8 Size = Item->GetNumberOfSlotsTaken();
    int32 LeftSearch = Slot - 1;
    int32 RightSearch = Slot + int32(Size);
    for (int32 Offset = 0; Offset < int32(Size); ++Offset)
    {
        int32 Index = Slot + Offset;
        if (!Bar.Slots.IsValidIndex(Index) || !Bar.Slots[Index].bEnabled ||
            !Bar.DataDefinition.QuickbarSlots.IsValidIndex(Index))
        {
            LocalPending.Empty(0);
            return false;
        }
        const uint8 Type = uint8(Item->NativeGetItemType());
        bool bAccepts = false;
        for (const auto Accepted : Bar.DataDefinition.QuickbarSlots[Index].AcceptedItemTypes)
            if (uint8(Accepted) == Type) { bAccepts = true; break; }
        if (!bAccepts) { LocalPending.Empty(0); return false; }
        const FQuickBarSlot& SlotData = Bar.Slots[Index];
        if (SlotData.Items.Num() == 0 && !SlotData.bIsOccupied) continue;
        if (SlotData.bIsOccupied) Index = SlotData.UsedBySlotIndex;
        // Native assumes UsedBySlotIndex is valid and accesses it without a bounds check.
        const FQuickBarSlot& Head = Bar.Slots[Index];
        FGuid Guid(0, 0, 0, 0);
        if (Head.Items.Num() > 0) Guid = Head.Items[0];
        else if (Head.bIsOccupied) Guid = Head.UsedByItemGuid;
        if (MoveGuids.Contains(Guid) || LocalPending.Contains(Guid)) continue;
        UFortItem* Occupant = NativeOwnerInventoryLookup_598_b0(Owner, Guid);
        if (!Occupant) continue;
        int32 RightFound = MAX_int32, LeftFound = MAX_int32;
        const bool bRight = Native_14204f0f0(Owner, Occupant, Bar, RightSearch,
                                             false, MoveGuids, NativeFlag, RightFound);
        const bool bLeft = Native_14204f0f0(Owner, Occupant, Bar, LeftSearch,
                                            true, MoveGuids, NativeFlag, LeftFound);
        if (!bRight && !bLeft) { LocalPending.Empty(0); return false; }
        // Native compares distances even when only one search succeeds; its initial
        // sentinel is INT_MAX and the search helper may update the out-parameters.
        const int32 RightDistance = int32(uint32(RightFound) - uint32(RightSearch));
        const int32 LeftDistance = int32(uint32(LeftFound) - uint32(LeftSearch));
        const bool bChooseRight = (RightDistance < LeftDistance) ||
            (RightDistance == LeftDistance && !bPush);
        const int32 Destination = bChooseRight ? RightFound : LeftFound;
        if (bChooseRight) RightSearch = Destination + 1;
        else LeftSearch = Destination - 1;
        LocalPending.Add(Guid, {Occupant, Index, Destination});
    }
    return true;
}

// 0x14204f2a0. FQuickBar's copy is deep (native copy helpers 14203f720,
// 14203f840 and 140769940); mutations below affect only this working copy.
bool Native_14204f2a0(AFortQuickBars* Owner, const TArray<FQuickbarMovePair>& Moves,
                      const FQuickBar& Bar, bool bPush, const TArray<FGuid>& MoveGuids,
                      FQuickbarPending& Pending)
{
    FQuickBar Working = Bar;
    for (const FQuickbarMovePair& Move : Moves)
    {
        const FGuid Guid = Move.Item->NativeGetItemGuid();
        if (Guid.IsValid())
            for (int32 Index = 0; Index < Working.Slots.Num(); ++Index)
                if (Working.Slots[Index].Items.Contains(Guid))
                {
                    if (Working.Slots[Index].bEnabled)
                        Native_142065b00(Working, Guid, Index);
                    break;
                }
        const uint8 Size = Move.Item->GetNumberOfSlotsTaken();
        for (int32 Index = Move.Slot; Index < Move.Slot + int32(Size); ++Index)
        {
            if (!Working.Slots.IsValidIndex(Index) || !Working.Slots[Index].bEnabled)
                return false;
            Working.Slots[Index].bIsReserved = true;
        }
    }
    for (const FQuickbarMovePair& Move : Moves)
    {
        FQuickbarPending LocalPending;
        if (!Native_14204ec80(Owner, Move.Item, Working, Move.Slot, bPush,
                              MoveGuids, true, LocalPending))
        {
            Pending.Empty(0);
            return false;
        }
        Native_14203f030(Pending, LocalPending);
        for (const auto& Entry : LocalPending)
        {
            Native_142065b00(Working, Entry.Key, Entry.Value.FromSlot);
            Native_142049220(Working, Entry.Value.Item, Entry.Value.ToSlot);
        }
    }
    return true;
}
