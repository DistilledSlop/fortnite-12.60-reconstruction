// CONFIDENCE: LOW -- DRAFT, UNCOUNTED. Bind the opaque helpers and virtual slots before integration.
// Native 0x14202a600 takes SearchString by value. FString/TArray/FText destructors
// below reproduce the temporary ownership and cleanup at each return.
if (SearchString.Len() == 0)
    return true;
TArray<FString> Terms;
NativeSplit_142bae420(SearchString, Terms, NativeDelimiter_1459a4bf4, true);
if (Terms.Num() == 0)
    return true;

TArray<FString> Fields;
{
    FText Value = NativeVirtualText_3a8(); // actual virtual dispatch, semantic name unresolved
    Fields.Add(NativeTextToString_142c40540(Value));
}
{
    FText Value = NativeVirtualText_3b8(); // actual virtual dispatch, semantic name unresolved
    Fields.Add(NativeTextToString_142c40540(Value));
}
for (const FString& Field : Fields)
{
    int32 Matches = 0;
    for (const FString& Term : Terms)
        if (NativeFind_142bace90(Field, *Term, 1, 0, -1) != -1)
            ++Matches;
    if (Matches == Terms.Num())
        return true;
}
return false;
