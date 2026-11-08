// Original body 0x1420a4240 (thunk 0x1429c5fc0) for AFortMinigame::SetStatForPlayerBucket.
// Offset evidence: [RCX+0xf0] AActor::Role (GetLocalRole()); [RCX+0x870] CurrentState (reflected enum byte); RCX+0x810 RoundStats; [RoundStats+0x20] is PlayerBucketStats (TArray, Num at +0x28, Max +0x2c), element 0x18 bytes, BucketIndex dword at element+0x10.
// BucketIndex == -1 never matches an existing entry (compare skipped); a new zeroed element is added (TArray::AddZeroed). ABI: EDX = BucketIndex, R8 = InFilter, R9D = InValue.
// CONFIDENCE: MEDIUM
if (GetLocalRole() == ROLE_Authority && static_cast<uint8>(CurrentState) != 0)
{
	FFortMinigamePlayerBucketStats* BucketStats = nullptr;
	if (BucketIndex != -1)
	{
		for (FFortMinigamePlayerBucketStats& Candidate : RoundStats.PlayerBucketStats)
		{
			if (Candidate.BucketIndex == BucketIndex)
			{
				BucketStats = &Candidate;
				break;
			}
		}
	}
	if (BucketStats == nullptr)
	{
		const int32 NewElementIndex = RoundStats.PlayerBucketStats.AddZeroed();
		BucketStats = &RoundStats.PlayerBucketStats[NewElementIndex];
		BucketStats->BucketIndex = BucketIndex;
	}
	AFortMinigame::NativeSetStatFilterEntryValue1420a40c0(&RoundStats, reinterpret_cast<uint8*>(BucketStats), InFilter.Get(), InValue);
	OnRep_StatsChanged();
}
