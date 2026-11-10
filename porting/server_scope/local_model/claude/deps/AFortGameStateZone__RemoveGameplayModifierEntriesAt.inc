// dep AFortGameStateZone__RemoveGameplayModifierEntriesAt original 0x141ea9700
// CONFIDENCE: MEDIUM. Original 0x141ea9700.
if (Count != 0)
{
	for (int32 i = 0; i < Count; ++i)
	{
		FActiveGameplayModifier& Removed = Entries[Index + i];
		Removed.Reset();   // zeroes ref count and frees the owned arrays (0x141e99170 / 0x1408a5b50 / FMemory_Free)
	}
	const int32 Tail = Entries.Num() - Index - Count;
	if (Tail != 0)
	{
		Entries.MoveTail(Index, Index + Count, Tail);   // memmove
	}
	Entries.SetNum(Entries.Num() - Count);
	if (bAllowShrink)
	{
		ShrinkEntries(Entries);   // 0x141eab210
	}
}
