// CONFIDENCE: HIGH
if (!Context)
{
    return nullptr;
}
UWorld* World = GEngine->GetWorldFromContextObject(Context, EGetWorldErrorMode::LogAndReturnNull);
AFortGameStateAthena* GameState = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;
AFortVolumeManager* Manager = GameState ? GameState->NativeGetVolumeManager() : nullptr;
return IsValid(Manager) ? Manager : nullptr;
