// CONFIDENCE: high; original source recovery, build/runtime pending.
UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
AFortGameState* GameState = World ? Cast<AFortGameState>(World->GetGameState()) : nullptr;
return GameState ? GameState->WorldManager : nullptr;
