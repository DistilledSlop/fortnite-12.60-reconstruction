// CONFIDENCE: LOW. Exec thunk 0x14292bed0 reads VoteType (byte) and calls virtual slot 0x940, native body 0x141ea3c40. That body looks up the failed-vote lockout bucket for VoteType in the bucket table at this+0xeb0 (chain via the index array at this+0xee8, count at this+0xef8, head checks at this+0xeb8/+0xee4). If found, it removes every queued lockout callback registered for that VoteType (FUN_1406a9dc0 moves them into a temporary array), then releases each callback's buffer and shared reference.
// Original address: 0x14292bed0 (exec). Native: 0x141ea3c40 (virtual slot 0x940 on AFortGameStateZone). The removal helper FUN_1406a9dc0 is captured but not ported line by line; its semantics are taken as RemoveAll.
if (FailedVoteLockoutBuckets.Num() != FailedVoteLockoutFreeCount)
{
	FFailedVoteLockoutBucket* Bucket = FindFailedVoteLockoutBucket(VoteType);
	if (Bucket != nullptr)
	{
		TArray<FVoteLockoutCallback> Removed = Bucket->Callbacks.RemoveAll(VoteType);
		for (FVoteLockoutCallback& Callback : Removed)
		{
			Callback.Release();
		}
	}
}
