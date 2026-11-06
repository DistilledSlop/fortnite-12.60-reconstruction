// CONFIDENCE: high; native 0x1424ab2a0 selects the first non-null FortWorldManager in the object hash.
{
    TArray<UObject*> Managers;
    GetObjectsOfClass(AFortWorldManager::StaticClass(), Managers, true, RF_ClassDefaultObject, EInternalObjectFlags::None);
    for (UObject* Manager : Managers)
    {
        if (Manager)
        {
            UWorld* World = GEngine->GetWorldFromContextObject(Manager, EGetWorldErrorMode::LogAndReturnNull);
            AFortGameStateZone* GameState = World ? Cast<AFortGameStateZone>(World->GetGameState()) : nullptr;
            return GameState ? GameState->GetGameDifficulty() : 0.0f;
        }
    }
    return 0.0f;
}
