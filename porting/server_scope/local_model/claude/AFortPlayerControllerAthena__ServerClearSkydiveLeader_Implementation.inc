// CONFIDENCE: MEDIUM
// Original target 0x1419f1170.
if (SkydiveLeader.Ptr == nullptr)
{
	return;
}
SkydiveLeader.bFlag = true;	// byte at +8 of the leader record
if (auto* GameState = GetFortGameStateAthena(this))
{
	// Count the remaining skydive candidates (GameState list via FUN_1419982c0, keyed by PlayerState +0xfdc when valid).
	bool bReadyPlayerState = PlayerState != nullptr && PlayerState->IsA(AFortPlayerStateAthena::StaticClass());
	uint8 Key = bReadyPlayerState ? PlayerState->SkydiveKey_0xfdc : 0;
	TArray<AActor*> Candidates = Helper_1419982c0(GameState, Key);
	int32 EligibleCount = 0;
	for (AActor* Candidate : Candidates)
	{
		if (Candidate == nullptr)
		{
			continue;
		}
		if (auto* Controller = Candidate->Controller_0xe0)
		{
			if (Controller->IsA(AFortPlayerControllerAthena::StaticClass()) && Controller->Flag_0x2be8 == 0)
			{
				++EligibleCount;
			}
		}
	}
	if (EligibleCount > 1)
	{
		// More than one eligible candidate: only the leader record is cleared below.
		SkydiveLeader.Ptr = nullptr;
		Helper_1419e89a0(this);
		return;
	}
}
// Clear the leader's own skydive timer when it is still an AFortPlayerControllerAthena.
if (auto* Leader = SkydiveLeader.Ptr->Controller_0xe0)
{
	if (Leader->IsA(AFortPlayerControllerAthena::StaticClass()))
	{
		Leader->SkydiveTimer_0x2be0 = 0;
		Helper_1419e89a0(Leader);
	}
}
SkydiveLeader.Ptr = nullptr;
Helper_1419e89a0(this);
