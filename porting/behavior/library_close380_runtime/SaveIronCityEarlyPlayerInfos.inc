// CONFIDENCE: high; native 0x142555180 copies the game instance's complete registered player array.
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (World)
    {
        AFortGameStateOutpost* GameState = Cast<AFortGameStateOutpost>(World->GetGameState());
        if (!GameState)
        {
            return;
        }
        UFortGameInstance* GameInstance = Cast<UFortGameInstance>(World->GetGameInstance());
        if (GameState && GameInstance)
        {
            GameState->IronCityEarlyPlayerInfos = GameInstance->RegisteredPlayers;
        }
    }
}
