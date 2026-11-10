// CONFIDENCE: MEDIUM. Reacts to the building's initialization. Once the authority game mode is ready, marks the saved-meta properties as processed; a building loaded from save also gets its loaded-from-save hook.
// Original address: 0x142737b30 (exec). Native body: 0x141bb0ce0 (virtual slot 0x638). The native body reads only InitializationReason; BuildingPersistentState is not used.
AGameModeBase* GameMode = GetWorld() ? GetWorld()->AuthorityGameMode : nullptr;
if (GameMode == nullptr || !GameMode->IsA(AFortGameModeAthena::StaticClass()) || !GameMode->IsBuildingInitializationReady())
{
    return;
}

if (InitializationReason == EFortBuildingInitializationReason::LoadedFromSave)
{
    OnBuildingLoadedFromSave();
}

bSavedMetaPropertiesProcessed = true;
