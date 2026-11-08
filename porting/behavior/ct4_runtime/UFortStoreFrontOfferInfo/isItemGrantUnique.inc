// Behaviour snippet: UFortStoreFrontOfferInfo::isItemGrantUnique (body only; spliced into the generated member function).
// Original address: 0x1450d2610 (thunk 0x1452f8280)
// Offset evidence: this+0x28 is below the class's lowest own offset (0x40), so it is read raw as an owner pointer.
// Owner+0x210 is the element array Data pointer and owner+0x218 the int32 Num; elements are 0x38 bytes, FString at +0x0, int32 at +0x10.
// Asm: first element whose string equals TemplateId (wide, case-sensitive, null-terminated compare) returns (element.+0x10 == 1); no match, null owner or empty array returns false.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint8* OfferOwner = *reinterpret_cast<const uint8* const*>(Raw + 0x28);
if (OfferOwner == nullptr)
{
    return false;
}
const uint8* OwnerEntries = *reinterpret_cast<const uint8* const*>(OfferOwner + 0x210);
const int32 EntryCount = *reinterpret_cast<const int32*>(OfferOwner + 0x218);
const TCHAR* Needle = (TemplateId.Num() != 0) ? *TemplateId : TEXT("");
for (int32 EntryIndex = 0; EntryIndex < EntryCount; ++EntryIndex)
{
    const uint8* Entry = OwnerEntries + static_cast<int64>(EntryIndex) * 0x38;
    const TCHAR* EntryChars = (*reinterpret_cast<const int32*>(Entry + 0x8) != 0)
        ? *reinterpret_cast<const TCHAR* const*>(Entry)
        : TEXT("");
    if (FCString::Strcmp(Needle, EntryChars) == 0)
    {
        return *reinterpret_cast<const int32*>(Entry + 0x10) == 1;
    }
}
return false;
