// CONFIDENCE: LOW
// Original target 0x1419f10b0.
// Game state from GetWorld()->GameState (vslot 0x150, world +0x130), accepted only when it IsA AFortGameStateAthena.
AActor* GameState = nullptr;
if (UWorld* World = GetWorld())
{
	if (auto* State = World->GameState_0x130)
	{
		if (State->IsA(AFortGameStateAthena::StaticClass()))
		{
			GameState = State;
		}
	}
}
// GetFortEngine() (FUN_141ed2350); the engine must have its +0xc9 flag set, and FUN_14199e430 must accept the game state.
if (auto* Engine = GetFortEngine())
{
	if (Engine->Flag_0xc9 && GameState != nullptr && Helper_14199e430(GameState))
	{
		if (DeviceButtonComponent != nullptr)
		{
			// Notify the component's delegate subsystem if it has listeners (count at +0x... of DeviceButtonComponent[5]).
			if (DeviceButtonComponent->ListenerCount_5 > 0)
			{
				Helper_1406a9dc0(DeviceButtonComponent->Subsystem_4, 0);
			}
			// FUN_141c74820(this): lookup of the controller's device handler (placeholder); then FUN_141c8cf90 broadcasts the click.
			if (auto* Handler = Helper_141c74820(this))
			{
				Helper_141c8cf90(Handler, DeviceButtonComponent->ClickedDelegateKey_4);
			}
		}
	}
}
