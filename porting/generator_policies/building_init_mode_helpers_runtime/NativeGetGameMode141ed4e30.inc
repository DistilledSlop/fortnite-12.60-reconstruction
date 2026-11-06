// CONFIDENCE: HIGH
// 0x141ed4e30. The original only checks GEngine on the fallback path.
if (ContextObject)
{
    UWorld* World = GEngine->GetWorldFromContextObject(ContextObject, EGetWorldErrorMode::ReturnNull);
    if (World)
    {
        if (UFortGameInstance* GameInstance = Cast<UFortGameInstance>(World->GetGameInstance()))
        {
            return GameInstance->NativeGetSubGame141e5f160();
        }
    }
}
return UFortAssetManager::NativeGetGameSubMode();
