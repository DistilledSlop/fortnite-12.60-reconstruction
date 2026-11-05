// CONFIDENCE: HIGH
    FBox Bounds(ForceInit);
    UWorld* World = GetWorld();
    for (TActorIterator<AFortStaticMeshActor> It(World); It; ++It)
    {
        AFortStaticMeshActor* Actor = *It;
        ULevel* Level = Actor->GetLevel();
        if (!Level)
            continue;

        const AFortWorldSettings* Settings = Cast<AFortWorldSettings>(Level->GetWorldSettings());
        if (Settings && !Settings->bUseProceduralFoliage)
            continue;

        const FString LevelPath = Level->GetPathName();
        if (LevelPath.StartsWith(TEXT("/Game/Maps/Skybox/"), ESearchCase::IgnoreCase) ||
            (LevelPath.StartsWith(TEXT("/Game/Maps/Zones/TheFarmstead/"), ESearchCase::IgnoreCase) &&
             (LevelPath.StartsWith(TEXT("/Game/Maps/Zones/TheFarmstead/GameStart_Cine"), ESearchCase::IgnoreCase) ||
              LevelPath.StartsWith(TEXT("/Game/Maps/Zones/TheFarmstead/Homebase"), ESearchCase::IgnoreCase))))
            continue;

        Bounds += Actor->GetComponentsBoundingBox(false, false);
    }

    const AFortGameStateZone* GameState = World->GetGameState<AFortGameStateZone>();
    if (GameState && GameState->StructuralSupportSystem)
        Bounds = NativeClipWindImpulseBounds(Bounds, GameState->StructuralSupportSystem->NativeWorldBounds_0x384);
    return Bounds;
