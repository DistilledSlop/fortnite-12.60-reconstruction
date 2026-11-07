// Original body address: 0x144e476d0
// Offset evidence: Pointer at object offset 0xb70 (below this class lowest own offset 0xb80, raw). Pointee byte at +0x69 (pointee class unresolved). Null path returns false.
// CONFIDENCE: MEDIUM
const uint8* Channel = *reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(this) + 0xb70);
if (Channel == nullptr)
{
    return false;
}
return Channel[0x69] != 0;
