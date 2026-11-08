// Helper 0x142bcf940: bounded wide-string compare with ASCII case folding. RCX=LeftChars, RDX=RightChars, R8=MaxCount (SIZE_T).
// Evidence: asm 0x142bcf940. Raw-equal nonzero chars continue; raw-equal zero char returns 0; a raw mismatch with any char >= 0x80 returns the raw difference;
// otherwise the folded difference is returned when nonzero. Fold table at .rdata 0x1465b3720 (128 bytes read from the PE image):
// identity except bytes 0x41..0x5A map to 0x61..0x7A. Callers 0x142caabb0 and 0x141e12980 branch on EAX==0.
// CONFIDENCE: HIGH
for (uint64 Index = 0; Index < MaxCount; ++Index)
{
    const uint32 LeftChar = static_cast<uint32>(LeftChars[Index]);
    const uint32 RightChar = static_cast<uint32>(RightChars[Index]);
    if (LeftChar != RightChar)
    {
        if (((LeftChar | RightChar) & 0xffffff80u) != 0)
        {
            return static_cast<int32>(LeftChar - RightChar);
        }
        const int32 FoldedLeft = (LeftChar >= 'A' && LeftChar <= 'Z') ? static_cast<int32>(LeftChar + 0x20u) : static_cast<int32>(LeftChar);
        const int32 FoldedRight = (RightChar >= 'A' && RightChar <= 'Z') ? static_cast<int32>(RightChar + 0x20u) : static_cast<int32>(RightChar);
        const int32 FoldedDiff = FoldedLeft - FoldedRight;
        if (FoldedDiff != 0)
        {
            return FoldedDiff;
        }
    }
    else if (LeftChar == 0)
    {
        return 0;
    }
}
return 0;
