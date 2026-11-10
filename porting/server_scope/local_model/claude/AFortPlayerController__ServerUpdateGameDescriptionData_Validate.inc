// CONFIDENCE: MEDIUM
// Original: 0x1423ad5a0. Checks only the game state. The other parameters are unused.
AGameStateBase* GameState = GetWorld() ? GetWorld()->GameState : nullptr;
if (GameState && GameState->IsA(AFortGameStateAthena::StaticClass()))
{
	if (NativeIsGameDescriptionEditable(GameState))
	{
		if (NativeGetSettings()->bGameDescriptionFlag)
		{
			return true;
		}
	}
}
return false;
