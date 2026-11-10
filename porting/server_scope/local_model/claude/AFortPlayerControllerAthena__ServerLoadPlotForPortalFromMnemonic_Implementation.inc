// CONFIDENCE: MEDIUM
// Original target 0x1419f1dd0. Header order: Mnemonic (FString*), Portal (UObject*).
if (Mnemonic == nullptr)
{
	return;
}
if (static_cast<int32>(Portal->ObjectFlags) < 2)
{
	return;
}
if (Helper_141ba8b20(this) != nullptr)
{
	// Already has a matched PlayerState: load only when it is the caller's own state (or both are null).
	auto* CallerState = PlayerState;
	if (CallerState != nullptr && !CallerState->IsA(AFortPlayerState::StaticClass()))
	{
		CallerState = nullptr;
	}
	if (Helper_141ba8b20(this) != CallerState)
	{
		return;
	}
}
// Build the plot-load request context for this controller (FUN_14237f3d0). The context is released at scope end.
auto Context = Helper_14237f3d0(this);
(void)Context;
