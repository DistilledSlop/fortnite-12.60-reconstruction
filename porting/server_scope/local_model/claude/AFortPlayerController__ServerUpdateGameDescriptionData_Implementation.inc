// CONFIDENCE: MEDIUM
// Original: 0x1423ad4a0 (virtual Implementation of ServerUpdateGameDescriptionData). Callees not named here are described in the evidence file.
// Apply the game description to a minigame settings building; other targets are rejected with a warning.
if (DataTarget && DataTarget->IsA(AFortMinigameSettingsBuilding::StaticClass()))
{
	NativeApplyGameDescription(DataTarget, GameName, GameDescriptionText);
	return;
}
UE_LOG(LogFort, Warning, TEXT("GAME DESCRIPTION: target actor is not MinigameSettingsBuilding: %s"), *GetNameSafe(DataTarget));
