// Helper 0x142cd4550 (static on AFortAIPawn).
// Evidence: porting/generated/native-cap-helpers3/0x142cd4550.asm, 60 instructions, no calls.
// Caller: 0x142cd43f0 passes RCX = [RDI] (string base) and RDX = int32* index, then uses EAX.
// ABI: RCX = const char* Str, RDX = int32* IndexInOut, returns EAX.
// Behavior: scan back from *IndexInOut - 1 over decimal digits; require a '_' before them, 1..10 digits,
// no leading zero unless one digit, value < 0x7fffffff. On success set *IndexInOut to the index before '_'
// and return value + 1. On failure return 0 and leave *IndexInOut unchanged.
// CONFIDENCE: MEDIUM
const int32 Index = *IndexInOut;
int32 DigitCount = 0;
int32 Scan = Index - 1;
while (Scan >= 0 && Str[Scan] >= '0' && Str[Scan] <= '9')
{
    DigitCount += 1;
    Scan -= 1;
}

const int32 Start = Index - DigitCount;
if (DigitCount == 0 || DigitCount >= Index || Start < 1 || Str[Start - 1] != '_' || DigitCount > 10)
{
    return 0;
}
if (DigitCount != 1 && Str[Start] == '0')
{
    return 0;
}

int64 Value = 0;
for (int32 Offset = 0; Offset < DigitCount; ++Offset)
{
    Value = Value * 10 + static_cast<int64>(Str[Start + Offset] - '0');
}
if (Value >= 0x7fffffff)
{
    return 0;
}

*IndexInOut = Index - DigitCount - 1;
return static_cast<int32>(Value + 1);
