// CONFIDENCE: MEDIUM. Original exec 0x14296e620; body 0x141ef4e30 -> 0x141f88460 (game state WorldManager, field +0x3d0) -> 0x141f881f0 (the manager's world game state, WorldLevel at +0x2f4).
// Returns the WorldLevel of the world reached through the game state's WorldManager, or 0 at the first missing link.
UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
AFortGameState* GameState = World != nullptr ? Cast<AFortGameState>(World->GameState) : nullptr;
if (GameState == nullptr || GameState->WorldManager == nullptr)
{
	return 0;
}

UWorld* ManagerWorld = GameState->WorldManager->GetWorld();
AFortGameState* ManagerGameState = ManagerWorld != nullptr ? Cast<AFortGameState>(ManagerWorld->GameState) : nullptr;
if (ManagerGameState == nullptr)
{
	return 0;
}

return ManagerGameState->WorldLevel;
