// CONFIDENCE: HIGH
// 0x141df62a0. Fatal log normally terminates before the native fallback.
UFortAssetManager* Manager = GEngine ? Cast<UFortAssetManager>(GEngine->AssetManager) : nullptr;
if (!Manager)
{
    UE_LOG(LogFort, Fatal, TEXT("Invalid AssetManager class used, must be UFortAssetManager"));
    Manager = NewObject<UFortAssetManager>();
}
return *Manager;
