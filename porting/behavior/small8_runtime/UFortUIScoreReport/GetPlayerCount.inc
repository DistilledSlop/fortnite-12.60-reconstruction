// Original body address: 0x1450e9500
// Offset evidence: Pointer LocalPlayer is the reflected member at +0x28. Pointee int32 at +0x318 (pointee class unresolved, raw). Null path returns 0, which matches the original EAX value when the pointer is zero.
// CONFIDENCE: MEDIUM
const uint8* Player = reinterpret_cast<const uint8*>(LocalPlayer);
if (Player == nullptr)
{
    return 0;
}
return *reinterpret_cast<const int32*>(Player + 0x318);
