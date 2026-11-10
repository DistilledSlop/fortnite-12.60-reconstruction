// CONFIDENCE: MEDIUM. Finds the first SpawnGroupVisuals entry whose primary soft path equals SelectedCreatureVisualPath. When that entry has all three of its soft paths set, starts an async load of them; HandleAsyncLoadVisualsComplete applies the result later. Only the first match is used.
// Original address: 0x14274c0d0.
for (int32 Index = 0; Index < SpawnGroupVisuals.Num(); ++Index)
{
    const FSpawnGroupVisual& Entry = SpawnGroupVisuals[Index];
    if (!SoftObjectPathsEqual(Entry.PrimaryPath, SelectedCreatureVisualPath))
    {
        continue;
    }
    if (Entry.PrimaryPath.IsNone() || Entry.SecondaryPath.IsNone() || Entry.TertiaryPath.IsNone())
    {
        continue;
    }
    TArray<FSoftObjectPath> Paths;
    Paths.Add(Entry.PrimaryPath);
    Paths.Add(Entry.SecondaryPath);
    Paths.Add(Entry.TertiaryPath);
    bCreatureVisualsDoneLoading = false;
    FStreamableDelegate Callback = FStreamableDelegate::CreateUObject(this, &ABuildingProp_CreaturePlacer::HandleAsyncLoadVisualsComplete);
    const FString DebugName = TEXT("RequestAsyncLoad ArrayDelegate");
    SelectedCreatureVisualsLoadHandle = UAssetManager::Get().StreamableManager.RequestAsyncLoad(Paths, Callback, DebugName);
    return;
}
