// Helper 0x142cd4630 (AFortAIPawn static). Parses a trailing "_<digits>" suffix from a UTF-16 string.
// Asm evidence: R11 = *len; scan backwards from Text[len-1] while (c - '0') <= 9, counting digits in R9D (JA exits to the
// continuation with R9 = digit count); fail if digits == 0 or digits >= len; Text[len-digits-1] must be '_' (0x5f);
// digits <= 10; multi-digit runs must not start with '0'; value (R10) accumulated as 10*R10 + digit, must be < 0x7fffffff;
// success writes *len = len - digits - 1 and returns value + 1 (EAX = R10 + 1); failure returns 0 and leaves *len unchanged.
// CONFIDENCE: HIGH
const int32 TotalLen = *InOutLength;
if (TotalLen <= 0)
{
    return 0;
}
int32 NumDigits = 0;
int32 ScanIndex = TotalLen - 1;
while (ScanIndex >= 0)
{
    const uint16 Ch = Text[ScanIndex];
    if (static_cast<uint16>(Ch - 0x30) > 9)
    {
        break;
    }
    ++NumDigits;
    --ScanIndex;
}
if (NumDigits == 0 || NumDigits >= TotalLen)
{
    return 0;
}
const int32 DigitStart = TotalLen - NumDigits;
if (Text[DigitStart - 1] != 0x5f)
{
    return 0;
}
if (NumDigits > 10)
{
    return 0;
}
if (NumDigits > 1 && Text[DigitStart] == 0x30)
{
    return 0;
}
int64 ParsedValue = 0;
for (int32 DigitIndex = 0; DigitIndex < NumDigits; ++DigitIndex)
{
    ParsedValue = ParsedValue * 10 + (static_cast<int64>(Text[DigitStart + DigitIndex]) - 0x30);
}
if (ParsedValue >= 0x7fffffff)
{
    return 0;
}
*InOutLength = TotalLen - NumDigits - 1;
return static_cast<int32>(ParsedValue + 1);
