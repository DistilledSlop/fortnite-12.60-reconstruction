// CONFIDENCE: HIGH. Original 0x141fbc250; the cache is built once per settings.
if (NativeFoundationStreamingCacheInitialized_0x818)
    return;
NativeFoundationStreamingRecords_0x820.Reset();
if (IsInPersistentLevel(false))
    NativeAppendFoundationStreamingRecords(*this);
UWorld* World = GetWorld();
for (ULevelStreaming* Streaming : World->GetStreamingLevels())
{
    if (Streaming->IsA<ULevelStreamingAlwaysLoaded>())
    {
        if (ULevel* Level = Streaming->GetLoadedLevel())
        {
            AFortWorldSettings* Source = Cast<AFortWorldSettings>(Level->GetWorldSettings(true));
            if (Source && Source->bAllowBuildingStreamingData && Source->BuildingFoundationStreamingData.Num() != 0)
                NativeAppendFoundationStreamingRecords(*Source);
        }
    }
}
NativeFoundationStreamingCacheInitialized_0x818 = true;
