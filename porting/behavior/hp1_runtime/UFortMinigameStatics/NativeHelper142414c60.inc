// Helper 0x142414c60, host UFortMinigameStatics (static).
// Evidence: RBX = [RCX+0xe0]; null -> return 0. CALL AFortPlayerController::StaticClass() (hint).
// ClassRaw+0x38 is an int32 index into the holder array at [RBX+0x10]+0x30 (Num at holder+0x38; index above Num -> 0).
// The array entry at that index must equal ClassRaw+0x30; if so return RBX, else 0.
// CONFIDENCE: MEDIUM
uint8* EntryRaw = *reinterpret_cast<uint8* const*>(reinterpret_cast<const uint8*>(Context) + 0xe0);
if (!EntryRaw)
{
    return nullptr;
}
const uint8* ClassRaw = reinterpret_cast<const uint8*>(AFortPlayerController::StaticClass());
const uint8* ClassSlotRaw = ClassRaw + 0x30;
const int32 ClassIndex = *reinterpret_cast<const int32*>(ClassRaw + 0x38);
const uint8* HolderRaw = *reinterpret_cast<const uint8* const*>(EntryRaw + 0x10);
if (ClassIndex > *reinterpret_cast<const int32*>(HolderRaw + 0x38))
{
    return nullptr;
}
const uint8* SlotArrayRaw = *reinterpret_cast<const uint8* const*>(HolderRaw + 0x30);
if (*reinterpret_cast<const uint8* const*>(SlotArrayRaw + static_cast<int64>(ClassIndex) * 8) != ClassSlotRaw)
{
    return nullptr;
}
return EntryRaw;
