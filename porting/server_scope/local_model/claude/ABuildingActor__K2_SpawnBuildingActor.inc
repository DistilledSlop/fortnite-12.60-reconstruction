// CONFIDENCE: MEDIUM. Spawns a building actor of the requested class at Transform. Validates the world context and class, applies Athena spawn overrides from the world subsystem when the game state matches, then spawns.
// Original address: 0x1427379d0 (exec). Native helper: 0x141bafb20 (world/class checks, spawn params, UWorld::SpawnActor via 0x1442e6580).
UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
if (World == nullptr || BuildingClass == nullptr || !BuildingClass->IsChildOf(ABuildingActor::StaticClass()))
{
    return nullptr;
}

FActorSpawnParameters SpawnParameters;
AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(World->GetGameState());
if (GameState != nullptr && IsAthenaSpawnOverrideActive(GameState))
{
    if (UFortBuildingSpawnSubsystem* Subsystem = GetBuildingSpawnSubsystem(World))
    {
        ApplySpawnOverride(SpawnParameters, Subsystem, Transform.GetLocation());
    }
}

return Cast<ABuildingActor>(World->SpawnActor(BuildingClass, Transform, SpawnParameters));
