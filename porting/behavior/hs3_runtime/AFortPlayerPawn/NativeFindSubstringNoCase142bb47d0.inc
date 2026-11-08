// helper 0x142bb47d0 (AFortPlayerPawn): ASCII case-insensitive wide-string substring search.
// Evidence: RDX=needle, RCX=haystack; needle[0] and haystack chars uppercased ('a'..'z' -> -0x20);
// on first-char match calls 0x142bcf940 (NativeCompareWideStringsNoCase142bcf940) with (haystack+1, needle+1, len-1),
// a zero result is a match and the function returns the haystack position of that first char; null inputs or end of haystack return nullptr.
// CONFIDENCE: HIGH
if (Haystack == nullptr || Needle == nullptr)
{
    return nullptr;
}
int32 NeedleLength = 0;
while (Needle[NeedleLength] != 0)
{
    ++NeedleLength;
}
const int64 CompareCount = static_cast<int64>(NeedleLength) - 1;
const uint16 NeedleHead = (Needle[0] >= 'a' && Needle[0] <= 'z') ? static_cast<uint16>(Needle[0] - 0x20) : Needle[0];

const uint16* Scan = Haystack;
uint16 Current = *Scan;
++Scan;
if (Current == 0)
{
    return nullptr;
}
while (true)
{
    const uint16 CurrentUpper = (Current >= 'a' && Current <= 'z') ? static_cast<uint16>(Current - 0x20) : Current;
    if (CurrentUpper == NeedleHead)
    {
        if (UFortGlobals::NativeCompareWideStringsNoCase142bcf940(Scan, Needle + 1, static_cast<uint64>(CompareCount)) == 0)
        {
            return Scan - 1;
        }
    }
    Current = *Scan;
    ++Scan;
    if (Current == 0)
    {
        return nullptr;
    }
}
