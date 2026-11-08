// Helper 0x140708cf0, host UFortNiagaraGameplayLibrary (20 callers; e.g. caller 0x142ee63c0 at CALL 0x140708cf0 with RCX=RDI+0xc8, R8=key, RDX=out).
// Offset evidence (raw; the set is not a UObject, no reflected member): [+0x08] int32 Num, [+0x34] int32 compare value,
//   [+0x38] inline bucket table, [+0x40] heap bucket table pointer, [+0x48] int32 bucket count (mask = count-1),
//   [+0x00] element data pointer, element stride 0x20: uint64 Key at +0x00, int32 NextIndex at +0x18.
// Hash is the asm 32-bit mix of (Key>>4); result is the element index, or -1 when absent. Returns OutIndexPtr as in RAX.
// CONFIDENCE: MEDIUM
if (*reinterpret_cast<const int32*>(SetPtr + 0x08) == *reinterpret_cast<const int32*>(SetPtr + 0x34)) { *OutIndexPtr = -1; return OutIndexPtr; }
const uint64 HashShift = LookupKey >> 4;
const uint32 HashLow = static_cast<uint32>(HashShift);
uint32 HashR9 = 0x9e3779b9u - HashLow;
uint32 HashEax = HashLow << 8;
HashR9 ^= HashEax;
HashEax = HashR9 >> 0xd;
uint32 HashEcx = HashR9 + HashLow;
uint32 HashR10 = HashLow - HashR9;
HashEcx = 0u - HashEcx;
HashEcx ^= HashEax;
HashR10 -= HashEcx;
HashEax = HashEcx >> 0xc;
HashR10 ^= HashEax;
HashR9 -= HashR10;
HashR9 -= HashEcx;
HashEax = HashR10 << 0x10;
HashR9 ^= HashEax;
HashEcx -= HashR9;
HashEax = HashR9 >> 5;
HashEcx -= HashR10;
HashEcx ^= HashEax;
HashR10 -= HashR9;
HashR10 -= HashEcx;
HashEax = HashEcx >> 3;
HashR10 ^= HashEax;
HashR9 -= HashR10;
HashR9 -= HashEcx;
HashEax = HashR10 << 0xa;
HashR9 ^= HashEax;
HashEcx -= HashR9;
HashEcx -= HashR10;
const uint64 HashMix = static_cast<uint64>(static_cast<int64>(static_cast<int32>(HashEcx))) ^ (static_cast<uint64>(HashR9) >> 0xf);
const int64 HashMask = static_cast<int64>(*reinterpret_cast<const int32*>(SetPtr + 0x48)) - 1;
const uint64 BucketIndex = HashMix & static_cast<uint64>(HashMask);
const uint8* HashHeap = *reinterpret_cast<uint8* const*>(SetPtr + 0x40);
const uint8* BucketBase = HashHeap ? HashHeap : (SetPtr + 0x38);
int32 ElementIndex = *reinterpret_cast<const int32*>(BucketBase + BucketIndex * 4);
if (ElementIndex != -1)
{
    const uint8* ElementData = *reinterpret_cast<uint8* const*>(SetPtr);
    for (;;)
    {
        const uint8* Element = ElementData + (static_cast<int64>(ElementIndex) << 5);
        if (*reinterpret_cast<const uint64*>(Element) == LookupKey) { *OutIndexPtr = ElementIndex; return OutIndexPtr; }
        ElementIndex = *reinterpret_cast<const int32*>(Element + 0x18);
        if (ElementIndex == -1) { break; }
    }
}
*OutIndexPtr = -1;
return OutIndexPtr;
