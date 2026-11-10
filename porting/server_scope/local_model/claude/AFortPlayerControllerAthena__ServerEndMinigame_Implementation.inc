// CONFIDENCE: LOW
// Original target 0x1419f1ad0.
AActor* Minigame = nullptr;
bool bInMinigame = Helper_1419c3880(this, &Minigame);	// out-param receives the controller's minigame; returns true when the GameState check passes
if (bInMinigame || (Minigame != nullptr && (Minigame->Flag_0x408 == 1 || (Minigame->Flags_0x2d8 & 1))))
{
	auto* Manager = Helper_141c795a0();	// placeholder accessor (takes no argument in the binary)
	if (Manager != nullptr && !Manager->IsPendingKill())
	{
		Helper_14208b4f0(Manager, this, !bAbandon);	// FUN_14208b4f0 (not read)
	}
}
