// helper 0x142caa6e0 (UFortGameplayCueFunctionLibrary): case-insensitive keyword search in a wide string,
// honoring a word-start check (previous char must not be an ASCII letter or digit), then parses the number that follows
// the keyword with _wtof (import api-ms-win-crt-convert-l1-1-0 _wtof, mapped to FCString::Atod) and stores (float) into *OutValue.
// Keyword compare uses UFortGlobals::NativeCompareWideStringsNoCase142bcf940 (call returns 0 on equal).
// CONFIDENCE: MEDIUM
if (Haystack == nullptr || Keyword == nullptr)
{
    return false;
}
int32 KeywordLength = 0;
while (Keyword[KeywordLength] != 0)
{
    ++KeywordLength;
}
const int64 CompareCount = static_cast<int64>(KeywordLength) - 1;
const uint16 KeywordHead = (Keyword[0] >= 'a' && Keyword[0] <= 'z') ? static_cast<uint16>(Keyword[0] - 0x20) : Keyword[0];

bool bPrevAlnum = false;
const uint16* Scan = Haystack;
uint16 Current = *Scan;
++Scan;
if (Current == 0)
{
    return false;
}
while (true)
{
    const uint16 CurrentUpper = (Current >= 'a' && Current <= 'z') ? static_cast<uint16>(Current - 0x20) : Current;
    if (!bPrevAlnum && CurrentUpper == KeywordHead)
    {
        if (UFortGlobals::NativeCompareWideStringsNoCase142bcf940(Scan, Keyword + 1, static_cast<uint64>(CompareCount)) == 0)
        {
            const uint16* MatchStart = Scan - 1;
            if (MatchStart == nullptr)
            {
                return false;
            }
            *OutValue = static_cast<float>(FCString::Atod(reinterpret_cast<const TCHAR*>(MatchStart + KeywordLength)));
            return true;
        }
    }
    bPrevAlnum = (CurrentUpper >= 'A' && CurrentUpper <= 'Z') || (CurrentUpper >= '0' && CurrentUpper <= '9');
    Current = *Scan;
    ++Scan;
    if (Current == 0)
    {
        return false;
    }
}
