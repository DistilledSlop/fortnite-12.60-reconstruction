// Native helper 0x14207a350 (static helper on AFortMinigame; callers EvaluateStatQuery 0x14208ca30 and GetScoreboardRowForPlayer 0x1420941c0).
// Asm: RCX = TArray header (Data at +0, Num at +8); entries are 0x10 bytes with a class pointer in the first qword and a value at +8.
// Per entry: RSI = entry class, RDI = *RDX (wanted class); each is kept only when non-null and IsChildOf(UFortMinigameStatFilter::StaticClass() = 0x1429d45e0);
// the entry address is returned when the two kept classes are equal, otherwise the scan continues; null when no entry matches.
// CONFIDENCE: MEDIUM
UClass* const FilterBaseClass = UFortMinigameStatFilter::StaticClass();
uint8* const EntryData = *reinterpret_cast<uint8* const*>(EntryArray);
const int32 EntryCount = *reinterpret_cast<const int32*>(EntryArray + 8);
uint8* const EntryEnd = EntryData + static_cast<int64>(EntryCount) * 0x10;
for (uint8* CurEntry = EntryData; CurEntry != EntryEnd; CurEntry += 0x10)
{
	UClass* EntryFilterClass = *reinterpret_cast<UClass* const*>(CurEntry);
	if (EntryFilterClass != nullptr && (FilterBaseClass == nullptr || !EntryFilterClass->IsChildOf(FilterBaseClass)))
	{
		EntryFilterClass = nullptr;
	}
	UClass* WantedFilterClass = *FilterClassKey;
	if (WantedFilterClass != nullptr && (FilterBaseClass == nullptr || !WantedFilterClass->IsChildOf(FilterBaseClass)))
	{
		WantedFilterClass = nullptr;
	}
	if (EntryFilterClass == WantedFilterClass)
	{
		return CurEntry;
	}
}
return nullptr;
