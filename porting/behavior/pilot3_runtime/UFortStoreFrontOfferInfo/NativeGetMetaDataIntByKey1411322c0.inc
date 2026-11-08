// Helper 0x1411322c0: look up a string key in a metadata pair array and parse the value as an int.
// Evidence: [RCX+0x20] = TArray Data, [RCX+0x28] = Num, element stride 0x20 (SHL 5) = TPair<FString,FString>
//   (Key FString at +0, Value FString at +0x10). The struct is embedded (callers pass this+0x30), so RCX is a raw struct pointer.
// 0x142bcf780 = case-insensitive wide compare through a lowercase table (FCString::Stricmp), 0 = equal.
// Value copy (Reserve + memcpy) then sign/one-dot/iswdigit scan = FCString::IsNumeric, then _wtoi = FCString::Atoi.
// Value with Num <= 1 (empty), non-numeric value, or missing key returns the default in R8D.
// lookup_offset UFortStoreFrontOfferInfo 0x20: no reflected member (callers use differently-based structs).
// CONFIDENCE: MEDIUM
const TArray<TPair<FString, FString>>& MetaPairs = *reinterpret_cast<const TArray<TPair<FString, FString>>*>(MetaStruct + 0x20);
for (const TPair<FString, FString>& Candidate : MetaPairs)
{
    if (FCString::Stricmp(*Candidate.Key, **LookupKey) == 0)
    {
        const FString ValueCopy = Candidate.Value;
        if (ValueCopy.Len() > 0 && FCString::IsNumeric(*ValueCopy))
        {
            return FCString::Atoi(*ValueCopy);
        }
        return FallbackValue;
    }
}
return FallbackValue;
