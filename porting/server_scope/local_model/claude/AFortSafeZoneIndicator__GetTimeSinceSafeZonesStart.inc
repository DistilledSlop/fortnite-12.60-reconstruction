// CONFIDENCE: HIGH. Seconds since the safe-zone phase started, measured on the game state's server clock. Returns 0 when the world has no Athena game state.
// Original addresses: exec 0x142ad2780 (thunk; writes the float result). Body 0x141a48f40 (captured and read in full).

AFortGameStateAthena* GameState = nullptr;
UWorld* World = GetWorld();
if (World != nullptr)
{
    GameState = Cast<AFortGameStateAthena>(World->GameState);
}

if (GameState == nullptr)
{
    return 0.0f;
}

return GameState->GetServerWorldTimeSeconds() - GameState->SafeZonesStartTime;
