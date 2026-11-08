// Original address: 0x141fc1840 (APartyDisplayManager helper; hash-chain lookup keyed by int32)
// Offset evidence: PlacementActors Num is reflected at +0x220 (used by name). Raw set-like layout: element Data at +0x298,
// Elements.Num int32 at +0x2a0, NumFreeIndices int32 at +0x2cc, hash heap pointer at +0x2d8 (inline table at +0x2d0),
// HashSize int32 at +0x2e0. Element stride 0x180: key int32 at +0, value pointer at +8, next-in-chain int32 at +0x178 (-1 ends).
// Raw access because the reflected HeroPlayerPawns TMap value FHeroSlotInfo is generated empty, so its stride is unknown to C++.
// CONFIDENCE: MEDIUM
if (SlotIndex < 0 || SlotIndex >= ManagerObj->PlacementActors.Num())
{
    return nullptr;
}
const uint8* RawManager = reinterpret_cast<const uint8*>(ManagerObj);
const int32 RawElementCount = *reinterpret_cast<const int32*>(RawManager + 0x2a0);
if (RawElementCount == *reinterpret_cast<const int32*>(RawManager + 0x2cc))
{
    return nullptr;
}
const int32 RawHashSize = *reinterpret_cast<const int32*>(RawManager + 0x2e0);
const uint8* RawHeapHashTable = *reinterpret_cast<const uint8* const*>(RawManager + 0x2d8);
const uint8* RawHashTable = RawHeapHashTable != nullptr ? RawHeapHashTable : (RawManager + 0x2d0);
const int64 RawBucketMask = static_cast<int64>(RawHashSize) - 1;
int32 RawElementIndex = *reinterpret_cast<const int32*>(RawHashTable + 4 * (RawBucketMask & static_cast<int64>(SlotIndex)));
const uint8* RawElements = *reinterpret_cast<const uint8* const*>(RawManager + 0x298);
while (RawElementIndex != -1)
{
    const uint8* RawElement = RawElements + static_cast<int64>(RawElementIndex) * 0x180;
    if (*reinterpret_cast<const int32*>(RawElement) == SlotIndex)
    {
        return *reinterpret_cast<UObject* const*>(RawElement + 8);
    }
    RawElementIndex = *reinterpret_cast<const int32*>(RawElement + 0x178);
}
return nullptr;
