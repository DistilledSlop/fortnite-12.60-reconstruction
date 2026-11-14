// CONFIDENCE: HIGH
// Original 0x141bdc710, x86-64 SSE2. ActorName is the FName ToString result.
// Windows original hashes ASCII-uppercased UTF-16 code units, including both
// halves of surrogate pairs, with the deprecated two-byte CRC table.
FTCHARToUTF16 NameUtf16(*ActorName, ActorName.Len());
uint32 NameHash = 0;
for (int32 I = 0; I < NameUtf16.Length(); ++I)
{
    uint16 Unit = static_cast<uint16>(NameUtf16.Get()[I]);
    if (Unit >= 'a' && Unit <= 'z')
        Unit -= 'a' - 'A';
    NameHash = (NameHash >> 8) ^ FCrc::CRCTable_DEPRECATED[(NameHash ^ Unit) & 0xff];
    NameHash = (NameHash >> 8) ^ FCrc::CRCTable_DEPRECATED[(NameHash ^ (Unit >> 8)) & 0xff];
}
const auto OriginalRound = [](float Value) -> uint32
{
    // Preserve both SSE operations and the MXCSR rounding mode, including
    // NaN/out-of-range cvt's integer-indefinite result. SAR is emulated with
    // unsigned bits to avoid relying on signed right-shift semantics.
    const __m128 V = _mm_set_ss(Value);
    const __m128 Twice = _mm_add_ss(V, V);
    const int32 Rounded = _mm_cvtss_si32(_mm_add_ss(Twice, _mm_set_ss(0.5f)));
    const uint32 Bits = static_cast<uint32>(Rounded);
    return (Bits >> 1) | (Bits & 0x80000000u);
};
uint32 Result = HashCombine(NameHash, OriginalRound(Location.X));
Result = HashCombine(Result, OriginalRound(Location.Y));
return HashCombine(Result, OriginalRound(Location.Z));
