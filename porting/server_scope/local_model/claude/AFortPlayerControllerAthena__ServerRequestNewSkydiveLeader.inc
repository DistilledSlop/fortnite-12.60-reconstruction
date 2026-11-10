// CONFIDENCE: MEDIUM. Original exec 0x142a5bb60 (RPC thunk, no parameters).
// Validate is virtual slot 0x2818 -> Stub_ReturnTrue (0x1406801d0), so no validation runs (STUB).
// Implementation is virtual slot 0x2820 -> 0x1419f21f0, inlined below.
// Helpers used: FUN_14447fc60 (skydive squad size), FUN_144481890 (this controller's squad index), FUN_14447f7e0 (next squad member PlayerState), FUN_1419e89a0 (skydive leader replication refresh). Only their first levels were read; see deps.
if (SkydiveLeader == PlayerState)
{
	const int32 SquadSize = GetSkydiveSquadSize();
	int32 SquadIndex = 0;
	if (TryGetSquadIndex(SquadIndex))
	{
		SquadIndex = (SquadIndex + 1) % SquadSize; // the computed index is not read again by this body
		AFortPlayerStateAthena* NewLeader = GetNextSquadPlayerState();
		if (NewLeader != nullptr && NewLeader->IsA(AFortPlayerStateAthena::StaticClass()))
		{
			SkydiveLeader = NewLeader;
			RefreshSkydiveLeader(); // FUN_1419e89a0 on this controller
			AFortPlayerControllerAthena* NewLeaderController = NewLeader->Owner; // NewLeader+0xe0
			if (NewLeaderController != nullptr && NewLeaderController->IsA(AFortPlayerControllerAthena::StaticClass()))
			{
				NewLeaderController->SkydiveLeader = NewLeader;
				NewLeaderController->RefreshSkydiveLeader(); // FUN_1419e89a0 on the new leader's controller
			}
		}
	}
}
