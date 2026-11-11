// CONFIDENCE: HIGH
// Native 0x14296c480 -> 0x141ef0ec0 -> 0x141c77490: the world's game state must be an AFortGameStateZone; its
// StructuralSupportSystem is returned, otherwise null.
const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
const AFortGameStateZone* GameState = World ? Cast<AFortGameStateZone>(World->GetGameState()) : nullptr;
return GameState ? GameState->StructuralSupportSystem : nullptr;
