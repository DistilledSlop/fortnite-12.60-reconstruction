// CONFIDENCE: MEDIUM. Original exec 0x142965b30; cores 0x141ee4910 then 0x141a4cf10.
// Forwards the scoring tags to the controller's player state when that state is authoritative and is an Athena player state.
if (!IsValid(Controller))
{
	return;
}

AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(Controller->PlayerState);
if (IsValid(PlayerState) && PlayerState->GetLocalRole() == ROLE_Authority)
{
	PlayerState->ProcessScoringEvent(TargetTags);
}
