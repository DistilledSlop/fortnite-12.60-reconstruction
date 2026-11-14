// CONFIDENCE: HIGH
// Original 0x141fca500. Split on the first case-insensitive /Temp occurrence,
// then take the part before the last underscore in the right-hand string.
FString Right;
SelectedInstance.ToString().Split(TEXT("/Temp"), nullptr, &Right,
                                  ESearchCase::IgnoreCase, ESearchDir::FromStart);
int32 UnderscoreIndex = INDEX_NONE;
if (Right.FindLastChar(TEXT('_'), UnderscoreIndex))
    return FName(*Right.Left(UnderscoreIndex));
return NAME_None;
