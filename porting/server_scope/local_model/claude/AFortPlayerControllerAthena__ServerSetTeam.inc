// CONFIDENCE: MEDIUM. Original exec 0x142a5c480 (RPC thunk, one byte parameter InTeam).
// Validate is virtual slot 0x2478 -> 0x1419f2740: returns bit 2 of the byte at +0x9fc of the team settings object (FUN_141991c10(this, true)).
// Implementation is virtual slot 0x2480 -> 0x1419f2540, inlined below.
// Unresolved names: GameMode virtual slot 0x9e0 (0x14194f5e0, can-join check), GameMode virtual slot 0xb28 (0x141935140, team-change accept), pawn virtual slot 0xa78 (apply team tags). Team tag settings are FUN_141ed1800 at +0x380 and +0x410.
if (!((GetAthenaTeamSettings(true)->Field_0x9fc >> 2) & 1))
{
	RPC_ValidateFailed(TEXT("ServerSetTeam_Validate"));
	return;
}

// Implementation
if (MyFortPawn != nullptr)
{
	UWorld* World = GetWorld();
	AFortGameModeAthena* GameMode = (World != nullptr) ? World->AuthorityGameMode : nullptr; // World+0x128
	if (GameMode != nullptr && GameMode->IsA(AFortGameModeAthena::StaticClass()))
	{
		UFortTeamSettings* Settings = GetAthenaTeamSettings(true);
		const bool bInTeamRange = !(InTeam < Settings->MinTeamIndex || Settings->MaxTeamIndex < InTeam); // +0xc8 / +0xc9
		bool bCanJoin = false;
		if (PlayerState != nullptr && PlayerState->IsA(AFortPlayerState::StaticClass()))
		{
			bCanJoin = GameMode->CanPlayerStateJoinTeam_Slot0x9e0(PlayerState);
		}
		if (bInTeamRange && bCanJoin && GameMode->AcceptTeamChange_Slot0xb28(this, InTeam) && !(MyFortPawn->Field_0x520 & 8))
		{
			FGameplayTagContainer TeamTags;
			TeamTags.AddTag(GetTeamTagSettings()->Field_0x380); // FUN_141ed1800 + 0x380
			TeamTags.AddTag(GetTeamTagSettings()->Field_0x410); // FUN_141ed1800 + 0x410
			FGameplayTagContainer TeamTagsCopy = TeamTags;
			MyFortPawn->ApplyTeamTags_Slot0xa78(TeamTagsCopy, this, false);
		}
	}
}
