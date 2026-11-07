// Original body address: 0x141657e50 (thunk 0x1427980b0). Body asm: 40 instructions.
// Offset evidence: TSet-style hash probe. Elements at this+0x290 (data) with count at +0x298 and free count at +0x2c4; element stride 16 = {uint8 key @0, float value @4, int32 next @8}.
// Hash buckets: heap pointer at +0x2d0 when set, else inline at +0x2c8; mask = HashSize(+0x2d8) - 1. Gap in the class's own region, so NativeRecovered members are used.
// Factor 0xc returns 0.0; empty set returns 0.0; missing key returns 0.0.
// CONFIDENCE: MEDIUM
const uint8 Factor = static_cast<uint8>(AIDirectorFactor);
if (Factor == 0xc) { return 0.0f; }
if (NativeRecoveredInt32_0x298 == NativeRecoveredInt32_0x2c4) { return 0.0f; }
const int64 HashMask = static_cast<int64>(NativeRecoveredInt32_0x2d8) - 1;
const int32* Buckets = NativeRecoveredUint64_0x2d0 != 0
    ? reinterpret_cast<const int32*>(NativeRecoveredUint64_0x2d0)
    : reinterpret_cast<const int32*>(&NativeRecoveredInt32_0x2c8);
int32 ElementIndex = Buckets[HashMask & static_cast<int64>(Factor)];
if (ElementIndex == -1) { return 0.0f; }
const uint8* Elements = reinterpret_cast<const uint8*>(NativeRecoveredUint64_0x290);
while (true)
{
    const uint8* Element = Elements + static_cast<int64>(ElementIndex) * 16;
    if (Element[0] == Factor) { return *reinterpret_cast<const float*>(Element + 4); }
    ElementIndex = *reinterpret_cast<const int32*>(Element + 8);
    if (ElementIndex == -1) { return 0.0f; }
}
