// CONFIDENCE: HIGH. Original 0x141bea280.
if (bVisible && !NativeIsFoundationAndParentsEnabled())
        return;

    for (AFortHLODSMActor* Proxy : SpawnedLODActors)
        if (Proxy)
            Proxy->NativeSetFoundationProxyVisibility(bVisible);

    UWorld* World = GetWorld();
    if (bIncludeStreamingRecords)
    {
        // Original does not null-check the world/settings in this path. The
        // world settings accessor uses the persistent level, not a streamed level.
        AFortWorldSettings* Settings = static_cast<AFortWorldSettings*>(
            GetWorld()->GetWorldSettings(false, true));
        NativeSetRecordProxyVisibility(StreamingData, bVisible);
        for (int32 ChildIndex : StreamingData.ChildStreamingDataIndices)
            NativeSetRecordProxyVisibility(Settings->NativeGetFoundationStreamingRecord(ChildIndex), bVisible);
    }

    if (!bVisible && bForceDitheringTransition && World->IsGameWorld() &&
        LevelStreamInfo && LevelStreamInfo->GetLoadedLevel())
    {
        // Compact nulls in order without shrinking allocation.
        HLODActors.RemoveAll([](ALODActor* Actor) { return Actor == nullptr; });
        if (HLODActors.Num() == 0 && SpawnedLODActors.Num() != 0)
        {
            ULevel* Level = LevelStreamInfo->GetLoadedLevel();
            const int32 LastLODLevel = static_cast<AFortWorldSettings*>(Level->GetWorldSettings())->SplineHLODIndex + 1;
            for (AActor* Actor : Level->Actors)
                if (ALODActor* LOD = Cast<ALODActor>(Actor))
                    if (LOD->LODLevel == StreamingData.PersistentHLODLevelIndex ||
                        LOD->LODLevel == LastLODLevel)
                        HLODActors.Add(LOD);
        }
        for (ALODActor* LOD : HLODActors)
            LOD->StartDitherTransition();
    }
