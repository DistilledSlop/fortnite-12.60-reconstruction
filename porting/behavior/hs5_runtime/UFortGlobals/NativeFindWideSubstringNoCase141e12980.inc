// Native 0x141e12980: case-insensitive wide-string substring search.
// Returns the first position in SearchChars where PatternChars matches (ASCII a..z folded to upper case), or nullptr.
// Asm evidence: null guards on RDX/RCX; first pattern char folded by -0x20 when in 0x61..0x7a; haystack scanned
// until NUL; tail compared through UFortGlobals::NativeCompareWideStringsNoCase142bcf940 with MaxCount = strlen(pattern)-1.
// CONFIDENCE: HIGH
if (SearchChars == nullptr || PatternChars == nullptr)
{
    return nullptr;
}
uint16 WideFirstChar = PatternChars[0];
if (static_cast<uint32>(WideFirstChar) - 0x61u < 0x1au)
{
    WideFirstChar = static_cast<uint16>(WideFirstChar - 0x20);
}
int32 WideTailCount = -1;
do
{
    ++WideTailCount;
} while (PatternChars[WideTailCount] != 0);
--WideTailCount;

uint16 WideCurrent = SearchChars[0];
if (WideCurrent == 0)
{
    return nullptr;
}
const uint16* WideCursor = SearchChars + 1;
do
{
    uint16 WideFolded = WideCurrent;
    if (static_cast<uint32>(WideFolded) - 0x61u < 0x1au)
    {
        WideFolded = static_cast<uint16>(WideFolded - 0x20);
    }
    if (WideFolded == WideFirstChar
        && UFortGlobals::NativeCompareWideStringsNoCase142bcf940(
               WideCursor, PatternChars + 1, static_cast<uint64>(static_cast<int64>(WideTailCount))) == 0)
    {
        return WideCursor - 1;
    }
    WideCurrent = *WideCursor;
    ++WideCursor;
} while (WideCurrent != 0);
return nullptr;
