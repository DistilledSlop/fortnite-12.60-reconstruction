// Behaviour snippet: AFortPlayerControllerSpectating::IsPlayerInRelevancy (body only).
// Original body address: 0x1423d9390 (thunk 0x142a6bed0). Calls AFortPlayerState::GetCurrentPawn (0x1419b6d20) twice; second result reused (pure const getter).
// Offset evidence: [FPS+0x334] byte compared with 0 -> AFortPlayerState::PlayerRole (EFortPlayerRole, uint8). FPS pending-kill check = IsPendingKill (flags bit 29).
// CONFIDENCE: HIGH
if (!FPS || FPS->IsPendingKill())
{
	return false;
}
if (static_cast<uint8>(FPS->PlayerRole) != 0)
{
	return false;
}
AFortPlayerPawn* CurrentPawn = FPS->GetCurrentPawn();
if (!CurrentPawn || CurrentPawn->IsPendingKill())
{
	return false;
}
return true;
