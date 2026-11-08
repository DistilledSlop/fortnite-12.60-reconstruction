// Original body 0x144d06920 (thunk 0x145260ab0). Offset evidence: same sparse element table as the item list (NativeRecoveredPtr_0x2c8, bit words, NativeRecoveredInt32_0x2f0); the body sums the int32 at element+0x8 over allocated elements with no filter.
// CONFIDENCE: MEDIUM
int32 Total = 0;
const uint8* Elements = reinterpret_cast<const uint8*>(NativeRecoveredPtr_0x2c8);
const uint32* Words = NativeRecoveredPtr_0x2e8 != 0
    ? reinterpret_cast<const uint32*>(NativeRecoveredPtr_0x2e8)
    : reinterpret_cast<const uint32*>(reinterpret_cast<const uint8*>(this) + 0x2d8);
for (int32 Index = 0; Index < NativeRecoveredInt32_0x2f0; ++Index)
{
    if (((Words[Index >> 5] >> (Index & 31)) & 1u) == 0)
    {
        continue;
    }
    Total += *reinterpret_cast<const int32*>(Elements + static_cast<int64>(Index) * 20 + 8);
}
return Total;
