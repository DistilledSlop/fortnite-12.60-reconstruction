// Behaviour snippet: UAthenaSpectateMatchModalBase::IsGameSessionString (body only; spliced into the generated member function).
// Original body: 0x144b38df0 (thunk 0x1451bd7d0; byte-identical body shared with UAthenaViewStreamingReplayModalBase::IsValidGameSessionId). Offset evidence: no receiver access. The FString argument is read at offsets +0x0 (Data, UTF-16 code units) and +0x8 (Num including the terminator). The asm requires Num - 1 == 0x20 (Num == 0x21) and every code unit in [0..32) to be 0-9, A-F or a-f.
// CONFIDENCE: HIGH
const int32 NumWithTerminator = String.Num();
if (NumWithTerminator != 0x21)
{
    return false;
}
const uint16* Chars = reinterpret_cast<const uint16*>(*String);
for (int32 Index = 0; Index < 0x20; ++Index)
{
    const uint16 Code = Chars[Index];
    const uint16 DigitOffset = static_cast<uint16>(Code - 0x30);
    const uint16 UpperOffset = static_cast<uint16>(Code - 0x41);
    const uint16 LowerOffset = static_cast<uint16>(Code - 0x61);
    if (!(DigitOffset <= 9 || UpperOffset <= 5 || LowerOffset <= 5))
    {
        return false;
    }
}
return true;
