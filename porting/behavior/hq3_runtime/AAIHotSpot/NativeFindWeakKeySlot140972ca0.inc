// Helper 0x140972ca0 (AAIHotSpot host). Hash lookup of a FWeakObjectPtr key (int64) in a TMap-like table.
// Table layout from asm: [+0x0] element data ptr (20-byte elements: key int64 at +0, next index int32 at +0xc),
// [+0x8] int32 count compared with [+0x34] (equal = empty), [+0x38] inline bucket array,
// [+0x40] bucket array pointer (inline when null), [+0x48] int32 bucket count (power of two).
// Hash = (int64)(int32)(Key>>32) ^ (int64)(int32)Key, masked by bucket count - 1. Key equality via
// NativeWeakObjectPtrEquivalent1407327b0 (ported helper 0x1407327b0). Writes the found index, or -1, to *OutIndex.
// Every instruction of the asm is accounted for.
// CONFIDENCE: HIGH
if (*reinterpret_cast<const int32*>(Table + 0x8) == *reinterpret_cast<const int32*>(Table + 0x34))
{
    *OutIndex = -1;
    return OutIndex;
}
const uint8* LkElems = *reinterpret_cast<const uint8* const*>(Table);
uint8* LkBucketPtr = *reinterpret_cast<uint8* const*>(Table + 0x40);
const uint8* LkBuckets = LkBucketPtr != nullptr ? LkBucketPtr : Table + 0x38;
int64 LkHash = static_cast<int64>(static_cast<int32>(static_cast<uint64>(Key) >> 32)) ^ static_cast<int64>(static_cast<int32>(Key));
LkHash &= static_cast<int64>(*reinterpret_cast<const int32*>(Table + 0x48)) - 1;
int32 LkIdx = *reinterpret_cast<const int32*>(LkBuckets + LkHash * 4);
while (LkIdx != -1)
{
    const uint8* LkEntry = LkElems + static_cast<int64>(LkIdx) * 20;
    int64 LkEntryKey = *reinterpret_cast<const int64*>(LkEntry);
    if (UCreativeQuestComponent::NativeWeakObjectPtrEquivalent1407327b0(reinterpret_cast<const uint8*>(&LkEntryKey), reinterpret_cast<const uint8*>(&Key)))
    {
        *OutIndex = LkIdx;
        return OutIndex;
    }
    LkIdx = *reinterpret_cast<const int32*>(LkEntry + 0xc);
}
*OutIndex = -1;
return OutIndex;
