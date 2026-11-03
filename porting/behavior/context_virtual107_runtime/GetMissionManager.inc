// CONFIDENCE: HIGH
// 0x144cceab0 calls virtual GetWorld and resolves the game state mission manager.
UWorld* World = GetWorld();
const AFortGameState* GameState = World ? Cast<AFortGameState>(World->GetGameState()) : nullptr;
return GameState ? GameState->MissionManager : nullptr;
