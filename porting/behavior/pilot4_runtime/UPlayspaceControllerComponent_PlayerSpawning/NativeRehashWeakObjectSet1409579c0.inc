// Helper 0x1409579c0 (153 insns): RCX = TSet. Layout from asm: Elements data pointer at +0x0 (16-byte elements:
// 8-byte key, int32 HashNextId at +8, int32 HashIndex at +0xc); allocation-flag bit array at +0x10 (4 inline words,
// secondary word pointer at +0x20, NumBits at +0x28, MaxBits at +0x2c); hash storage = TInlineAllocator<1> at +0x38
// (inline slot +0x38, secondary pointer +0x40) with HashSize at +0x48.
// Key hash from asm: IMUL hi32*0x17 then ADD lo32 = TWeakObjectPtr hash (ObjectIndex + 23*SerialNumber).
// Asm: free the secondary hash buffer (0x142bfd200 with size 0); if HashSize != 0: allocate HashSize dwords when HashSize > 1
// (0x140690920 = TInlineAllocator::ResizeAllocation, element size 4), fill all buckets with -1, then for every allocated
// element index in increasing order (bit iterator 0x140687d00): HashIndex = hash & (HashSize-1); store HashIndex at
// +0xc, previous bucket head at +8, and the element index in the bucket.
// CONFIDENCE: MEDIUM (element key type recovered from the hash function only; every instruction accounted for)
void*& SecondaryHash = *reinterpret_cast<void**>(Set + 0x40);
if (SecondaryHash)
{
    FMemory::Free(SecondaryHash);
    SecondaryHash = nullptr;
}
const int32 HashSize = *reinterpret_cast<const int32*>(Set + 0x48);
if (HashSize == 0)
{
    return;
}
if (HashSize > 1)
{
    SecondaryHash = FMemory::Malloc(static_cast<SIZE_T>(HashSize) * 4, 0);
}
int32* HashBuckets = reinterpret_cast<int32*>(SecondaryHash ? SecondaryHash : static_cast<void*>(Set + 0x38));
for (int32 BucketIndex = 0; BucketIndex < HashSize; ++BucketIndex)
{
    HashBuckets[BucketIndex] = -1;
}
const int32 FlagBitCount = *reinterpret_cast<const int32*>(Set + 0x28);
const uint32* FlagWords = *reinterpret_cast<uint32* const*>(Set + 0x20)
    ? *reinterpret_cast<uint32* const*>(Set + 0x20)
    : reinterpret_cast<const uint32*>(Set + 0x10);
uint8* ElementData = *reinterpret_cast<uint8**>(Set);
for (int32 ElementIndex = 0; ElementIndex < FlagBitCount; ++ElementIndex)
{
    if ((FlagWords[ElementIndex >> 5] & (1u << (ElementIndex & 31))) == 0)
    {
        continue;
    }
    uint8* Element = ElementData + static_cast<SIZE_T>(ElementIndex) * 16;
    const uint64 ElementKey = *reinterpret_cast<const uint64*>(Element);
    const uint32 KeyHash = static_cast<uint32>(ElementKey) + static_cast<uint32>(ElementKey >> 32) * 0x17u;
    const int32 BucketIndex = static_cast<int32>(KeyHash & static_cast<uint32>(HashSize - 1));
    *reinterpret_cast<int32*>(Element + 0xc) = BucketIndex;
    *reinterpret_cast<int32*>(Element + 0x8) = HashBuckets[BucketIndex];
    HashBuckets[BucketIndex] = ElementIndex;
}
