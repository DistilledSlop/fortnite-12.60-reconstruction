// CONFIDENCE: high
// 0x141df7530 and its common loader 0x141df76b0.
if (GameDataCommon)
    return GameDataCommon;

UPrimaryDataAsset* LoadedData = nullptr;
const FPrimaryAssetType DataType(UFortGameData::StaticClass()->GetFName());
if (!GameDataNameCommon.IsEmpty())
{
    TSharedPtr<FStreamableHandle> Handle = LoadPrimaryAssetsWithType(
        DataType, DefaultBundleState, FStreamableDelegate(), 0);
    if (Handle.IsValid())
    {
        Handle->WaitUntilComplete(0.0f, false);
        LoadedData = Cast<UPrimaryDataAsset>(Handle->GetLoadedAsset());
    }
} // The native shared handle is released before error handling and caching.
if (!LoadedData)
{
    UE_LOG(LogFort, Fatal, TEXT("Failed to load GameData asset at %s. Type %s. This is not recoverable and likely means you do not have the correct data to run Fortnite."),
        *GameDataNameCommon, *DataType.ToString());
}
GameDataCommon = Cast<UFortGameData>(LoadedData);
return GameDataCommon;
