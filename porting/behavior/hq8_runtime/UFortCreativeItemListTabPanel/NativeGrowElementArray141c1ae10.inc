// Helper 0x141c1ae10 (UFortCreativeItemListTabPanel): TArray slack-grow for a 0x150-byte element array.
// Storage layout: Data at +0x0, ArrayNum at +0x8 (int32), ArrayMax at +0xc (int32).
// Callee evidence: FMemory::QuantizeSize (0x142bfce10) and FMemory::Realloc (0x142bfd200) per callee_hints.
// CONFIDENCE: HIGH
const int32 ArrayNumValue = *reinterpret_cast<const int32*>(ArrayStorage + 0x8);
const int32 ArrayMaxValue = *reinterpret_cast<const int32*>(ArrayStorage + 0xc);
int64 GrowElementCount = 1;
if (ArrayMaxValue != 0)
{
    GrowElementCount = static_cast<int64>(ArrayNumValue) + ((static_cast<int64>(ArrayNumValue) * 3) >> 3);
}
else if (ArrayNumValue > 1)
{
    GrowElementCount = static_cast<int64>(ArrayNumValue);
}
const uint64 RequestBytes = static_cast<uint64>(GrowElementCount) * 0x150ULL;
const uint64 QuantizedBytes = FMemory::QuantizeSize(static_cast<SIZE_T>(RequestBytes), 0);
int32 NewArrayMax = static_cast<int32>(QuantizedBytes / 0x150ULL);
if (ArrayNumValue > NewArrayMax)
{
    NewArrayMax = 0x7fffffff;
}
*reinterpret_cast<int32*>(ArrayStorage + 0xc) = NewArrayMax;
uint8* OldArrayData = *reinterpret_cast<uint8**>(ArrayStorage);
if (OldArrayData != nullptr || NewArrayMax != 0)
{
    const SIZE_T NewAllocBytes = static_cast<SIZE_T>(static_cast<int64>(NewArrayMax)) * 0x150;
    *reinterpret_cast<void**>(ArrayStorage) = FMemory::Realloc(OldArrayData, NewAllocBytes, 0);
}
