// CONFIDENCE: HIGH
// 0x144cc8180 calls virtual GetWorld and resolves the game state mission manager.
UWorld* World = GetWorld();
const AFortGameState* GameState = World ? Cast<AFortGameState>(World->GetGameState()) : nullptr;
const AFortMissionManager* Manager = GameState ? GameState->MissionManager : nullptr;
return Manager ? Manager->BluGloManager : nullptr;
