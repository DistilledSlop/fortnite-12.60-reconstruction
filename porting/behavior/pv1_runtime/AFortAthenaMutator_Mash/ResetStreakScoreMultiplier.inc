// Original body 0x141ab9dc0 (thunk 0x1427e9fd0) for AFortAthenaMutator_Mash::ResetStreakScoreMultiplier.
// Offset evidence: [RCX+0x11a8] is PlayerDataArray (reflected FFastArraySerializer struct); Entries TArray at +0x108 (=0x12b0, Num 0x12b8), stride 0x80; entry+0x10 = PlayerState, +0x54/+0x55 = StreakScoreLevel/StreakMultiplierCurrentKillCount.
// Tail sequence (ReplicationID = ++IDCounter with wrap, ReplicationKey++, ItemMap reset via 0x14072a990, ArrayReplicationKey++ with wrap, CachedNumItems = -1) is FFastArraySerializer::MarkItemDirty.
// CONFIDENCE: MEDIUM
for (FMashPlayerDataEntry& Entry : PlayerDataArray.Entries)
{
	if (Entry.PlayerState == PlayerState)
	{
		Entry.StreakScoreLevel = 0;
		Entry.StreakMultiplierCurrentKillCount = 0;
		PlayerDataArray.MarkItemDirty(Entry);
		return;
	}
}
