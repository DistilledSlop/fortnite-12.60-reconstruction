// CONFIDENCE: MEDIUM
// Original address: exec thunk 0x1428f0020 -> FUN_1419331a0(Context, NewAIClass).
// The helper touches only global replication-graph listener state; Context is not read.
// Listener array: DAT_1478c2670 (16-byte entries, count _DAT_1478c2678, max _DAT_1478c267c, reentrancy depth _DAT_1478c2684).
++GListenerReentrancyDepth;
bool bNeedsPrune = false;
for (int32 Index = GListeners.Num() - 1; Index >= 0; --Index)
{
	FListenerEntry& Entry = GListeners[Index];
	// Entry is bad when its weak/valid flag is clear, its listener is null, or the listener rejects NewAIClass (vtable slot 0x50).
	if (!Entry.IsValid() || Entry.Listener == nullptr || !Entry.Listener->AcceptsAIClass(NewAIClass))
	{
		bNeedsPrune = true;
	}
}
--GListenerReentrancyDepth;

if (bNeedsPrune && GListenerReentrancyDepth < 1)
{
	const int32 OldCount = GListeners.Num();   // iVar3 in the decompile: count captured before the prune pass
	// Prune pass (forward): drop entries that are invalid, null, or report stale (vtable slot 0x20).
	for (int32 Index = 0; Index < GListeners.Num(); )
	{
		FListenerEntry& Entry = GListeners[Index];
		if (!Entry.IsValid() || Entry.Listener == nullptr || Entry.Listener->IsStale())
		{
			GListeners.RemoveAt(Index, 1, /*bAllowShrinking*/ true);   // FUN_140690780
		}
		else
		{
			++Index;
		}
	}
	// Capacity hint: twice the count, at least 2; shrink when the hint is below the old count and the array is not already at count.
	GListenerSlack = (GListeners.Num() * 2 < 3) ? 2 : GListeners.Num() * 2;
	if (GListenerSlack < OldCount && GListeners.Max() != GListeners.Num())
	{
		GListeners.Shrink();   // FUN_1406911b0
	}
}
