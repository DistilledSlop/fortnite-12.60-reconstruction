// Original body address: 0x1450b1d90
// Offset evidence: Pointer at object offset 0x28 (below this class lowest own offset 0x40, not reflected, raw). Pointee (class unresolved) has data pointer at +0x248, Num int32 at +0x250, element stride 24 bytes, int32 at element +4. Returns -1 when Num <= index. Pointee is not null-checked in the original.
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint8* Pointee = *reinterpret_cast<const uint8* const*>(Raw + 0x28);
const int32 Num = *reinterpret_cast<const int32*>(Pointee + 0x250);
if (Num <= RequirementIndex)
{
    return -1;
}
const uint8* Data = *reinterpret_cast<const uint8* const*>(Pointee + 0x248);
return *reinterpret_cast<const int32*>(Data + static_cast<int64>(RequirementIndex) * 24 + 4);
