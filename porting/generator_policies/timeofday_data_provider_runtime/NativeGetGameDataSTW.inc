// CONFIDENCE: HIGH
// 0x141df78c0 and its common loader 0x141df76b0.
if (GameDataSTW)
    return GameDataSTW;

UPrimaryDataAsset* LoadedData = nullptr;
const FPrimaryAssetType DataType(UGameDataSTW::StaticClass()->GetFName());
if (!GameDataNameSTW.IsEmpty())
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
        *GameDataNameSTW, *DataType.ToString());
}
GameDataSTW = Cast<UGameDataSTW>(LoadedData);
return GameDataSTW;
