// Behaviour snippet: AFortPlayerControllerSpectating::IsValidToFollow (body only).
// Original body address: 0x1423d97f0 (thunk 0x142a6bf70). IsA check uses AFortPlayerStateAthena::StaticClass (0x142a8ae70) and the Cast chain.
// Offset evidence: [FPS+0x334] = PlayerRole (uint8, nonzero fails). [FPS+0xd00] bit 0 is not reflected (member_field NativeRecoveredUint8_0xd00 on AFortPlayerState, bit test 0x1).
// Athena-only NativeAllowsReplaySpectating216 (0x141a4a2d0) runs when the IsA check passed and the 0xd00 bit is clear, reached through the pawn check.
// CONFIDENCE: MEDIUM
static_assert(sizeof(AFortPlayerStateAthena) > 0, "AFortPlayerStateAthena must be complete for the IsA and replay-spectating calls");
if (!FPS || FPS->IsPendingKill())
{
	return false;
}
if (static_cast<uint8>(FPS->PlayerRole) != 0)
{
	return false;
}
const bool bIsAthenaState = FPS->IsA(AFortPlayerStateAthena::StaticClass());
AFortPlayerStateAthena* AthenaState = bIsAthenaState ? static_cast<AFortPlayerStateAthena*>(FPS) : nullptr;
const bool bSkipPawnCheck = bIsAthenaState && ((FPS->NativeRecoveredUint8_0xd00 & 0x1) != 0);
if (!bSkipPawnCheck && !FPS->GetCurrentPawn())
{
	return false;
}
if (AthenaState && !AthenaState->NativeAllowsReplaySpectating216())
{
	return false;
}
AFortPlayerPawn* CurrentPawn = FPS->GetCurrentPawn();
if (!CurrentPawn)
{
	return true;
}
if (!CurrentPawn->IsDead())
{
	return true;
}
return CurrentPawn->GetExtraLives() != 0;
