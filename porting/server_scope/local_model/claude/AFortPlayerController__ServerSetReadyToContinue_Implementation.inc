// CONFIDENCE: MEDIUM
// Original: 0x1423acae0 (virtual Implementation of ServerSetReadyToContinue). Callees not named here are described in the evidence file.
// Ready-to-continue state for the player state. Authority only; otherwise forward to the server.
if (GetLocalRole() != ROLE_Authority)
{
	NativeForwardReadyToContinue(bTryStartTimer, RequestedState, bReady);
	return;
}
AFortPlayerState* FortPlayerState = Cast<AFortPlayerState>(PlayerState);
if (!FortPlayerState)
{
	return;
}
const uint32 StateFlags = FortPlayerState->NativeRecoveredFlags_0x330;
AFortGameMode* GameMode = Cast<AFortGameMode>(GetWorld()->AuthorityGameMode);
FortPlayerState->NativeSetReadyToContinue(bTryStartTimer, RequestedState, bReady);
if ((StateFlags & 0x2) == 0 && (StateFlags & 0x8) == 0 && bTryStartTimer && GameMode)
{
	if (GameMode->NativeGetRequiredReadyPlayerCount() <= NativeCountReadyPlayers(GameMode))
	{
		NativeStartReadyTimer(GetWorld());
	}
}
