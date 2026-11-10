// CONFIDENCE: LOW
// Original: 0x1423ac960 (virtual Implementation of ServerSetPartyOwner). Callees not named here are described in the evidence file.
// Record the new party owner on the player state, tell the game mode's party manager, then refresh the owner-changed bindings.
AFortPlayerState* FortPlayerState = Cast<AFortPlayerState>(PlayerState);
if (!FortPlayerState)
{
	return;
}
FortPlayerState->NativeSetPartyOwnerUniqueId(PartyOwnerUniqueId);
AFortGameMode* GameMode = Cast<AFortGameMode>(GetWorld()->AuthorityGameMode);
if (GameMode && GameMode->PartyManager)
{
	GameMode->PartyManager->NativeNotifyPartyOwnerChanged(&FortPlayerState->NativeRecoveredUniqueNetId_0x248, PartyOwnerUniqueId);
}
// Stale-binding cleanup of the owner-changed delegate at controller +0x5c0 (name proposed).
PartyOwnerChangedDelegate.RemoveStaleBindings();
