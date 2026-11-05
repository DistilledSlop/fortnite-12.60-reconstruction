// CONFIDENCE: high; original source recovery, build/runtime pending.
if (!WorldContextObject)
{
    return nullptr;
}
UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
AFortGameState* GameState = World ? Cast<AFortGameState>(World->GetGameState()) : nullptr;
return GameState ? GameState->WindManager : nullptr;
