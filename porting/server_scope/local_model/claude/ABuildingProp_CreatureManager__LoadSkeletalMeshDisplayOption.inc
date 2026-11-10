// CONFIDENCE: MEDIUM. Starts an asynchronous load of the skeletal mesh display option path. The returned handle replaces SkeletalMeshLoadHandle; completion is delivered to HandleAsyncLoadComplete.
// Original address: 0x14274b310.
if (SkeletalMeshDisplayOptionPath.AssetPathName.IsNone())
{
    return;
}
UFortAssetManager::Get();
TArray<FSoftObjectPath> Paths;
Paths.Add(SkeletalMeshDisplayOptionPath);
FStreamableDelegate Callback = FStreamableDelegate::CreateUObject(this, &ABuildingProp_CreatureManager::HandleAsyncLoadComplete);
const FString DebugName = TEXT("RequestAsyncLoad ArrayDelegate");
SkeletalMeshLoadHandle = UAssetManager::Get().StreamableManager.RequestAsyncLoad(Paths, Callback, DebugName);
